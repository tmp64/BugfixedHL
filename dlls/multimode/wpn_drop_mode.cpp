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
	return "WpnDropMode";
}

const char *CWpnDropMode::GetShortTitle()
{
	return "Heavy Weapons";
}

const char *CWpnDropMode::GetDescription()
{
	return "Gun is so heavy you drop it after each shot.";
}

void CWpnDropMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	pPlayer->GiveNamedItem("weapon_357");
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

void CWpnDropMode::PlayerThink(CBasePlayer *pPlayer)
{
	// Infinite ammo
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("357")] = 36;
}
