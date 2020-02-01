#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "items.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"

#include "dm_mode.h"
#include "oneshot_mode.h"
#include "recoil_mode.h"
#include "wpn_drop_mode.h"
#include "warmup_mode.h"

extern ConVar mp_multimode;

ConVar mp_mm_min_players("mp_mm_min_players", "1");
ConVar mp_mm_warmup_time("mp_mm_warmup_time", "10");
ConVar mp_mm_freeze_time("mp_mm_freeze_time", "5");
ConVar mp_mm_game_time("mp_mm_game_time", "30");

CHalfLifeMultimode::CHalfLifeMultimode() : CHalfLifeMultiplay()
{
	// Init HUD texts
	m_WarmupTextParams.x = -1.0f;
	m_WarmupTextParams.y = 0.25f;
	m_WarmupTextParams.effect = 0;
	m_WarmupTextParams.r1 = 255;
	m_WarmupTextParams.g1 = 255;
	m_WarmupTextParams.b1 = 255;
	m_WarmupTextParams.fadeinTime = 0.1f;
	m_WarmupTextParams.fadeoutTime = 0.1f;
	m_WarmupTextParams.holdTime = 0.9f;
	m_WarmupTextParams.fxTime = 0.9f;

	m_TimerTextParams.x = -1.0f;
	m_TimerTextParams.y = 0.05f;
	m_TimerTextParams.effect = 0;
	m_TimerTextParams.r1 = 0xF2;	// Orange color
	m_TimerTextParams.g1 = 0x9D;
	m_TimerTextParams.b1 = 0x35;
	m_TimerTextParams.fadeinTime = 0.1f;
	m_TimerTextParams.fadeoutTime = 0.1f;
	m_TimerTextParams.holdTime = 0.9f;
	m_TimerTextParams.fxTime = 0.9f;

	m_ModeTitleTextParams.x = -1.0f;
	m_ModeTitleTextParams.effect = 0;
	m_ModeTitleTextParams.fadeinTime = 0.3f;
	m_ModeTitleTextParams.fadeoutTime = 0.6f;
	m_ModeTitleTextParams.holdTime = mp_mm_freeze_time.Get() - 0.6f;
	m_ModeTitleTextParams.fxTime = mp_mm_freeze_time.Get() - 0.6f;

	m_ModeTitleTextParams.y = 0.3f;

	m_ModeInfoTextParams = m_ModeTitleTextParams;
	m_ModeInfoTextParams.y = 0.35f;
	m_ModeInfoTextParams.r1 = 255;
	m_ModeInfoTextParams.g1 = 255;
	m_ModeInfoTextParams.b1 = 255;

	// Do not give crowbar and glock
	m_bGiveDefaultWeapons = false;

	// Back up skill data
	m_DefSkillData = gSkillData;

	// Warmup
	m_pWarmupMode = new CWarmupMode();

	// Create mode instances
	//m_pModes[(int)ModeID::DM] = new CDmMode();
	m_pModes[(int)ModeID::OneShot] = new COneshotMode();
	m_pModes[(int)ModeID::Recoil] = new CRecoilMode();
	m_pModes[(int)ModeID::WpnDrop] = new CWpnDropMode();
}

void CHalfLifeMultimode::SwitchToWaiting()
{
	if (m_State == State::Waiting)
		return;

	if (m_State == State::Warmup)
	{
		m_State = State::Waiting;
		return;
	}

	FinishCurMode();
	m_State = State::Waiting;
	m_pCurMode = m_pWarmupMode;
	BeginCurMode(false);
}

void CHalfLifeMultimode::SwitchToWarmup()
{
	if (m_State == State::Warmup)
		return;

	m_flWarmupEndTime = gpGlobals->time + mp_mm_warmup_time.Get();

	if (m_State == State::Waiting)
	{
		m_State = State::Warmup;
		return;
	}

	FinishCurMode();
	m_State = State::Warmup;
	m_pCurMode = m_pWarmupMode;
	BeginCurMode(false);
}

