//
// memory_linux.cpp
//
// Memory pathcer for Linux
//

#include "hud.h"
#include "com_utils.h"

void Memory::OnLibraryInit()
{
}

void Memory::OnLibraryDeinit()
{
}

void Memory::OnHudInit()
{
	gEngfuncs.pfnConsolePrint("Memory patching failed: platform not supported\n");
}

void Memory::OnFrame()
{
}


RGBA SetConsoleColor(RGBA color)
{
	return color;
}
