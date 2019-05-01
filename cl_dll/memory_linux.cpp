//
// memory_linux.cpp
//
// Memory pathcer for Linux
//

#include <algorithm>
#include <list>
#include <vector>
#include <string>
#include <cassert>
#include <cstdarg>
#include <fstream>
#include <map>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <link.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "hud.h"
#include "com_utils.h"
#include "memory.h"
#include "svc_messages.h"

// Inititalize with nullptr to trigger SIGSEGV when used
void **g_EngineBuf = nullptr;
int *g_EngineBufSize = nullptr;
int *g_EngineReadPos = nullptr;
UserMessage **g_pUserMessages = nullptr;
// Any non-Windows build is new and uses SDL
bool g_bNewerBuild = true;

static int MPROTECT_PAGESIZE = sysconf(_SC_PAGE_SIZE);
static int MPROTECT_PAGEMASK = ~(MPROTECT_PAGESIZE-1);

namespace Memory
{
struct module_info_t
{
	// [start; end)
	size_t start = 0;
	size_t end = 0;
	void *handle = nullptr;
	bool isValid = false;
};

typedef void (*msg_parse_func_t)();

struct svc_func_t
{
	unsigned char opcode;
	const char *pszname;
	msg_parse_func_t *pfnParse;
};

struct sizebuf_t
{
  const char *buffername;
  uint16_t flags;
  byte *data;
  int maxsize;
  int cursize;
};

// Console output
static bool g_bMessagesPrinted = false;
static std::list<std::string> g_DebugMsgs;
static void MemConPrintf(const char *fmt, ...);

// Modules
static module_info_t g_HwModule, g_GameUIModule;
static bool GetModuleInfo(const std::string &modName, module_info_t &info);
static void *ResolveSymbol(void *handle, const char *symbol);

// Memory patching
static std::map<size_t, int> g_AddrToProtect;
static void LoadProtectFromProc();
static int GetProtectForAddr(size_t addr);
static uint32_t HookDWord(size_t origAddr, uint32_t newDWord); // Replaces double word on specified address with new dword, returns old dword
static void ExchangeMemoryBytes(size_t *origAddr, size_t *dataAddr, uint32_t size); // Exchanges bytes between memory address and bytes array

// Svc Messages
static svc_func_t *g_pSvcMessagesTable = nullptr;
static sizebuf_t *g_pEngineBuffer = nullptr;
static bool InitSvcHook();

// GameUI patches
static CGameConsole003 *g_pGameConsole = nullptr;
static RGBA *g_pPrintColor = nullptr;
static void PatchGameUI();

}

//-----------------------------------------------------------------------------------------------------------
// Debug output
//-----------------------------------------------------------------------------------------------------------
void Memory::MemConPrintf(const char *fmt, ...)
{
	char buf[256];
	va_list va;
	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);
	g_DebugMsgs.push_back(buf);
}

//-----------------------------------------------------------------------------------------------------------
// Module functions
//-----------------------------------------------------------------------------------------------------------
bool Memory::GetModuleInfo(const std::string &modName, Memory::module_info_t &info)
{
	info.handle = dlopen(modName.c_str(), RTLD_NOW | RTLD_NOLOAD);
	try
	{
		const char *filename = "/proc/self/maps";
		std::ifstream maps(filename);
		if (maps.fail())
		{
			MemConPrintf("GetModuleAddress %s\n", modName.c_str());
			MemConPrintf("Failed to open %s: %s\n", filename, strerror(errno));
			return false;
		}
		
		std::vector<std::pair<size_t, size_t>> addrs;
		addrs.reserve(4);
		
		std::string line;
		while (std::getline(maps, line))
		{
			if (line.substr(line.size() - modName.size(), modName.size()) != modName)
				continue;
			size_t start = std::stoul(line.substr(0, 8), nullptr, 16);
			size_t end = std::stoul(line.substr(9, 8), nullptr, 16);
			addrs.push_back(std::make_pair<>(start, end));
		}
		
		if (addrs.size() == 0)
		{
			MemConPrintf("GetModuleAddress %s\n", modName.c_str());
			MemConPrintf("Module not found in %s\n", filename);
			return false;
		}
		
		std::sort(addrs.begin(), addrs.end());
		
		info.start = addrs[0].first;
		info.end = addrs[addrs.size() - 1].second;
		info.isValid = true;
		
		return true;
	}
	catch (const std::exception &e)
	{
		MemConPrintf("GetModuleAddress %s\n", modName.c_str());
		MemConPrintf("Exception occured: %s\n", e.what());
		return false;
	}
}