void CHalfLifeMultimode::SwitchToNextMode()
{
	FinishCurMode();

	m_CurModeId = (ModeID)((int)m_CurModeId + 1);
	if (m_CurModeId == ModeID::ModeCount)
		m_CurModeId = (ModeID)1;

	// Skip commented modes
	while (!m_pModes[(int)m_CurModeId])
		m_CurModeId = (ModeID)((int)m_CurModeId + 1);

	m_pCurMode = m_pModes[(int)m_CurModeId];

	m_State = State::Game;
	BeginCurMode(true);
}

void CHalfLifeMultimode::BeginCurMode(bool bEnableFreezeTime)
{
	ALERT(at_console, "Preparing to run [%d] %s\n", (int)m_CurModeId, m_pCurMode->GetModeName());

	// Respawn stuff
	// TODO: Remove corpses?
	edict_t *pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		const char *classname = STRING(pEdict->v.classname);

		if (!strncmp("weapon_", classname, 7))
		{
			CBaseEntity *pBase = CBaseEntity::Instance(pEdict);
			CBasePlayerItem *pWeapon = dynamic_cast<CBasePlayerItem *>(pBase);
			if (pWeapon)
			{
				if (m_pCurMode->ShouldRespawnWeapons() && m_pCurMode->ShouldRespawnWeapon(classname))
				{
					if (pWeapon->pev->effects & EF_NODRAW)
					{
						pWeapon->pev->nextthink = gpGlobals->time + 0;
					}
				}
				else
				{
					pWeapon->pev->effects |= EF_NODRAW;// invisible for now
					pWeapon->SetTouch(NULL);// no touch
					pWeapon->SetThink(&CBasePlayerItem::AttemptToMaterialize);
					pWeapon->pev->nextthink = gpGlobals->time + 9999999;
				}
			}
		}
		else if (!strncmp("item_", classname, 5))
		{
			CBaseEntity *pBase = CBaseEntity::Instance(pEdict);
			CItem *pItem = dynamic_cast<CItem *>(pBase);
			if (pItem)
			{
				if (m_pCurMode->ShouldRespawnItems())
					pItem->Materialize();
				else
					pItem->Respawn();	// Would call FlItemRespawnTime() and never respawn
			}
		}
		else if (!strncmp("ammo_", classname, 5))
		{
			CBaseEntity *pBase = CBaseEntity::Instance(pEdict);
			CBasePlayerAmmo *pAmmo = dynamic_cast<CBasePlayerAmmo *>(pBase);
			if (pAmmo)
			{
				if (m_pCurMode->ShouldRespawnAmmo())
				{
					// Call m_pCurMode->ShouldRespawnWeapon to check
					// if we should respawn ammo
					bool should = false;

					if (!_stricmp(classname, "ammo_9mmclip") ||
						!_stricmp(classname, "ammo_glockclip") ||
						!_stricmp(classname, "ammo_mp5clip") ||
						!_stricmp(classname, "ammo_9mmAR") ||
						!_stricmp(classname, "ammo_9mmbox"))
						should = m_pCurMode->ShouldRespawnWeapon("weapon_mp5") || m_pCurMode->ShouldRespawnWeapon("weapon_glock");
					else if (!_stricmp(classname, "ammo_mp5grenades") ||
						!_stricmp(classname, "ammo_ARgrenades"))
						should = m_pCurMode->ShouldRespawnWeapon("weapon_mp5");
					else if (!_stricmp(classname, "ammo_357"))
						should = m_pCurMode->ShouldRespawnWeapon("weapon_357");
					else if (!_stricmp(classname, "ammo_buckshot"))
						should = m_pCurMode->ShouldRespawnWeapon("weapon_shotgun");
					else if (!_stricmp(classname, "ammo_crossbow"))
						should = m_pCurMode->ShouldRespawnWeapon("weapon_crossbow");
					else if (!_stricmp(classname, "ammo_rpgclip"))
						should = m_pCurMode->ShouldRespawnWeapon("weapon_rpg");
					else if (!_stricmp(classname, "ammo_gaussclip"))
						should = m_pCurMode->ShouldRespawnWeapon("weapon_gauss") || m_pCurMode->ShouldRespawnWeapon("weapon_egon");

					if (should)
						pAmmo->Materialize();
					else
					{
						// Hide
						pAmmo->pev->effects |= EF_NODRAW;
						pAmmo->SetTouch(NULL);
						UTIL_SetOrigin(pAmmo->pev, g_pGameRules->VecAmmoRespawnSpot(pAmmo));// move to wherever I'm supposed to repawn.
						pAmmo->SetThink(&CBasePlayerAmmo::Materialize);
						pAmmo->pev->nextthink = gpGlobals->time + 99999999;
					}
				}
				else
					pAmmo->Respawn();	// Would call FlAmmoRespawnTime() and never respawn
			}
		}
		else if (!strcmp("weaponbox", classname))
		{
			CWeaponBox *pEnt = (CWeaponBox *)CBaseEntity::Instance(pEdict);
			pEnt->Kill();
		}
	}

	// Respawn all players
	m_bFreezeOnSpawn = true;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		edict_t *pEdict = INDEXENT(i);
		if (!pEdict || pEdict->free)
			continue;
		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pEdict);

		if (pPlayer)
		{
			if (IsSpectator(pPlayer))
				continue;

			pEdict->v.health = 0;
			pPlayer->Spawn(); // See PlayerSpawn
		}
	}

	m_bFreezeOnSpawn = false;

	// Restore skill data
	gSkillData = m_DefSkillData;

	// Reset timer updates
	m_flNextTimerUpdate = gpGlobals->time + 0;

	if (m_State != State::Waiting && m_State != State::Warmup)
	{
		// Show mode info message
		m_pCurMode->GetShortTitleColor(m_ModeTitleTextParams.r1, m_ModeTitleTextParams.g1, m_ModeTitleTextParams.b1);
		UTIL_DirectorHudMessageAll(m_ModeTitleTextParams, m_pCurMode->GetShortTitle(), true);
		UTIL_DirectorHudMessageAll(m_ModeInfoTextParams, m_pCurMode->GetDescription(), true);

		m_pCurMode->OnFreezeStart();

		if (bEnableFreezeTime)
		{
			m_State = State::FreezeTime;
			m_flFreezeEndTime = gpGlobals->time + mp_mm_freeze_time.Get();
			m_iFreezeNextSec = mp_mm_freeze_time.Get();
		}
		else
		{
			StartCurMode();
		}
	}
	else
	{
		// Start immediately
		m_pCurMode->OnFreezeStart();
		StartCurMode();
	}
}

