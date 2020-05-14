#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "basemode.h"

CBaseMode::~CBaseMode()
{
}

void CBaseMode::GetShortTitleColor(byte &r, byte &g, byte &b)
{
	// Default orange
	r = 252;
	g = 186;
	b = 3;
}

void CBaseMode::OnInit()
{
}

void CBaseMode::OnPostInit()
{
}

void CBaseMode::ValidateConfig(const nlohmann::json &json)
{
}

void CBaseMode::ApplyConfig(const nlohmann::json &json)
{
}

void CBaseMode::OnFreezeStart()
{
}

void CBaseMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	pPlayer->GiveNamedItem("weapon_crowbar");
	pPlayer->GiveNamedItem("weapon_9mmhandgun");
	pPlayer->GiveAmmo(68, "9mm", _9MM_MAX_CARRY);// 4 full reloads
}

void CBaseMode::OnStart()
{
}

void CBaseMode::OnEnd()
{
}

void CBaseMode::OnSwitchOff()
{
}

void CBaseMode::Think()
{
}

void CBaseMode::ClientDisconnected(edict_t *pClient)
{
}

void CBaseMode::PlayerSpawn(CBasePlayer *pPlayer)
{
}

void CBaseMode::PlayerThink(CBasePlayer *pPlayer)
{
}

void CBaseMode::OnPrimaryAttack(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
}

bool CBaseMode::PlayerCanRespawn(CBasePlayer *pPlayer)
{
	return true;
}

float CBaseMode::PlayerSpawnTime(CBasePlayer *pPlayer)
{
	return gpGlobals->time;	// Now!
}

edict_t *CBaseMode::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	return nullptr;
}

int CBaseMode::PointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	return 1;
}

void CBaseMode::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
}

bool CBaseMode::ShouldRespawnWeapons()
{
	return true;
}

bool CBaseMode::ShouldRespawnWeaponsAfterPickUp()
{
	return true;
}

bool CBaseMode::ShouldRespawnAmmo()
{
	return true;
}

bool CBaseMode::ShouldRespawnAmmoAfterPickUp()
{
	return true;
}

bool CBaseMode::ShouldRespawnItems()
{
	return true;
}

bool CBaseMode::ShouldRespawnItemsAfterPickUp()
{
	return true;
}

bool CBaseMode::ShouldRespawnWeapon(const char *classname)
{
	return true;
}

float CBaseMode::GetWeaponRespawnTime()
{
	return WEAPON_RESPAWN_TIME;
}

float CBaseMode::GetAmmoRespawnTime()
{
	return AMMO_RESPAWN_TIME;
}

float CBaseMode::GetItemRespawnTime()
{
	return ITEM_RESPAWN_TIME;
}

bool CBaseMode::CanHaveWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	return true;
}

void CBaseMode::PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
}

bool CBaseMode::CanHaveItem(CBasePlayer *pPlayer, CItem *pItem)
{
	return true;
}

void CBaseMode::PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem)
{
}

bool CBaseMode::PlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	return true;
}

float CBaseMode::PlayerFallDamage(CBasePlayer *pPlayer)
{
	return -1;
}

float CBaseMode::HealthChargerRechargeTime()
{
	return -1;
}

float CBaseMode::SuitChargerRechargeTime()
{
	return -1;
}

int CBaseMode::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_ACTIVE;
}

int CBaseMode::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_ACTIVE;
}

CBaseMode::CBaseMode()
{
}