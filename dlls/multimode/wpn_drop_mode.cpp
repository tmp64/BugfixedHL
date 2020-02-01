#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"
#include "wpn_drop_mode.h"

CWpnDropMode::CWpnDropMode() : CBaseMode()
{
}

const char *CWpnDropMode::GetModeName()
{
	return "CWpnDropMode";
}

const char *CWpnDropMode::GetShortTitle()
{
	return "TODO: Unnamed";
}

const char *CWpnDropMode::GetDescription()
{
	return "Weapon drops after each shot.";
}

void CWpnDropMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	pPlayer->GiveNamedItem("weapon_357");
	pPlayer->GiveNamedItem("ammo_357");
	pPlayer->GiveNamedItem("ammo_357");
	pPlayer->GiveNamedItem("ammo_357");
}

bool CWpnDropMode::ShouldRespawnWeapons()
{
	return false;
}

bool CWpnDropMode::ShouldRespawnAmmo()
{
	return false;
}

void CWpnDropMode::OnPrimaryAttack(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	// Drop active weapon
	pPlayer->DropPlayerItem("");
}