void CHalfLifeMultimode::StartCurMode()
{
	ALERT(at_console, "Running mode [%d] %s\n", (int)m_CurModeId, m_pCurMode->GetModeName());

	if (m_State == State::FreezeTime)
	{
		m_State = State::Game;
		m_flEndTime = gpGlobals->time + mp_mm_game_time.Get();
	}

	m_pCurMode->OnStart();

	// Unfreeze players and play sound on all clients
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		edict_t *pEdict = INDEXENT(i);

		if (!pEdict || pEdict->free)
			continue;

		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pEdict);
		if (pPlayer)
		{
			if (!pPlayer->m_bIsBot)
				CLIENT_COMMAND(pEdict, "spk buttons/bell1.wav\n");

			if (IsSpectator(pPlayer))
				continue;

			pPlayer->m_flNextAttack = 0;
			pPlayer->pev->flags &= ~FL_FROZEN;
		}
	}
}

void CHalfLifeMultimode::FinishCurMode()
{
	if (m_pCurMode)
	{
		m_pCurMode->OnEnd();
	}

	m_pCurMode = nullptr;
}

bool CHalfLifeMultimode::IsSpectator(CBasePlayer *pPlayer)
{
	return (pPlayer->pev->iuser1 || pPlayer->pev->iuser2 || pPlayer->m_bInWelcomeCam);
}

const char *CHalfLifeMultimode::GetGameDescription()
{
	return "HL Multimode";
}