// Unlike dlsym, this function can get addr of non-exported symbols
// Based on AMX Mod X's MemoryUtils::ResolveSymbol
// https://github.com/alliedmodders/amxmodx/blob/master/public/memtools/MemoryUtils.cpp
void *Memory::ResolveSymbol(void *handle, const char *symbol)
{
	void *addr = dlsym(handle, symbol);

	if (addr)
	{
		return addr;
	}

	struct link_map *dlmap;
	struct stat dlstat;
	int dlfile;
	uintptr_t map_base;
	Elf32_Ehdr *file_hdr;
	Elf32_Shdr *sections, *shstrtab_hdr, *symtab_hdr, *strtab_hdr;
	Elf32_Sym *symtab;
	const char *shstrtab, *strtab;
	uint16_t section_count;
	uint32_t symbol_count;

	dlmap = (struct link_map *)handle;
	symtab_hdr = NULL;
	strtab_hdr = NULL;


	/* If symbol isn't in our table, then we have open the actual library */
	dlfile = open(dlmap->l_name, O_RDONLY);
	if (dlfile == -1 || fstat(dlfile, &dlstat) == -1)
	{
		close(dlfile);
		return NULL;
	}

	/* Map library file into memory */
	file_hdr = (Elf32_Ehdr *)mmap(NULL, dlstat.st_size, PROT_READ, MAP_PRIVATE, dlfile, 0);
	map_base = (uintptr_t)file_hdr;
	if (file_hdr == MAP_FAILED)
	{
		close(dlfile);
		return NULL;
	}
	close(dlfile);

	if (file_hdr->e_shoff == 0 || file_hdr->e_shstrndx == SHN_UNDEF)
	{
		munmap(file_hdr, dlstat.st_size);
		return NULL;
	}

	sections = (Elf32_Shdr *)(map_base + file_hdr->e_shoff);
	section_count = file_hdr->e_shnum;
	/* Get ELF section header string table */
	shstrtab_hdr = &sections[file_hdr->e_shstrndx];
	shstrtab = (const char *)(map_base + shstrtab_hdr->sh_offset);

	/* Iterate sections while looking for ELF symbol table and string table */
	for (uint16_t i = 0; i < section_count; i++)
	{
		Elf32_Shdr &hdr = sections[i];
		const char *section_name = shstrtab + hdr.sh_name;

		if (strcmp(section_name, ".symtab") == 0)
		{
			symtab_hdr = &hdr;
		}
		else if (strcmp(section_name, ".strtab") == 0)
		{
			strtab_hdr = &hdr;
		}
	}

	/* Uh oh, we don't have a symbol table or a string table */
	if (symtab_hdr == NULL || strtab_hdr == NULL)
	{
		munmap(file_hdr, dlstat.st_size);
		return NULL;
	}

	symtab = (Elf32_Sym *)(map_base + symtab_hdr->sh_offset);
	strtab = (const char *)(map_base + strtab_hdr->sh_offset);
	symbol_count = symtab_hdr->sh_size / symtab_hdr->sh_entsize;

	void *pFoundSymbol = nullptr;
	
	/* Iterate symbol table starting from the begining */
	for (uint32_t i = 0; i < symbol_count; i++)
	{
		Elf32_Sym &sym = symtab[i];
		unsigned char sym_type = ELF32_ST_TYPE(sym.st_info);
		const char *sym_name = strtab + sym.st_name;

		/* Skip symbols that are undefined or do not refer to functions or objects */
		if (sym.st_shndx == SHN_UNDEF || (sym_type != STT_FUNC && sym_type != STT_OBJECT))
		{
			continue;
		}

		if (strcmp(symbol, sym_name) == 0)
		{
			pFoundSymbol = (void *)(dlmap->l_addr + sym.st_value);
			break;
		}
	}

	munmap(file_hdr, dlstat.st_size);
	return pFoundSymbol;
}

