#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode/multimode_gamerules.h"
#include "game.h"
#include "demolition_mode.h"

static MMConfigVar<CDemolitionMode, int> mp_mm_demolition_hp("hp", 150);
static MMConfigVar<CDemolitionMode, int> mp_mm_demolition_limit("satchel_limit", 5);

CDemolitionMode::CDemolitionMode() : CBaseMode()
{
}

ModeID CDemolitionMode::GetModeID()
{
	return MODE_ID;
}

const char *CDemolitionMode::GetModeName()
{
	return MODE_NAME;
}

const char *CDemolitionMode::GetShortTitle()
{
	return "Demolition";
}

const char *CDemolitionMode::GetDescription()
{
	if (RANDOM_FLOAT(0, 1) <= 0.00007)
		return "Waht makes me a good demoman? If I were a bad demoman, I wouldn't be sittin' here discussin' it with ya now, would I?";
	else
		return "Use explosives to eliminate your enemies.\nRocket jumps are encouraged.";
}

void CDemolitionMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	pPlayer->GiveNamedItem("weapon_satchel");
}

void CDemolitionMode::PlayerThink(CBasePlayer *pPlayer)
{
	if (!pPlayer->IsAlive())
		return;

	CSatchel *pSatchel = dynamic_cast<CSatchel *>(pPlayer->m_pActiveItem);
	if (pSatchel)
	{
		if (pSatchel->m_chargeReady == 0) // SATCHEL_IDLE = 0
		{
			int ammo = pPlayer->m_rgAmmo[pSatchel->m_iPrimaryAmmoType];

			for (; ammo < 5; ammo++)
				pPlayer->GiveNamedItem("weapon_satchel");
		}
	}
	else if (!pPlayer->HasNamedPlayerItem("weapon_satchel"))
	{
		for (int i = 0; i < 5; i++)
			pPlayer->GiveNamedItem("weapon_satchel");
	}
}

void CDemolitionMode::PlayerSpawn(CBasePlayer *pPlayer)
{
	pPlayer->m_iMaxHealth = mp_mm_demolition_hp.Get();
	pPlayer->pev->max_health = mp_mm_demolition_hp.Get();
	pPlayer->pev->health = mp_mm_demolition_hp.Get();
}

bool CDemolitionMode::ShouldRespawnWeapons()
{
	return false;
}

bool CDemolitionMode::ShouldRespawnAmmo()
{
	return false;
}

int CDemolitionMode::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_NO;
}

int CDemolitionMode::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_NO;
}