void CHalfLifeMultimode::Think()
{
	BaseClass::Think();
	
	if (m_pCurMode)
	{
		m_pCurMode->Think();
	}

	switch (m_State)
	{
	case State::Invalid:
	{
		SwitchToWaiting();
		break;
	}
	case State::Waiting:
	{
		if (gpGlobals->time >= m_flNextTimerUpdate)
		{
			// TODO: Show "Waiting for players" message
			// TODO: and check for player count


			// Count connected players
			int iPlayerCount = 0;
			int iMinPlayerCount = mp_mm_min_players.Get();
			for (int i = 1; i <= gpGlobals->maxClients; i++)
			{
				edict_t *pEdict = INDEXENT(i);
				if (!pEdict || pEdict->free)
					continue;

				CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pEdict);
				if (pPlayer && !pPlayer->m_bIsBot)
					iPlayerCount++;
			}

			if (iPlayerCount >= iMinPlayerCount)
			{
				// Start warmup
				SwitchToWarmup();
			}
			else
			{
				// Show hud message
				char buf[128];
				snprintf(buf, sizeof(buf), "Waiting for players [%d / %d]", iPlayerCount, iMinPlayerCount);
				UTIL_DirectorHudMessageAll(m_WarmupTextParams, buf, false);
			}

			m_flNextTimerUpdate = gpGlobals->time + 1;
		}

		break;
	}
	case State::Warmup:
	{
		if (gpGlobals->time >= m_flNextTimerUpdate)
		{
			char buf[128];
			int seconds = m_flWarmupEndTime - gpGlobals->time;
			snprintf(buf, sizeof(buf), "Warm-up [%d:%02d]", seconds / 60, seconds % 60);
			UTIL_DirectorHudMessageAll(m_WarmupTextParams, buf, false);
			m_flNextTimerUpdate = gpGlobals->time + 1;
		}

		if (gpGlobals->time >= m_flWarmupEndTime)
		{
			SwitchToNextMode();
		}

		break;
	}
	case State::FreezeTime:
	{
		if (gpGlobals->time >= m_flFreezeEndTime)
		{
			StartCurMode();
		}
		else if (gpGlobals->time >= m_flNextTimerUpdate)
		{
			const char *szSuitVoice[10] = {
				nullptr,
				"fvox/one.wav",
				"fvox/two.wav",
				"fvox/three.wav",
				"fvox/four.wav",
				"fvox/five.wav",
				"fvox/six.wav",
				"fvox/seven.wav",
				"fvox/eight.wav",
				"fvox/nine.wav"
			};

			// Play sound on all clients
			for (int i = 1; i <= gpGlobals->maxClients; i++)
			{
				edict_t *pEdict = INDEXENT(i);

				if (!pEdict || pEdict->free)
					continue;

				CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pEdict);
				if (pPlayer)
				{
					if (m_iFreezeNextSec >= 1 && m_iFreezeNextSec <= 9 && !pPlayer->m_bIsBot)
					{
						CLIENT_COMMAND(pEdict, "spk %s\n", szSuitVoice[m_iFreezeNextSec]);
					}
				}
			}

			m_iFreezeNextSec--;
			m_flNextTimerUpdate = gpGlobals->time + 1;
		}

		break;
	}
	case State::Game:
	{
		// Update timer
		if (gpGlobals->time >= m_flNextTimerUpdate)
		{
			char buf[128];
			int seconds = m_flEndTime - gpGlobals->time;
			snprintf(buf, sizeof(buf), "%d:%02d", seconds / 60, seconds % 60);
			UTIL_DirectorHudMessageAll(m_TimerTextParams, buf, false);
			m_flNextTimerUpdate = gpGlobals->time + 1;
		}

		if (gpGlobals->time >= m_flEndTime)
		{
			SwitchToNextMode();
		}

		break;
	}
	}
}

void CHalfLifeMultimode::ClientDisconnected(edict_t *pClient)
{
	if (m_pCurMode)
	{
		m_pCurMode->ClientDisconnected(pClient);
	}

	BaseClass::ClientDisconnected(pClient);
}

float CHalfLifeMultimode::FlPlayerFallDamage(CBasePlayer *pPlayer)
{
	if (m_pCurMode)
	{
		float dmg = m_pCurMode->PlayerFallDamage(pPlayer);
		if (dmg != -1)
			return dmg;
	}

	return BaseClass::FlPlayerFallDamage(pPlayer);
}

