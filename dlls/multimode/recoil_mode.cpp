#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"
#include "recoil_mode.h"

struct wpn_t
{
	const char *ent;
	float rnd;
};

// weapon : probability of spawn
// If probability is zero, this weapon is never given when spawning
// the player but will be allowed to be spawned in the world
static wpn_t s_Wpns[] = {
	{ "weapon_glock", 0.05 },
	{ "weapon_357", 0.15 },
	{ "weapon_shotgun", 0.4 },
	{ "weapon_mp5", 0.4 },

	{ "weapon_crowbar", 0 },
};

CRecoilMode::CRecoilMode() : CBaseMode()
{
	// Prepare s_Wpns
	float sum = 0;
	for (int i = 0; i < HLARRAYSIZE(s_Wpns); i++)
	{
		if (s_Wpns[i].rnd != 0)
		{
			sum += s_Wpns[i].rnd;
			s_Wpns[i].rnd = sum;
		}
	}
}

const char *CRecoilMode::GetModeName()
{
	return "RecoilMode";
}

const char *CRecoilMode::GetShortTitle()
{
	return "Extreme Recoil";
}

const char *CRecoilMode::GetDescription()
{
	return "All weapons push you back by a lot.";
}

void CRecoilMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	// Give crowbar
	pPlayer->GiveNamedItem("weapon_crowbar");

	// Give random weapon
	float rnd = RANDOM_FLOAT(0, 1);
	int i;
	for (i = 0; i < HLARRAYSIZE(s_Wpns); i++)
	{
		if (rnd <= s_Wpns[i].rnd || fabs(rnd - s_Wpns[i].rnd) <= 0.001)
			break;
	}
	pPlayer->GiveNamedItem(s_Wpns[i].ent);

	// Give a lot of ammo
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("buckshot")] = 125;
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("9mm")] = 250;
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("357")] = 36;
}

bool CRecoilMode::ShouldRespawnWeapon(const char *classname)
{
	for (int i = 0; i < HLARRAYSIZE(s_Wpns); i++)
	{
		if (!_stricmp(classname, s_Wpns[i].ent))
			return true;
	}

	// I guess Valve couldn't agree how to call the guns so some have multiple names
	if (!_stricmp(classname, "weapon_9mmAR") ||
		!_stricmp(classname, "weapon_9mmhandgun") ||
		!_stricmp(classname, "weapon_python"))
		return true;

	return false;
}
