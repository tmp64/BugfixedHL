#include "amxxmodule.h"
#include <IBugfixedServer.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

static IBugfixedServer *gServer = nullptr;

// native bhl_get_player_supports(idx)
static cell AMX_NATIVE_CALL bhl_get_player_supports(AMX *amx, cell *params)
{
	if (!gServer)
		return 0;
	int idx = params[1];
	if (idx < 0 || idx > 32)
		return 0;
	return (cell)gServer->GetPlayerSupports(idx);
}

// native bhl_is_color_text_enabled(idx)
static cell AMX_NATIVE_CALL bhl_is_color_text_enabled(AMX *amx, cell *params)
{
	if (!gServer)
		return 0;
	int idx = params[1];
	if (idx < 0 || idx > 32)
		return 0;
	return (cell)gServer->IsColoredTextEnabled(idx);
}

// native bhl_get_client_version_string(idx, buf[], size)
static cell AMX_NATIVE_CALL bhl_get_client_version_string(AMX *amx, cell *params)
{
	if (!gServer)
		return 0;
	int idx = params[1];
	int size = params[3];
	if (idx < 0 || idx > 32)
		return 0;
	char buf[64];
	bool result = gServer->GetClientVersionString(idx, buf, sizeof(buf));
	if (!result)
		return 0;
	MF_SetAmxString(amx, params[2], buf, size);
	return 1;
}

// native bhl_get_client_version(idx, &major, &minor, &patch)
static cell AMX_NATIVE_CALL bhl_get_client_version(AMX *amx, cell *params)
{
	if (!gServer)
		return 0;
	int idx = params[1];
	if (idx < 0 || idx > 32)
		return 0;
	
	cell *addr[3];
	addr[0] = MF_GetAmxAddr(amx, params[2]);
	addr[1] = MF_GetAmxAddr(amx, params[3]);
	addr[2] = MF_GetAmxAddr(amx, params[4]);

	return gServer->GetClientVersion(idx, *addr[0], *addr[1], *addr[2]);
}

// native bhl_is_client_modified(idx)
static cell AMX_NATIVE_CALL bhl_is_client_dirty(AMX *amx, cell *params)
{
	if (!gServer)
		return 0;
	int idx = params[1];
	if (idx < 0 || idx > 32)
		return 0;
	return (cell)gServer->IsClientDirty(idx);
}

// native bhl_get_client_version_commit(idx, buf[], size)
static cell AMX_NATIVE_CALL bhl_get_client_version_commit(AMX *amx, cell *params)
{
	if (!gServer)
		return 0;
	int idx = params[1];
	int size = params[3];
	if (idx < 0 || idx > 32)
		return 0;
	char buf[16];
	bool result = gServer->GetClientVersionCommit(idx, buf, sizeof(buf));
	if (!result)
		return 0;
	MF_SetAmxString(amx, params[2], buf, size);
	return 1;
}

AMX_NATIVE_INFO bugfixedapi_Exports[] =
{
	{ "bhl_get_player_supports", bhl_get_player_supports },
	{ "bhl_is_color_text_enabled", bhl_is_color_text_enabled },
	{ "bhl_get_client_version_string", bhl_get_client_version_string },
	{ "bhl_get_client_version", bhl_get_client_version },
	{ "bhl_is_client_dirty", bhl_is_client_dirty },
	{ "bhl_get_client_version_commit", bhl_get_client_version_commit },
	{ nullptr, nullptr }
};

bool GetInterfaceFromServer()
{
    GetServerInterfaceFunc pIfaceFunc = nullptr;
    
#ifdef _WIN32
	HMODULE hSrvDll = GetModuleHandle("hl.dll");
	if (!hSrvDll)
	{
		LOG_ERROR(PLID, "Failed to get IBugfixedServer from hl.dll:");
		LOG_ERROR(PLID, "\tGetModuleHandle returned invalid handle.");
		return false;
	}

	pIfaceFunc = (GetServerInterfaceFunc)GetProcAddress(hSrvDll, "AGHL_GetServerInterface");
	if (!pIfaceFunc)
	{
		LOG_ERROR(PLID, "Failed to get IBugfixedServer from hl.dll:");
		LOG_ERROR(PLID, "\tAGHL_GetServerInterface not found (GetProcAddress returned null).");
		return false;
	}
#else
	void *pSrvDll = nullptr;
	
	const char *fileNames[] = {
		"hl.so", "hl_i386.so", nullptr
	};
	
	for (int i = 0; fileNames[i]; i++)
	{
		pSrvDll = dlopen(fileNames[i], RTLD_NOW);
		if (pSrvDll)
		{
			LOG_MESSAGE(PLID, "dlopen('%s') successfully", fileNames[i]);
			break;
		}
		LOG_ERROR(PLID, "dlopen('%s') failed: %s", fileNames[i], dlerror());
	}
	
	if (!pSrvDll)
	return false;
	
	pIfaceFunc = (GetServerInterfaceFunc)dlsym(pSrvDll, "AGHL_GetServerInterface");
	
	if (!pIfaceFunc)
	{
		LOG_ERROR(PLID, "Failed to get AGHL_GetServerInterface from hl.so:");
		LOG_ERROR(PLID, "\t%s.", dlerror());
		return false;
	}
    
#endif
    
	int srvVersion = 0;
	gServer = (IBugfixedServer *)pIfaceFunc(BUGFIXEDSERVERIFACE_VERSION, &srvVersion);

	if (!gServer)
	{
		LOG_ERROR(PLID, "Failed to get IBugfixedServer from hl.dll:");
		LOG_ERROR(PLID, "\tAGHL_GetServerInterface returned nullptr.");
		LOG_ERROR(PLID, "Is server DLL too old?");
		LOG_ERROR(PLID, "Server interface version: %d", srvVersion);
		LOG_ERROR(PLID, "AMXX module interface version: %d", BUGFIXEDSERVERIFACE_VERSION);
		return false;
	}
	
	return true;
}

void OnAmxxAttach()
{
	MF_AddNatives(bugfixedapi_Exports);

	if (!GetInterfaceFromServer())
	{
		LOG_ERROR(PLID, "********************************************************");
		LOG_ERROR(PLID, "Failed to get IBugfixedServer from hl.dll.");
		LOG_ERROR(PLID, "Check log above for details.");
		LOG_ERROR(PLID, "All bhl_ AMXX natives will not function.");
		LOG_ERROR(PLID, "********************************************************");
		return;
	}

	LOG_MESSAGE(PLID, "Got IBugfixedServer from server library.");
	LOG_MESSAGE(PLID, "BugfixedAPI AMXX module loaded successfully.");
}

void OnAmxxDetach()
{
	gServer = nullptr;
}
