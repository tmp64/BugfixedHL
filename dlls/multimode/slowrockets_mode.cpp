#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"
#include "slowrockets_mode.h"
#include "skill.h"

// Period in seconds in which rockets are given to players
ConVar mp_mm_rocket_respawn("mp_mm_rocket_respawn", "10");

// Max speed of the rockets
ConVar mp_mm_rocket_speed("mp_mm_rocket_speed", "120");

CSlowRocketsMode::CSlowRocketsMode() : CBaseMode()
{
}

const char *CSlowRocketsMode::GetModeName()
{
	return "SlowRocketsMode";
}

const char *CSlowRocketsMode::GetShortTitle()
{
	return "Slow Rockets";
}

void CSlowRocketsMode::GetShortTitleColor(byte &r, byte &g, byte &b)
{
	// TODO
	CBaseMode::GetShortTitleColor(r, g, b);
}

const char *CSlowRocketsMode::GetDescription()
{
	return "RPG rockets are S-L-O-W but do more damage.";
}

void CSlowRocketsMode::OnFreezeStart()
{
	gSkillData.plrDmgRPG = 160;
}

void CSlowRocketsMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	pPlayer->GiveNamedItem("weapon_rpg");
	pPlayer->GiveNamedItem("ammo_rpgclip");
	m_flNextCheckTime[pPlayer->entindex()] = gpGlobals->time + mp_mm_rocket_respawn.Get();
}

bool CSlowRocketsMode::ShouldRespawnWeapons()
{
	return false;
}

bool CSlowRocketsMode::ShouldRespawnAmmo()
{
	return false;
}

void CSlowRocketsMode::PlayerThink(CBasePlayer *pPlayer)
{
	if (!pPlayer->IsAlive())
		return;

	if (gpGlobals->time >= m_flNextCheckTime[pPlayer->entindex()])
	{
		int ammo = pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("rockets")];

		if (ammo < 5)
		{
			for (; ammo < 5; ammo += 2)
				pPlayer->GiveNamedItem("ammo_rpgclip");
		}

		m_flNextCheckTime[pPlayer->entindex()] = gpGlobals->time + mp_mm_rocket_respawn.Get();
	}
}

int CSlowRocketsMode::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_NO;
}

int CSlowRocketsMode::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_NO;
}