//-----------------------------------------------------------------------------------------------------------
// Memory patching functions
//-----------------------------------------------------------------------------------------------------------
void Memory::LoadProtectFromProc()
{
	try
	{
		const char *filename = "/proc/self/maps";
		std::ifstream maps(filename);
		if (maps.fail())
		{
			MemConPrintf("LoadProtectFromProc()\n");
			MemConPrintf("Failed to open %s: %s\n", filename, strerror(errno));
		}
		
		std::string line;
		while (std::getline(maps, line))
		{
			size_t start = std::stoul(line.substr(0, 8), nullptr, 16);
			std::string protectStr = line.substr(19, 4);
			int protect = 0;
			for (char i : protectStr)
			{
				if (i == 'r')
					protect |= PROT_READ;
				else if (i == 'w')
					protect |= PROT_WRITE;
				else if (i == 'x')
					protect |= PROT_EXEC;
			}
			g_AddrToProtect[start] = protect;
		}
	}
	catch (const std::exception &e)
	{
		MemConPrintf("LoadProtectFromProc()\n");
		MemConPrintf("Exception occured: %s\n", e.what());
	}
}

int Memory::GetProtectForAddr(size_t addr)
{
	assert(g_AddrToProtect.size() > 0);
	auto it = g_AddrToProtect.upper_bound(addr);
	it--;
	return it->second;
}

uint32_t Memory::HookDWord(size_t origAddr, uint32_t newDWord)
{
	uint32_t origDWord = *reinterpret_cast<size_t *>(origAddr);
	mprotect(reinterpret_cast<void *>(origAddr & MPROTECT_PAGEMASK), MPROTECT_PAGESIZE, PROT_READ|PROT_WRITE|PROT_EXEC);	
	*(size_t *)origAddr = newDWord;
	mprotect(reinterpret_cast<void *>(origAddr & MPROTECT_PAGEMASK), MPROTECT_PAGESIZE, GetProtectForAddr(origAddr));	
	return origDWord;
}

void Memory::ExchangeMemoryBytes(size_t *origAddr, size_t *dataAddr, uint32_t size)
{
	mprotect(reinterpret_cast<void *>(reinterpret_cast<size_t>(origAddr) & MPROTECT_PAGEMASK), MPROTECT_PAGESIZE, PROT_READ|PROT_WRITE|PROT_EXEC);
	unsigned char data[MAX_PATTERN];
	int32_t iSize = size;
	while (iSize > 0)
	{
		size_t s = iSize <= MAX_PATTERN ? iSize : MAX_PATTERN;
		memcpy(data, origAddr, s);
		memcpy((void *)origAddr, (void *)dataAddr, s);
		memcpy((void *)dataAddr, data, s);
		iSize -= MAX_PATTERN;
	}
	mprotect(reinterpret_cast<void *>(reinterpret_cast<size_t>(origAddr) & MPROTECT_PAGEMASK), MPROTECT_PAGESIZE, GetProtectForAddr(reinterpret_cast<size_t>(origAddr)));
}

//-----------------------------------------------------------------------------------------------------------
// SVC messages hook
//-----------------------------------------------------------------------------------------------------------
bool Memory::InitSvcHook()
{
	if (!g_HwModule.handle)
	{
		MemConPrintf("Failed to get address of cl_parsefuncs from hw.so:\n");
		MemConPrintf("g_HwModule.handle is nullptr\n");
		return false;
	}
	
	g_pSvcMessagesTable = static_cast<svc_func_t *>(ResolveSymbol(g_HwModule.handle, "cl_parsefuncs"));
	if (!g_pSvcMessagesTable)
	{
		MemConPrintf("Failed to get address of cl_parsefuncs from hw.so\n");
		return false;
	}
	
	// Check first 3 elements just to be sure
	auto fnCheck = [](size_t idx, const char *name)
	{
		return strcmp(g_pSvcMessagesTable[idx].pszname, name) == 0;
	};
	
	if (!fnCheck(0, "svc_bad") || !fnCheck(1, "svc_nop") || !fnCheck(2, "svc_disconnect"))
	{
		g_pSvcMessagesTable = nullptr;
		MemConPrintf("cl_parsefuncs check has failed.\n");
		return false;
	}
	
	// Get addr of engine buffer
	g_pEngineBuffer = static_cast<sizebuf_t *>(ResolveSymbol(g_HwModule.handle, "net_message"));
	if (!g_pEngineBuffer)
	{
		MemConPrintf("Failed to get address of net_message from hw.so\n");
		return false;
	}
	
	g_EngineReadPos = static_cast<int *>(ResolveSymbol(g_HwModule.handle, "msg_readcount"));
	if (!g_EngineReadPos)
	{
		MemConPrintf("Failed to get address of msg_readcount from hw.so\n");
		return false;
	}
	
	g_EngineBuf = reinterpret_cast<void **>(&g_pEngineBuffer->data);
	g_EngineBufSize = &g_pEngineBuffer->cursize;
	
	// Get addr of UserMsg linked list
	g_pUserMessages = static_cast<UserMessage **>(ResolveSymbol(g_HwModule.handle, "gClientUserMsgs"));
	if (!g_pUserMessages)
	{
		MemConPrintf("Failed to get address of gClientUserMsgs from hw.so\n");
		return false;
	}
	
	return true;
}

