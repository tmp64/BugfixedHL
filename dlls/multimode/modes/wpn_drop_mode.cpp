#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode/multimode_gamerules.h"
#include "game.h"
#include "wpn_drop_mode.h"

// Period in seconds in which weapons are regiven to players
static MMConfigVar<CWpnDropMode, float> mp_mm_wpndrop_respawn("respawn", 4.5f);

// Should revolver have infinite ammo
static MMConfigVar<CWpnDropMode, bool> mp_mm_wpndrop_infammo("infammo", true);

// Random angle variation
static MMConfigVar<CWpnDropMode, float> mp_mm_wpndrop_rndangle("rndangle", 60);

CWpnDropMode::CWpnDropMode() : CBaseMode()
{
}

ModeID CWpnDropMode::GetModeID()
{
	return MODE_ID;
}

const char *CWpnDropMode::GetModeName()
{
	return MODE_NAME;
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
	// Infinite 357 ammo
	if (mp_mm_wpndrop_infammo.Get() && pPlayer->m_pActiveItem &&
		pPlayer->m_pActiveItem->m_iId == WEAPON_PYTHON)
	{
		CPython *pWeapon = (CPython *)pPlayer->m_pActiveItem;
		pWeapon->m_iClip = 6;
	}

	// Drop active weapon
	g_pGameRules->GetNextBestWeapon(pPlayer, pWeapon);

	UTIL_MakeVectors(pPlayer->pev->angles);

	CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create("weaponbox",
		pPlayer->pev->origin + gpGlobals->v_forward * 10, pPlayer->pev->angles, pPlayer->edict());
	pWeaponBox->pev->angles.x = 0;
	pWeaponBox->pev->angles.z = 0;
	pWeaponBox->PackWeapon(pWeapon);

	Vector drop_dir = pPlayer->pev->angles;
	drop_dir.y += RANDOM_FLOAT(-1.0f, 1.0f) * mp_mm_wpndrop_rndangle.Get() / 2;
	if (drop_dir.y >= 360)
		drop_dir.y -= 360;
	else if (drop_dir.y < 0)
		drop_dir.y += 360;

	UTIL_MakeVectors(drop_dir);
	pWeaponBox->pev->velocity = gpGlobals->v_forward * 300 + gpGlobals->v_forward * 100;

	// drop half of the ammo for this weapon.
	int iAmmoIndex = pPlayer->GetAmmoIndex(pWeapon->pszAmmo1()); // ???

	if (iAmmoIndex != -1)
	{
		// this weapon weapon uses ammo, so pack an appropriate amount.
		if (pWeapon->iFlags() & ITEM_FLAG_EXHAUSTIBLE)
		{
			// pack up all the ammo, this weapon is its own ammo type
			pWeaponBox->PackAmmo(MAKE_STRING(pWeapon->pszAmmo1()), pPlayer->m_rgAmmo[iAmmoIndex]);
			pPlayer->m_rgAmmo[iAmmoIndex] = 0;
		}
		else
		{
			// pack half of the ammo
			int ammoDrop = pPlayer->m_rgAmmo[iAmmoIndex] / 2;
			pWeaponBox->PackAmmo(MAKE_STRING(pWeapon->pszAmmo1()), ammoDrop);
			pPlayer->m_rgAmmo[iAmmoIndex] -= ammoDrop;
		}
	}
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
