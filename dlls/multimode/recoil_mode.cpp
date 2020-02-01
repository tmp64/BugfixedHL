#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"
#include "recoil_mode.h"

CRecoilMode::CRecoilMode() : CBaseMode()
{
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
	struct wpn_t
	{
		const char *ent;
		float rnd;
	};

	wpn_t wpns[] = {
		{ "weapon_pistol", 0.1 },
		{ "weapon_357", 0.3 },
		{ "weapon_shotgun", 0.3 },
		{ "weapon_mp5", 0.3 },
	};

	float sum = 0;
	for (int i = 0; i < HLARRAYSIZE(wpns); i++)
	{
		sum += wpns[i].rnd;
		wpns[i].rnd = sum;
	}

	float rnd = RANDOM_FLOAT(0, 1);
	int i;
	for (i = 0; i < HLARRAYSIZE(wpns); i++)
	{
		if (rnd <= wpns[i].rnd || fabs(rnd - wpns[i].rnd) <= 0.001)
			break;
	}
	pPlayer->GiveNamedItem(wpns[i].ent);

	// Give a lot of ammo
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("buckshot")] = 125;
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("9mm")] = 250;
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("357")] = 36;
}
