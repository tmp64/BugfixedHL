#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"
#include "biohazard_mode.h"

// Period in seconds in which snarks are given to players
ConVar mp_mm_biohaz_snark_respawn("mp_mm_biohaz_snark_respawn", "10");

CBiohazardMode::CBiohazardMode() : CBaseMode()
{
}

const char *CBiohazardMode::GetModeName()
{
	return "BiohazardMode";
}

const char *CBiohazardMode::GetShortTitle()
{
	return "Biohazard";
}

void CBiohazardMode::GetShortTitleColor(byte &r, byte &g, byte &b)
{
	// Green
	r = 87;
	g = 245;
	b = 66;
}

const char *CBiohazardMode::GetDescription()
{
	return "Only biological weapons are available";
}

void CBiohazardMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	if (RANDOM_FLOAT(0, 1) >= 0.5)
	{
		// 15 snarks
		pPlayer->GiveNamedItem("weapon_snark");
		pPlayer->GiveNamedItem("weapon_snark");
		pPlayer->GiveNamedItem("weapon_snark");
		m_Players[pPlayer->entindex()].wpnType = WeaponType::Snark;
	}
	else
	{
		pPlayer->GiveNamedItem("weapon_hornetgun");
		m_Players[pPlayer->entindex()].wpnType = WeaponType::Hornet;
	}
}

bool CBiohazardMode::ShouldRespawnWeapons()
{
	return false;
}

bool CBiohazardMode::ShouldRespawnAmmo()
{
	return false;
}

void CBiohazardMode::PlayerThink(CBasePlayer *pPlayer)
{
	if (!pPlayer->IsAlive())
		return;

	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("Hornets")] = 8;

	if (m_Players[pPlayer->entindex()].wpnType == WeaponType::Snark &&
		gpGlobals->time >= m_Players[pPlayer->entindex()].flNextTimerCheck)
	{
		int ammo = pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("Snarks")];
		//if (ammo < 15)
		//	ammo = 15;

		if (ammo < 15)
		{
			for (; ammo < 15; ammo += 5)
				pPlayer->GiveNamedItem("weapon_snark");
		}

		m_Players[pPlayer->entindex()].flNextTimerCheck = gpGlobals->time + mp_mm_biohaz_snark_respawn.Get();
	}
}

void CBiohazardMode::PlayerSpawn(CBasePlayer *pPlayer)
{
	m_Players[pPlayer->entindex()].flNextTimerCheck = gpGlobals->time + mp_mm_biohaz_snark_respawn.Get();
}