BOOL CHalfLifeMultimode::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	if (m_pCurMode)
	{
		return m_pCurMode->PlayerCanTakeDamage(pPlayer, pAttacker);
	}

	return false;
}

void CHalfLifeMultimode::PlayerSpawn(CBasePlayer *pPlayer)
{
	BaseClass::PlayerSpawn(pPlayer);

	if (IsSpectator(pPlayer))
		return;

	if (m_State == State::FreezeTime || m_bFreezeOnSpawn)
	{
		// Disable attack
		pPlayer->m_flNextAttack = gpGlobals->time + 9999999;

		// Freeze
		pPlayer->pev->flags |= FL_FROZEN;

		DROP_TO_FLOOR(pPlayer->edict());
	}

	// Regive weapons
	pPlayer->RemoveAllItems(false);

	if (m_pCurMode)
	{
		m_pCurMode->GivePlayerWeapons(pPlayer);
		m_pCurMode->PlayerSpawn(pPlayer);
	}
}

void CHalfLifeMultimode::PlayerThink(CBasePlayer *pPlayer)
{
	BaseClass::PlayerThink(pPlayer);

	if (m_pCurMode)
	{
		m_pCurMode->PlayerThink(pPlayer);
	}
}

BOOL CHalfLifeMultimode::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	if (m_pCurMode)
	{
		return m_pCurMode->PlayerCanRespawn(pPlayer);
	}

	return false;
}

float CHalfLifeMultimode::FlPlayerSpawnTime(CBasePlayer *pPlayer)
{
	if (m_pCurMode)
	{
		return m_pCurMode->PlayerSpawnTime(pPlayer);
	}

	return BaseClass::FlPlayerSpawnTime(pPlayer);
}

edict_t *CHalfLifeMultimode::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	if (m_pCurMode)
	{
		edict_t *p = m_pCurMode->GetPlayerSpawnSpot(pPlayer);
		if (p)
			return p;
	}

	return BaseClass::GetPlayerSpawnSpot(pPlayer);
}

int CHalfLifeMultimode::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	if (m_pCurMode)
	{
		return m_pCurMode->PointsForKill(pAttacker, pKilled);
	}

	return BaseClass::IPointsForKill(pAttacker, pKilled);
}

void CHalfLifeMultimode::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	BaseClass::PlayerKilled(pVictim, pKiller, pInflictor);

	if (m_pCurMode)
	{
		m_pCurMode->PlayerKilled(pVictim, pKiller, pInflictor);
	}
}

BOOL CHalfLifeMultimode::CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	bool can = BaseClass::CanHavePlayerItem(pPlayer, pWeapon);
	if (!can)
		return false;

	if (m_pCurMode)
	{
		return m_pCurMode->CanHaveWeapon(pPlayer, pWeapon);
	}

	return can;
}

void CHalfLifeMultimode::PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	BaseClass::PlayerGotWeapon(pPlayer, pWeapon);

	if (m_pCurMode)
	{
		m_pCurMode->PlayerGotWeapon(pPlayer, pWeapon);
	}
}

int CHalfLifeMultimode::WeaponShouldRespawn(CBasePlayerItem *pWeapon)
{
	return BaseClass::WeaponShouldRespawn(pWeapon);
}

float CHalfLifeMultimode::FlWeaponRespawnTime(CBasePlayerItem *pWeapon)
{
	if (m_pCurMode)
	{
		if (m_pCurMode->ShouldRespawnWeapons() && m_pCurMode->ShouldRespawnWeaponsAfterPickUp())
			return gpGlobals->time + m_pCurMode->GetWeaponRespawnTime();
		else
			return gpGlobals->time + 9999999;
	}

	return BaseClass::FlWeaponRespawnTime(pWeapon);
}

float CHalfLifeMultimode::FlWeaponTryRespawn(CBasePlayerItem *pWeapon)
{
	// TODO
	return BaseClass::FlWeaponTryRespawn(pWeapon);
}

Vector CHalfLifeMultimode::VecWeaponRespawnSpot(CBasePlayerItem *pWeapon)
{
	return BaseClass::VecWeaponRespawnSpot(pWeapon);
}

