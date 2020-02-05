#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"
#include "warmup_mode.h"

CWarmupMode::CWarmupMode() : CBaseMode()
{
}

const char *CWarmupMode::GetModeName()
{
	return "WarmupMode";
}

const char *CWarmupMode::GetShortTitle()
{
	return "Unused";
}

const char *CWarmupMode::GetDescription()
{
	return "Unused";
}

void CWarmupMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	pPlayer->GiveNamedItem("weapon_crowbar");
	pPlayer->GiveNamedItem("item_longjump");
}

bool CWarmupMode::ShouldRespawnWeapons()
{
	return false;
}

bool CWarmupMode::ShouldRespawnAmmo()
{
	return false;
}

int CWarmupMode::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_NO;
}

int CWarmupMode::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_NO;
}
