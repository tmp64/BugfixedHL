#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode/multimode_gamerules.h"
#include "game.h"
#include "dm_mode.h"

CDmMode::CDmMode() : CBaseMode()
{
}

ModeID CDmMode::GetModeID()
{
	return MODE_ID;
}

const char *CDmMode::GetModeName()
{
	return MODE_NAME;
}

const char *CDmMode::GetShortTitle()
{
	return "Deathmatch";
}

const char *CDmMode::GetDescription()
{
	return "Standart HLDM. Just for testing.";
}