BOOL CHalfLifeMultimode::CanHaveItem(CBasePlayer *pPlayer, CItem *pItem)
{
	if (m_pCurMode)
	{
		return m_pCurMode->CanHaveItem(pPlayer, pItem);
	}

	return BaseClass::CanHaveItem(pPlayer, pItem);
}

void CHalfLifeMultimode::PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem)
{
	BaseClass::PlayerGotItem(pPlayer, pItem);

	if (m_pCurMode)
	{
		m_pCurMode->PlayerGotItem(pPlayer, pItem);
	}
}

int CHalfLifeMultimode::ItemShouldRespawn(CItem *pItem)
{
	return BaseClass::ItemShouldRespawn(pItem);
}

float CHalfLifeMultimode::FlItemRespawnTime(CItem *pItem)
{
	if (m_pCurMode)
	{
		if (m_pCurMode->ShouldRespawnItems() && m_pCurMode->ShouldRespawnItemsAfterPickUp())
			return gpGlobals->time + m_pCurMode->GetItemRespawnTime();
		else
			return gpGlobals->time + 9999999;
	}

	return BaseClass::FlItemRespawnTime(pItem);
}

Vector CHalfLifeMultimode::VecItemRespawnSpot(CItem *pItem)
{
	return BaseClass::VecItemRespawnSpot(pItem);
}

BOOL CHalfLifeMultimode::CanHaveAmmo(CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry)
{
	return BaseClass::CanHaveAmmo(pPlayer, pszAmmoName, iMaxCarry);
}

void CHalfLifeMultimode::PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount)
{
	BaseClass::PlayerGotAmmo(pPlayer, szName, iCount);
}

int CHalfLifeMultimode::AmmoShouldRespawn(CBasePlayerAmmo *pAmmo)
{
	return BaseClass::AmmoShouldRespawn(pAmmo);
}

float CHalfLifeMultimode::FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo)
{
	if (m_pCurMode)
	{
		if (m_pCurMode->ShouldRespawnAmmo() && m_pCurMode->ShouldRespawnAmmoAfterPickUp())
			return gpGlobals->time + m_pCurMode->GetAmmoRespawnTime();
		else
			return gpGlobals->time + 9999999;
	}

	return BaseClass::FlAmmoRespawnTime(pAmmo);
}

Vector CHalfLifeMultimode::VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo)
{
	return BaseClass::VecAmmoRespawnSpot(pAmmo);
}

float CHalfLifeMultimode::FlHealthChargerRechargeTime()
{
	if (m_pCurMode)
	{
		return m_pCurMode->HealthChargerRechargeTime();
	}

	return BaseClass::FlHealthChargerRechargeTime();
}

float CHalfLifeMultimode::FlHEVChargerRechargeTime()
{
	if (m_pCurMode)
	{
		return m_pCurMode->SuitChargerRechargeTime();
	}

	return BaseClass::FlHEVChargerRechargeTime();
}

int CHalfLifeMultimode::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	if (m_pCurMode)
	{
		return m_pCurMode->DeadPlayerWeapons(pPlayer);
	}

	return BaseClass::DeadPlayerWeapons(pPlayer);
}

int CHalfLifeMultimode::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	if (m_pCurMode)
	{
		return m_pCurMode->DeadPlayerAmmo(pPlayer);
	}

	return BaseClass::DeadPlayerAmmo(pPlayer);
}

void CHalfLifeMultimode::OnPrimaryAttack(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	if (m_pCurMode)
	{
		return m_pCurMode->OnPrimaryAttack(pPlayer, pWeapon);
	}
}

bool IsRunningMultimode()
{
	return mp_multimode.operator bool();
}

bool IsRunningMultimode(ModeID mode)
{
	return (mp_multimode && static_cast<CHalfLifeMultimode *>(g_pGameRules)->m_CurModeId == mode);
}

CHalfLifeMultimode *GetMultimodeGR()
{
	ASSERT(mp_multimode.operator bool());
	return static_cast<CHalfLifeMultimode *>(g_pGameRules);
}