void Memory::HookSvcMessages(cl_enginemessages_t *pEngineMessages)
{
	// Ensure we have all needed addresses
	if (!g_pSvcMessagesTable || !g_EngineBuf || !g_EngineBufSize || !g_EngineReadPos || !g_pUserMessages)
		return;
	// Iterate over registered messages chain and exchange message handlers
	int len = sizeof(cl_enginemessages_t) / 4;
	uint32_t *msgAsArray = reinterpret_cast<uint32_t *>(pEngineMessages);
	
	for (int i = 0; i < len; i++)
	{
		if (!msgAsArray[i])
			continue;
		size_t funcAddr = msgAsArray[i];
		size_t addr = reinterpret_cast<size_t>(g_pSvcMessagesTable) + i * 12 + 8;
		size_t oldAddr = HookDWord(addr, funcAddr);
		msgAsArray[i] = oldAddr;
	}
}

void Memory::UnHookSvcMessages(cl_enginemessages_t *pEngineMessages)
{
	// We just do same exchange for functions addresses
	HookSvcMessages(pEngineMessages);
}

//-----------------------------------------------------------------------------------------------------------
// GameUI patches
//-----------------------------------------------------------------------------------------------------------
void Memory::PatchGameUI()
{
	if (!g_GameUIModule.handle)
	{
		MemConPrintf("Failed to get address of g_pGameConsole from gameui.so:\n");
		MemConPrintf("g_GameUIModule.handle is nullptr\n");
		return;
	}
	
	g_pGameConsole = static_cast<CGameConsole003 *>(ResolveSymbol(g_GameUIModule.handle, "g_GameConsole"));
	if (!g_pGameConsole)
	{
		MemConPrintf("Failed to get address of g_pGameConsole from gameui.so\n");
		return;
	}
	
	// Pointer to m_PrintColor
	g_pPrintColor = reinterpret_cast<RGBA *>(reinterpret_cast<size_t>(g_pGameConsole->panel) + 0x124);
}

//-----------------------------------------------------------------------------------------------------------
// Memory init
//-----------------------------------------------------------------------------------------------------------
void Memory::OnLibraryInit()
{
}

void Memory::OnLibraryDeinit()
{
}

void Memory::OnHudInit()
{
	if (gEngfuncs.CheckParm("-nomempatch", nullptr) != 0)
	{
		MemConPrintf("Memory patching is disabled via '-nomempatch'\n");
		return;
	}
	
	MemConPrintf("Loading mprotect data from /proc/self/maps\n");
	LoadProtectFromProc();
	MemConPrintf("Loaded %u entries\n", g_AddrToProtect.size());
	
	GetModuleInfo("hw.so", g_HwModule);
	GetModuleInfo("valve/cl_dlls/gameui.so", g_GameUIModule);
	
	if (!InitSvcHook())
		MemConPrintf("Failed to initialize SVC message hook.\n");
	else
		::HookSvcMessages();
	PatchGameUI();
}

void Memory::OnFrame()
{
	if (!g_bMessagesPrinted)
	{
		gEngfuncs.Con_Printf("Memory patching results:\n");
		for (const std::string &i : g_DebugMsgs)
		{
			gEngfuncs.Con_Printf("%s", i.c_str());
		}
		g_bMessagesPrinted = true;
	}
}

RGBA SetConsoleColor(RGBA color)
{
	RGBA oldcolor = *Memory::g_pPrintColor;
	*Memory::g_pPrintColor = color;
	return oldcolor;
}
