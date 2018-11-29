//
// memory_stub.cpp
//
// Stub memory patcher (does nothing)
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
