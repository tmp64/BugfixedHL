#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode/multimode_gamerules.h"
#include "game.h"
#include "biohazard_mode.h"

EHANDLE GetSnarkOwner(CBaseEntity *pEnt);

// Maximum count of snarks a player can have deployed
static MMConfigVar<CBiohazardMode, int> cfg_snark_count("snark_count", 10);

CBiohazardMode::CBiohazardMode() : CBaseMode()
{
}

ModeID CBiohazardMode::GetModeID()
{
	return MODE_ID;
}

const char *CBiohazardMode::GetModeName()
{
	return MODE_NAME;
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

void CBiohazardMode::OnFreezeStart()
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		m_Players[i] = PlayerInfo();
	}
}

void CBiohazardMode::OnSnarkSpawn(CBaseEntity *pSnark, CBasePlayer *pPlayer)
{
	int idx = pPlayer->entindex();
	if (m_Players[idx].snarkCount > 0)
		m_Players[idx].snarkCount--;
}

void CBiohazardMode::OnSnarkDeath(CBaseEntity *pSnark, CBasePlayer *pPlayer)
{
	int idx = pPlayer->entindex();
	m_Players[idx].snarkCount++;
}

void CBiohazardMode::ClientDisconnected(edict_t *pClient)
{
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pClient);

	KillAllSnarks(pPlayer);
	m_Players[pPlayer->entindex()] = PlayerInfo();
}

void CBiohazardMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	PlayerInfo &plInfo = m_Players[pPlayer->entindex()];
	WeaponType oldWpnType = plInfo.wpnType;
	WeaponType newWpnType = WeaponType::None;

	if (RANDOM_FLOAT(0, 1) >= 0.5)
	{
		pPlayer->GiveNamedItem("weapon_snark");
		newWpnType = WeaponType::Snark;
	}
	else
	{
		pPlayer->GiveNamedItem("weapon_hornetgun");
		newWpnType = WeaponType::Hornet;
	}

	if (oldWpnType == WeaponType::Snark && newWpnType != WeaponType::Snark)
	{
		// Kill any remaining snarks
		KillAllSnarks(pPlayer);
	}

	if (newWpnType == WeaponType::Snark && oldWpnType != WeaponType::Snark)
	{
		// Reset snark counter
		plInfo.snarkCount = cfg_snark_count.Get();
	}

	plInfo.wpnType = newWpnType;
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

	int idx = pPlayer->entindex();

	if (m_Players[idx].wpnType == WeaponType::Snark)
	{
		ASSERT(m_Players[idx].snarkCount >= 0);

		int &ammo = pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("Snarks")];

		if (ammo == 0 && m_Players[idx].snarkCount != 0)
		{
			pPlayer->GiveNamedItem("weapon_snark");
		}

		ammo = m_Players[idx].snarkCount;
	}
	else if (m_Players[idx].wpnType == WeaponType::Hornet)
	{
		pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("Hornets")] = 8;
	}
}

void CBiohazardMode::KillAllSnarks(CBasePlayer *pPlayer)
{
	CBaseEntity *i = UTIL_FindEntityByClassname(nullptr, "monster_snark");
	for (; i; i = UTIL_FindEntityByClassname(i, "monster_snark"))
	{
		if (GetSnarkOwner(i) == pPlayer)
			i->Killed(nullptr, GIB_ALWAYS);
	}
}
