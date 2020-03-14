#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"
#include "wpn_drop_mode.h"

// Period in seconds in which weapons are regiven to players
ConVar mp_mm_wpndrop_respawn("mp_mm_wpndrop_respawn", "4.5");

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
	if (!pPlayer->IsAlive())
		return;

	// Infinite ammo
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("357")] = 36;

	// Check weapon
	int idx = pPlayer->entindex();
	float &time = m_flGiveWeaponTime[idx];

	if (time == 0)
	{
		if (!pPlayer->HasNamedPlayerItem("weapon_357"))
		{
			time = gpGlobals->time + mp_mm_wpndrop_respawn.Get();
		}
	}
	else
	{
		if (pPlayer->HasNamedPlayerItem("weapon_357"))
		{
			time = 0;
		}
		else if (gpGlobals->time >= time)
		{
			pPlayer->GiveNamedItem("weapon_357");
		}
	}
}
