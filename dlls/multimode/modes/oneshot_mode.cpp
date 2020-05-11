#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode/multimode_gamerules.h"
#include "game.h"
#include "oneshot_mode.h"
#include "skill.h"

COneshotMode::COneshotMode() : CBaseMode()
{
}

ModeID COneshotMode::GetModeID()
{
	return MODE_ID;
}

const char *COneshotMode::GetModeName()
{
	return MODE_NAME;
}

const char *COneshotMode::GetShortTitle()
{
	return "One-Shot Gun";
}

void COneshotMode::GetShortTitleColor(byte &r, byte &g, byte &b)
{
	// Red
	r = 252;
	g = 53;
	b = 3;
}

const char *COneshotMode::GetDescription()
{
	return "Shotgun deals massive damage.";
}

void COneshotMode::OnFreezeStart()
{
	gSkillData.plrDmgBuckshot = 10000;
}

void COneshotMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	pPlayer->GiveNamedItem("weapon_shotgun");
}

bool COneshotMode::ShouldRespawnWeapons()
{
	return false;
}

bool COneshotMode::ShouldRespawnAmmo()
{
	return false;
}

void COneshotMode::Think()
{
}

void COneshotMode::PlayerThink(CBasePlayer *pPlayer)
{
	// Infinite shotgun ammo
	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("buckshot")] = 125;
}

int COneshotMode::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_NO;
}

int COneshotMode::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_NO;
}
