#include <string>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "items.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"

#include "modes/dm_mode.h"
#include "modes/oneshot_mode.h"
#include "modes/recoil_mode.h"
#include "modes/wpn_drop_mode.h"
#include "modes/warmup_mode.h"
#include "modes/biohazard_mode.h"
#include "modes/slowrockets_mode.h"
#include "modes/speed_mode.h"
#include "modes/boss_mode.h"

extern int g_multimode;

constexpr float ERROR_MESSAGE_PERIOD = 5;

ConVar mp_mm_config_file("mp_mm_config_file", "hl_multimode.json");

ConVar mp_mm_skip_warmup("mp_mm_skip_warmup", "0");
ConVar mp_mm_skip_mode("mp_mm_skip_mode", "0");

struct MultimodePlayerScore
{
	int idx;
	int score;
	int deaths;
};

static MultimodePlayerScore s_PlayerScores[MAX_PLAYERS + 1];

static int SortMultimodePlayerScore(const void *plhs, const void *prhs)
{
	MultimodePlayerScore &lhs = *(MultimodePlayerScore *)plhs;
	MultimodePlayerScore &rhs = *(MultimodePlayerScore *)prhs;
	if (lhs.score > rhs.score)
		return -1;
	if (lhs.score < rhs.score)
		return 1;

	if (lhs.deaths < rhs.deaths)
		return -1;
	if (lhs.deaths > rhs.deaths)
		return 1;

	return 0;
}

//-------------------------------------------------------------------
// Mode Manager
//-------------------------------------------------------------------
CHalfLifeMultimode::CModeManager::CModeManager()
{
	RegisterMode<CWarmupMode>();
	RegisterMode<CDmMode>();
	RegisterMode<COneshotMode>();
	RegisterMode<CRecoilMode>();
	RegisterMode<CWpnDropMode>();
	RegisterMode<CBiohazardMode>();
	RegisterMode<CSlowRocketsMode>();
	RegisterMode<CSpeedMode>();
	RegisterMode<CBossMode>();
}

CHalfLifeMultimode::CModeManager::~CModeManager()
{
	for (CBaseMode *i : m_ModeList)
	{
		delete i;
	}

	m_ModeList.clear();
}

void CHalfLifeMultimode::CModeManager::PrepareMode(ModeID mode, bool bShowModeInfo)
{
	SwitchOffMode();

	m_ActiveModeID = mode;
	m_pActiveMode = GetModeForID(mode);

	UTIL_LogPrintf("Preparing to run %s [%d]\n", m_pActiveMode->GetModeName(), (int)m_ActiveModeID);

	PrepareWorld();

	// Restore skill data
	gSkillData = GetMultimodeGR()->m_DefSkillData;

	m_pActiveMode->OnFreezeStart();

	if (bShowModeInfo)
	{
		hudtextparms_t &titleParams = GetMultimodeGR()->m_ModeTitleTextParams;
		hudtextparms_t &infoParams = GetMultimodeGR()->m_ModeInfoTextParams;

		// Show mode info message
		m_pActiveMode->GetShortTitleColor(titleParams.r1, titleParams.g1, titleParams.b1);
		UTIL_DirectorHudMessageAll(titleParams, m_pActiveMode->GetShortTitle(), true);
		UTIL_DirectorHudMessageAll(infoParams, m_pActiveMode->GetDescription(), true);
	}

	// Respawn all players
	GetMultimodeGR()->m_bFreezeOnSpawn = true;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		edict_t *pEdict = INDEXENT(i);
		if (!pEdict || pEdict->free)
			continue;
		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pEdict);

		if (pPlayer)
		{
			if (pPlayer->IsSpectator())
				continue;

			pEdict->v.health = 0;
			pPlayer->Spawn(); // See PlayerSpawn
		}
	}
	GetMultimodeGR()->m_bFreezeOnSpawn = false;
}

void CHalfLifeMultimode::CModeManager::StartMode()
{
	ASSERT(m_pActiveMode);
	UTIL_LogPrintf("Running mode %s [%d]\n", m_pActiveMode->GetModeName(), (int)m_ActiveModeID);
	m_pActiveMode->OnStart();
}

void CHalfLifeMultimode::CModeManager::FinishMode()
{
	ASSERT(m_pActiveMode);
	m_pActiveMode->OnEnd();
}

void CHalfLifeMultimode::CModeManager::SwitchOffMode()
{
	if (m_pActiveMode)
	{
		m_pActiveMode->OnSwitchOff();
	}

	m_pActiveMode = nullptr;
}

void CHalfLifeMultimode::CModeManager::PrepareWorld()
{
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
				if (m_pActiveMode->ShouldRespawnWeapons() && m_pActiveMode->ShouldRespawnWeapon(classname))
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
				if (m_pActiveMode->ShouldRespawnItems())
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
				if (m_pActiveMode->ShouldRespawnAmmo())
				{
					// Call m_pCurMode->ShouldRespawnWeapon to check
					// if we should respawn ammo
					bool should = false;

					if (!_stricmp(classname, "ammo_9mmclip") ||
						!_stricmp(classname, "ammo_glockclip") ||
						!_stricmp(classname, "ammo_mp5clip") ||
						!_stricmp(classname, "ammo_9mmAR") ||
						!_stricmp(classname, "ammo_9mmbox"))
						should = m_pActiveMode->ShouldRespawnWeapon("weapon_mp5") || m_pActiveMode->ShouldRespawnWeapon("weapon_glock");
					else if (!_stricmp(classname, "ammo_mp5grenades") ||
						!_stricmp(classname, "ammo_ARgrenades"))
						should = m_pActiveMode->ShouldRespawnWeapon("weapon_mp5");
					else if (!_stricmp(classname, "ammo_357"))
						should = m_pActiveMode->ShouldRespawnWeapon("weapon_357");
					else if (!_stricmp(classname, "ammo_buckshot"))
						should = m_pActiveMode->ShouldRespawnWeapon("weapon_shotgun");
					else if (!_stricmp(classname, "ammo_crossbow"))
						should = m_pActiveMode->ShouldRespawnWeapon("weapon_crossbow");
					else if (!_stricmp(classname, "ammo_rpgclip"))
						should = m_pActiveMode->ShouldRespawnWeapon("weapon_rpg");
					else if (!_stricmp(classname, "ammo_gaussclip"))
						should = m_pActiveMode->ShouldRespawnWeapon("weapon_gauss") || m_pActiveMode->ShouldRespawnWeapon("weapon_egon");

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
		else if (!strcmp("monster_snark", classname) ||
			!strcmp("hornet", classname))
		{
			CBaseMonster *pEnt = (CBaseMonster *)CBaseEntity::Instance(pEdict);
			pEnt->Killed(nullptr, GIB_NEVER);
		}
	}
}

//-------------------------------------------------------------------
// State Machine
//-------------------------------------------------------------------
void CHalfLifeMultimode::CStateMachine::CBaseState::OnSwitchTo(State oldState)
{
}

void CHalfLifeMultimode::CStateMachine::CBaseState::OnSwitchFrom(State newState)
{
}

CHalfLifeMultimode::CStateMachine::CStateMachine()
{
	AddState(State::Initial, new CInitialState());
	AddState(State::InvalidConfig, new CInvalidConfigState());
	AddState(State::Waiting, new CWaitingState());
	AddState(State::Warmup, new CWarmupState());
	AddState(State::FreezeTime, new CFreezeTimeState());
	AddState(State::Game, new CGameState());
	AddState(State::Intermission, new CIntermissionState());
	AddState(State::Endgame, new CEndgameState());
	AddState(State::FinalIntermission, new CFinalIntermissionState());

	m_ActiveStateID = (State)-1;
	SwitchTo(State::Initial);
}

CHalfLifeMultimode::CStateMachine::~CStateMachine()
{
	for (int i = 0; i < (int)State::StateCount; i++)
	{
		delete m_States[i];
	}
}

void CHalfLifeMultimode::CStateMachine::AddState(State stateId, CBaseState *stateObj)
{
	m_States[(int)stateId] = stateObj;
}

void CHalfLifeMultimode::CStateMachine::SwitchTo(State newStateId)
{
	if (m_ActiveStateID == newStateId)
	{
		return;
	}

	CBaseState *oldState = m_pActiveState;
	CBaseState *newState = GetStateObj(newStateId);

#ifdef _DEBUG
	UTIL_LogPrintf("CStateMachine: Switching from %d to %d\n", (int)m_ActiveStateID, (int)newStateId);
#endif

	if (oldState)
	{
		oldState->OnSwitchFrom(newStateId);
	}

	if (newState)
	{
		newState->ResetTimerUpdate();
		newState->OnSwitchTo(m_ActiveStateID);
	}

	m_ActiveStateID = newStateId;
	m_pActiveState = newState;
}

void CHalfLifeMultimode::CStateMachine::Think()
{
	if (m_pActiveState)
	{
		m_pActiveState->Think();
	}
}

//-------------------------------------------------------------------
// Initial State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CInitialState::Think()
{
	GetMultimodeGR()->m_StateMachine.SwitchTo(State::Waiting);
}

//-------------------------------------------------------------------
// Invalid Config State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CInvalidConfigState::Think()
{
	if (gpGlobals->time >= m_flNextTimerUpdate)
	{
		UTIL_ColoredClientPrintAll(HUD_PRINTTALK, "^1[Multimode]: ^6ERROR: ^8Server configuration is invalid.");
		UTIL_ColoredClientPrintAll(HUD_PRINTTALK, "^1[Multimode]: ^6ERROR: ^8Check server log.");
		m_flNextTimerUpdate = gpGlobals->time + ERROR_MESSAGE_PERIOD;
	}
}

void CHalfLifeMultimode::CInvalidConfigState::OnSwitchTo(State oldState)
{
	GetMultimodeGR()->PrepareMode(ModeID::WarmUp, false);
}

//-------------------------------------------------------------------
// Waiting For Players State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CWaitingState::Think()
{
	if (gpGlobals->time >= m_flNextTimerUpdate)
	{
		// Count connected players
		int iPlayerCount = 0;
		int iMinPlayerCount = GetMultimodeGR()->m_ParsedConfig.minPlayers;
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
			GetMultimodeGR()->m_StateMachine.SwitchTo(State::Warmup);
		}
		else
		{
			// Show hud message
			char buf[128];
			snprintf(buf, sizeof(buf), "Waiting for players [%d / %d]", iPlayerCount, iMinPlayerCount);
			UTIL_DirectorHudMessageAll(GetMultimodeGR()->m_WarmupTextParams, buf, false);
		}

		m_flNextTimerUpdate = gpGlobals->time + 1;
	}
}

void CHalfLifeMultimode::CWaitingState::OnSwitchTo(State oldState)
{
	if (oldState == State::Warmup)
	{
		return;
	}

	GetMultimodeGR()->PrepareMode(ModeID::WarmUp, false);
}

//-------------------------------------------------------------------
// Warmup State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CWarmupState::Think()
{
	if (gpGlobals->time >= m_flNextTimerUpdate)
	{
		char buf[128];
		int seconds = m_flWarmupEndTime - gpGlobals->time;
		snprintf(buf, sizeof(buf), "Warm-up [%d:%02d]", seconds / 60, seconds % 60);
		UTIL_DirectorHudMessageAll(GetMultimodeGR()->m_WarmupTextParams, buf, false);
		m_flNextTimerUpdate = gpGlobals->time + 1;
	}

	if (gpGlobals->time >= m_flWarmupEndTime)
	{
		GetMultimodeGR()->PrepareNextMode();
	}
}

void CHalfLifeMultimode::CWarmupState::OnSwitchTo(State oldState)
{
	m_flWarmupEndTime = gpGlobals->time + GetMultimodeGR()->m_ParsedConfig.warmupTime;

	if (oldState == State::Waiting)
	{
		return;
	}

	GetMultimodeGR()->PrepareMode(ModeID::WarmUp, false);
}

//-------------------------------------------------------------------
// Freeze Time State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CFreezeTimeState::Think()
{
	if (gpGlobals->time >= m_flFreezeEndTime)
	{
		GetMultimodeGR()->StartMode();
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
}

void CHalfLifeMultimode::CFreezeTimeState::OnSwitchTo(State oldState)
{
	m_flFreezeEndTime = gpGlobals->time + GetMultimodeGR()->m_ParsedConfig.freezeTime;
	m_iFreezeNextSec = GetMultimodeGR()->m_ParsedConfig.freezeTime;
}

//-------------------------------------------------------------------
// Game State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CGameState::Think()
{
	// Update timer
	if (gpGlobals->time >= m_flNextTimerUpdate)
	{
		char buf[128];
		int seconds = m_flEndTime - gpGlobals->time;
		snprintf(buf, sizeof(buf), "%d:%02d", seconds / 60, seconds % 60);
		UTIL_DirectorHudMessageAll(GetMultimodeGR()->m_TimerTextParams, buf, false);

		if (seconds <= 4)
		{
			// Play blip sound to signal the end
			const char *snd;

			if (seconds <= 1)
				snd = "blip2";
			else
				snd = "blip1";

			for (int i = 1; i <= gpGlobals->maxClients; i++)
			{
				edict_t *pEdict = INDEXENT(i);

				if (!pEdict || pEdict->free)
					continue;

				CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pEdict);
				if (pPlayer && !pPlayer->m_bIsBot)
					CLIENT_COMMAND(pEdict, "spk buttons/%s.wav\n", snd);
			}
		}

		m_flNextTimerUpdate = gpGlobals->time + 1;
	}

	if (gpGlobals->time >= m_flEndTime)
	{
		GetMultimodeGR()->FinishMode();
	}
}

void CHalfLifeMultimode::CGameState::OnSwitchTo(State oldState)
{
	GetMultimodeGR()->m_ModeManager.StartMode();

	m_flEndTime = gpGlobals->time + GetMultimodeGR()->GetGameTime();

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

			pPlayer->pev->flags &= ~FL_FROZEN;
			pPlayer->pev->flags &= ~FL_GODMODE;

			if (!pPlayer->IsSpectator())
			{
				pPlayer->m_flNextAttack = 0;
				pPlayer->m_iMultimodeScore = 0;
				pPlayer->m_iMultimodeDeaths = 0;
			}
		}
	}
}

//-------------------------------------------------------------------
// Intermission State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CIntermissionState::Think()
{
	if (gpGlobals->time >= m_flIntermEndTime)
	{
		GetMultimodeGR()->PrepareNextMode();
	}
}

void CHalfLifeMultimode::CIntermissionState::OnSwitchTo(State oldState)
{
	m_flIntermEndTime = gpGlobals->time + GetMultimodeGR()->m_ParsedConfig.intermTime;

	GetMultimodeGR()->m_ModeManager.FinishMode();

	// Freeze all players by changing max speed
	// (for slow down effect instead of immediate freeze)
	UTIL_SetPlayerMaxSpeed(0.01);

	int iPlayerCount = 0;
	int iPlayerScoreCount = 0;
	int iPlayerScoresIdx = 1;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			if (!pPlayer->IsSpectator())
			{
				// Disable weapons
				pPlayer->m_flNextAttack = gpGlobals->time + 99999999;	// never

				// Give god mode
				pPlayer->pev->flags |= FL_GODMODE;
			}

			// Play round end sound
			CLIENT_COMMAND(pPlayer->edict(), "spk buttons/button8.wav\n");

			// Set in score array
			if (pPlayer->m_iMultimodeScore != MULTIMODE_NO_SCORE)
			{
				s_PlayerScores[iPlayerScoresIdx].idx = i;
				s_PlayerScores[iPlayerScoresIdx].score = pPlayer->m_iMultimodeScore;
				s_PlayerScores[iPlayerScoresIdx].deaths = pPlayer->m_iMultimodeDeaths;

				iPlayerScoreCount++;
				iPlayerScoresIdx++;
			}

			iPlayerCount++;
		}
	}

	// Sort scores
	if (iPlayerCount > 0)
	{
		qsort(s_PlayerScores + 1, iPlayerScoreCount, sizeof(s_PlayerScores[0]), SortMultimodePlayerScore);

		// Show message
		std::string str;

		if (iPlayerCount < 2 || (iPlayerScoreCount >= 2 && s_PlayerScores[1].score == s_PlayerScores[2].score))
			str = "^3Draw!^0\n\n";
		else
			str = STRING(UTIL_PlayerByIndex(s_PlayerScores[1].idx)->pev->netname) + std::string("^0 is the winner!\n\n");

		for (int i = 1, len = min(iPlayerScoreCount, 7); i <= len; i++)
		{
			char buf[128];
			const char *playerName = STRING(UTIL_PlayerByIndex(s_PlayerScores[i].idx)->pev->netname);
			snprintf(buf, sizeof(buf), "%d. %s^0 - %d\n", i, playerName, s_PlayerScores[i].score);
			str += buf;
		}

		UTIL_ColoredDirectorHudMessageAll(GetMultimodeGR()->m_IntermStatsTextParams, str.c_str(), true);
	}
}

void CHalfLifeMultimode::CIntermissionState::OnSwitchFrom(State newState)
{
	UTIL_ResetPlayerMaxSpeed();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			// Reenable weapons
			pPlayer->m_flNextAttack = 0;

			// Take god mode
			pPlayer->pev->flags &= ~FL_GODMODE;
		}
	}
}

//-------------------------------------------------------------------
// Endgame State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CEndgameState::Think()
{
}

void CHalfLifeMultimode::CEndgameState::OnSwitchTo(State oldState)
{
	GetMultimodeGR()->PrepareMode(ModeID::WarmUp, false);

	// Sort scores
	int iPlayerScoresIdx = 1;
	int iPlayerCount = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			// Set in score array
			s_PlayerScores[iPlayerScoresIdx].idx = i;
			s_PlayerScores[iPlayerScoresIdx].score = pPlayer->pev->frags;
			s_PlayerScores[iPlayerScoresIdx].deaths = pPlayer->m_iDeaths;

			iPlayerCount++;
			iPlayerScoresIdx++;
		}
	}

	UTIL_ColoredClientPrintAll(HUD_PRINTTALK, "^2* The game has finished!");

	if (iPlayerCount > 0)
	{
		qsort(s_PlayerScores + 1, iPlayerCount, sizeof(s_PlayerScores[0]), SortMultimodePlayerScore);

		char buf[512];
		CBasePlayer *pWinner = (CBasePlayer *)UTIL_PlayerByIndex(s_PlayerScores[1].idx);

		// Say to all
		const char *playerName = STRING(pWinner->pev->netname);
		snprintf(buf, sizeof(buf), "^2* %s^8 is the winner with %d kills/%d deaths.",
			playerName, (int)pWinner->pev->frags, pWinner->m_iDeaths
		);

		UTIL_ColoredClientPrintAll(HUD_PRINTTALK, buf);

		// Say to the winner
		ColoredClientPrint(pWinner, HUD_PRINTTALK, "^1* Congratulations! ^8You have won the game!");
	}
}

//-------------------------------------------------------------------
// Final Intermission State
//-------------------------------------------------------------------
void CHalfLifeMultimode::CFinalIntermissionState::Think()
{
}

//-------------------------------------------------------------------
// Game Rules
//-------------------------------------------------------------------
CHalfLifeMultimode::CHalfLifeMultimode() : CHalfLifeMultiplay()
{
	// Do not give crowbar and glock
	m_bGiveDefaultWeapons = false;

	// Back up skill data
	m_DefSkillData = gSkillData;

	// Precache resources
	PRECACHE_SOUND("hl_multimode/crit_hit.wav");
	PRECACHE_SOUND("hl_multimode/crit_received.wav");

	// Remove timelimit
	g_engfuncs.pfnCvar_DirectSet(&timelimit, "0");

	// Load config file
	try
	{
		nlohmann::json config = LoadConfigFile();
		ApplyConfigFile(config);
	}
	catch (const std::exception &e)
	{
		UTIL_LogPrintf("[Multimode] ERROR: Failed to load config file.\n");
		UTIL_LogPrintf("[Multimode] ERROR: File: %s\n", mp_mm_config_file.GetString());
		UTIL_LogPrintf("[Multimode] ERROR: Message:\n");
		UTIL_LogPrintf("[Multimode] ERROR: %s\n", e.what());
		m_StateMachine.SwitchTo(State::InvalidConfig);
	}

	for (CBaseMode *mode : m_ModeManager.GetModeList())
	{
		mode->OnPostInit();
	}
}

CHalfLifeMultimode::~CHalfLifeMultimode()
{
}

void CHalfLifeMultimode::PrepareNextMode(bool bShowModeInfo)
{
	m_ModeManager.SwitchOffMode();

	// TODO: Implement queue
	static int iNextModeIdx = 0;
	auto &list = m_ModeManager.GetModeList();
	PrepareMode(list[iNextModeIdx]->GetModeID(), bShowModeInfo);
	iNextModeIdx++;
	if (iNextModeIdx == list.size())
	{
		iNextModeIdx = 0;
	}

	m_StateMachine.SwitchTo(State::FreezeTime);
}

void CHalfLifeMultimode::PrepareMode(ModeID modeId, bool bShowModeInfo)
{
	m_ModeManager.PrepareMode(modeId, bShowModeInfo);
}

void CHalfLifeMultimode::StartMode()
{
	m_StateMachine.SwitchTo(State::Game);
}

void CHalfLifeMultimode::FinishMode()
{
	m_StateMachine.SwitchTo(State::Intermission);
}

void CHalfLifeMultimode::SwitchOffMode()
{
	m_ModeManager.SwitchOffMode();
}

const char *CHalfLifeMultimode::GetGameDescription()
{
	return "HL Multimode";
}

void CHalfLifeMultimode::Think()
{
	BaseClass::Think();
	
	if (GetBaseMode())
	{
		GetBaseMode()->Think();
	}

	// Handle skip cvars
	if (mp_mm_skip_warmup)
	{
		mp_mm_skip_warmup.Set(0.0);
		if (GetState() == State::Warmup)
			PrepareNextMode();
	}
	else if (mp_mm_skip_mode.Get() > 0)
	{
		mp_mm_skip_mode.Set(mp_mm_skip_mode.Get() - 1);
		PrepareNextMode();
	}

	m_StateMachine.Think();
}

float CHalfLifeMultimode::GetGameTime()
{
	// TODO:
	return m_ParsedConfig.gameTime;
}

nlohmann::json CHalfLifeMultimode::LoadConfigFile()
{
	const char *configPath = mp_mm_config_file.GetString();
	char *pFile = (char *)LOAD_FILE_FOR_ME((char *)configPath, nullptr);

	if (!pFile)
		throw std::runtime_error(std::string("config file '") + configPath + "' failed to load");

	try
	{
		nlohmann::json config = nlohmann::json::parse(pFile);

		if (pFile)
			FREE_FILE(pFile);

		return config;
	}
	catch (const std::exception &)
	{
		if (pFile)
			FREE_FILE(pFile);
		throw;
	}
}

void CHalfLifeMultimode::ApplyConfigFile(const nlohmann::json &config)
{
	ParsedConfig mmParsedCfg;

	// Validate "multimode" config
	try
	{
		const nlohmann::json &mm = config.at("multimode");
		mmParsedCfg.minPlayers = mm.at("min_players").get<int>();
		mmParsedCfg.warmupTime = mm.at("warmup_time").get<int>();
		mmParsedCfg.freezeTime = mm.at("freeze_time").get<int>();
		mmParsedCfg.gameTime = mm.at("game_time").get<int>();
		mmParsedCfg.intermTime = mm.at("interm_time").get<int>();

		std::string onEnd = mm.at("on_end").get<std::string>();
		if (onEnd == "start_over")
			mmParsedCfg.onEnd = EndAction::StartOver;
		else if (onEnd == "restart")
			mmParsedCfg.onEnd = EndAction::Restart;
		else
			throw std::runtime_error("multimode.on_end contains invalid value '" + onEnd + "'");
	}
	catch (const std::exception &e)
	{
		throw std::runtime_error(std::string("while validating 'multimode': ") + e.what());
	}

	// Validate mode config
	const nlohmann::json &mode_configs = config.at("modes");
	for (CBaseMode *mode : m_ModeManager.GetModeList())
	{
		// FIXME: HACK
		if (!mode)
			continue;
		try
		{
			auto it = mode_configs.find(mode->GetModeName());
			if (it != mode_configs.end())
			{
				const nlohmann::json &item = *it;

				// Validate vars
				for (MMConfigVarBase *var : MMConfigVarBase::GetModeVars(mode->GetModeID()))
				{
					auto it2 = item.find(var->GetName());
					if (it2 != item.end() && !var->ValidateValue(*it2))
					{
						throw std::runtime_error(std::string("'") + var->GetName() + "': invalid value type");
					}
				}

				mode->ValidateConfig(item);
			}
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error(std::string("while validating mode '") + mode->GetModeName() + "': " + e.what());
		}
	}

	// Apply mode config
	try {
		for (CBaseMode *mode : m_ModeManager.GetModeList())
		{
			// FIXME: HACK
			if (!mode)
				continue;
			auto it = mode_configs.find(mode->GetModeName());
			if (it != mode_configs.end())
			{
				const nlohmann::json &item = *it;

				// Apply vars
				for (MMConfigVarBase *var : MMConfigVarBase::GetModeVars(mode->GetModeID()))
				{
					auto it2 = item.find(var->GetName());
					if (it2 != item.end())
					{
						var->SetJSON(*it2);
					}
				}

				mode->ApplyConfig(item);
			}
		}
	}
	catch (const std::exception &e)
	{
		UTIL_LogPrintf("ERROR: Unexpected exception while applying mode config: %s\n", e.what());
		abort();
	}

	m_ParsedConfig = mmParsedCfg;
	InitHudTexts();
}

void CHalfLifeMultimode::InitHudTexts()
{
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
	m_ModeTitleTextParams.holdTime = m_ParsedConfig.freezeTime - 0.6f;
	m_ModeTitleTextParams.fxTime = m_ParsedConfig.freezeTime - 0.6f;

	m_ModeTitleTextParams.y = 0.3f;

	m_ModeInfoTextParams = m_ModeTitleTextParams;
	m_ModeInfoTextParams.y = 0.35f;
	m_ModeInfoTextParams.r1 = 255;
	m_ModeInfoTextParams.g1 = 255;
	m_ModeInfoTextParams.b1 = 255;

	m_IntermStatsTextParams.x = 0.75;
	m_IntermStatsTextParams.y = 0.4f;
	m_IntermStatsTextParams.effect = 0;
	m_IntermStatsTextParams.r1 = 255;
	m_IntermStatsTextParams.g1 = 255;
	m_IntermStatsTextParams.b1 = 255;
	m_IntermStatsTextParams.fadeinTime = 1.f;
	m_IntermStatsTextParams.fadeoutTime = 0.1f;
	m_IntermStatsTextParams.holdTime = m_ParsedConfig.intermTime - 0.6f;
	m_IntermStatsTextParams.fxTime = m_ParsedConfig.intermTime - 0.6f;
}


bool IsRunningMultimode()
{
	return g_multimode;
}

bool IsRunningMultimode(ModeID mode)
{
	return (g_multimode && GetMultimodeGR()->GetModeID() == mode);
}

CHalfLifeMultimode *GetMultimodeGR()
{
	ASSERT(g_multimode);
	return static_cast<CHalfLifeMultimode *>(g_pGameRules);
}

CBaseMode *GetRunningMultimodeBase()
{
	return GetMultimodeGR()->GetBaseMode();
}

void CHalfLifeMultimode::ClientDisconnected(edict_t *pClient)
{
	if (GetBaseMode())
	{
		GetBaseMode()->ClientDisconnected(pClient);
	}

	BaseClass::ClientDisconnected(pClient);
}

float CHalfLifeMultimode::FlPlayerFallDamage(CBasePlayer *pPlayer)
{
	if (GetBaseMode())
	{
		float dmg = GetBaseMode()->PlayerFallDamage(pPlayer);
		if (dmg != -1)
			return dmg;
	}

	return BaseClass::FlPlayerFallDamage(pPlayer);
}

BOOL CHalfLifeMultimode::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	if (GetBaseMode())
	{
		return GetBaseMode()->PlayerCanTakeDamage(pPlayer, pAttacker);
	}

	return false;
}

void CHalfLifeMultimode::PlayerSpawn(CBasePlayer *pPlayer)
{
	BaseClass::PlayerSpawn(pPlayer);

	if (pPlayer->IsSpectator())
		return;

	if (GetState() == State::FreezeTime || m_bFreezeOnSpawn)
	{
		// Disable attack
		pPlayer->m_flNextAttack = gpGlobals->time + 9999999;

		pPlayer->pev->flags |= FL_FROZEN;
		pPlayer->pev->flags |= FL_GODMODE;

		DROP_TO_FLOOR(pPlayer->edict());
	}

	// Regive weapons
	pPlayer->RemoveAllItems(false);

	if (GetBaseMode())
	{
		GetBaseMode()->GivePlayerWeapons(pPlayer);
		GetBaseMode()->PlayerSpawn(pPlayer);
	}
}

void CHalfLifeMultimode::PlayerThink(CBasePlayer *pPlayer)
{
	BaseClass::PlayerThink(pPlayer);

	if (GetBaseMode())
	{
		GetBaseMode()->PlayerThink(pPlayer);
	}
}

BOOL CHalfLifeMultimode::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	if (GetState() == State::Intermission)
		return false;

	if (GetBaseMode())
	{
		return GetBaseMode()->PlayerCanRespawn(pPlayer);
	}

	return false;
}

float CHalfLifeMultimode::FlPlayerSpawnTime(CBasePlayer *pPlayer)
{
	if (GetBaseMode())
	{
		return GetBaseMode()->PlayerSpawnTime(pPlayer);
	}

	return BaseClass::FlPlayerSpawnTime(pPlayer);
}

edict_t *CHalfLifeMultimode::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	if (GetBaseMode())
	{
		edict_t *p = GetBaseMode()->GetPlayerSpawnSpot(pPlayer);
		if (p)
			return p;
	}

	return BaseClass::GetPlayerSpawnSpot(pPlayer);
}

int CHalfLifeMultimode::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	if (GetBaseMode())
	{
		return GetBaseMode()->PointsForKill(pAttacker, pKilled);
	}

	return BaseClass::IPointsForKill(pAttacker, pKilled);
}

void CHalfLifeMultimode::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	BaseClass::PlayerKilled(pVictim, pKiller, pInflictor);

	pVictim->m_iMultimodeDeaths++;

	CBaseEntity *ktmp = CBaseEntity::Instance(pKiller);

	if (ktmp && ktmp->IsPlayer())
	{
		CBasePlayer *pKillerEnt = (CBasePlayer *)ktmp;

		if (pVictim->pev == pKiller)
		{
			// killed self
			pKillerEnt->m_iMultimodeScore -= 1;
		}
		else if (ktmp && ktmp->IsPlayer())
		{
			pKillerEnt->m_iMultimodeScore += IPointsForKill(pKillerEnt, pVictim);
		}
		else
		{
			// killed by the world
			pKillerEnt->m_iMultimodeScore -= 1;
		}
	}

	if (GetBaseMode())
	{
		GetBaseMode()->PlayerKilled(pVictim, pKiller, pInflictor);
	}
}

BOOL CHalfLifeMultimode::CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	bool can = BaseClass::CanHavePlayerItem(pPlayer, pWeapon);
	if (!can)
		return false;

	if (GetBaseMode())
	{
		return GetBaseMode()->CanHaveWeapon(pPlayer, pWeapon);
	}

	return can;
}

void CHalfLifeMultimode::PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	BaseClass::PlayerGotWeapon(pPlayer, pWeapon);

	if (GetBaseMode())
	{
		GetBaseMode()->PlayerGotWeapon(pPlayer, pWeapon);
	}
}

int CHalfLifeMultimode::WeaponShouldRespawn(CBasePlayerItem *pWeapon)
{
	return BaseClass::WeaponShouldRespawn(pWeapon);
}

float CHalfLifeMultimode::FlWeaponRespawnTime(CBasePlayerItem *pWeapon)
{
	if (GetBaseMode())
	{
		if (GetBaseMode()->ShouldRespawnWeapons() && GetBaseMode()->ShouldRespawnWeaponsAfterPickUp())
			return gpGlobals->time + GetBaseMode()->GetWeaponRespawnTime();
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
	if (GetBaseMode())
	{
		return GetBaseMode()->CanHaveItem(pPlayer, pItem);
	}

	return BaseClass::CanHaveItem(pPlayer, pItem);
}

void CHalfLifeMultimode::PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem)
{
	BaseClass::PlayerGotItem(pPlayer, pItem);

	if (GetBaseMode())
	{
		GetBaseMode()->PlayerGotItem(pPlayer, pItem);
	}
}

int CHalfLifeMultimode::ItemShouldRespawn(CItem *pItem)
{
	return BaseClass::ItemShouldRespawn(pItem);
}

float CHalfLifeMultimode::FlItemRespawnTime(CItem *pItem)
{
	if (GetBaseMode())
	{
		if (GetBaseMode()->ShouldRespawnItems() && GetBaseMode()->ShouldRespawnItemsAfterPickUp())
			return gpGlobals->time + GetBaseMode()->GetItemRespawnTime();
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
	if (GetBaseMode())
	{
		if (GetBaseMode()->ShouldRespawnAmmo() && GetBaseMode()->ShouldRespawnAmmoAfterPickUp())
			return gpGlobals->time + GetBaseMode()->GetAmmoRespawnTime();
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
	if (GetBaseMode())
	{
		float time = GetBaseMode()->HealthChargerRechargeTime();
		if (time != -1)
			return time;
	}

	return BaseClass::FlHealthChargerRechargeTime();
}

float CHalfLifeMultimode::FlHEVChargerRechargeTime()
{
	if (GetBaseMode())
	{
		float time = GetBaseMode()->SuitChargerRechargeTime();
		if (time != -1)
			return time;
	}

	return BaseClass::FlHEVChargerRechargeTime();
}

int CHalfLifeMultimode::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	if (GetBaseMode())
	{
		return GetBaseMode()->DeadPlayerWeapons(pPlayer);
	}

	return BaseClass::DeadPlayerWeapons(pPlayer);
}

int CHalfLifeMultimode::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	if (GetBaseMode())
	{
		return GetBaseMode()->DeadPlayerAmmo(pPlayer);
	}

	return BaseClass::DeadPlayerAmmo(pPlayer);
}

int CHalfLifeMultimode::GetCritDamage(CBasePlayer *pAttacker, CBaseEntity *pVictim, int iOrigDmg, int iWeapon)
{
	if (GetBaseMode())
	{
		return GetBaseMode()->GetCritDamage(pAttacker, pVictim, iOrigDmg, iWeapon);
	}

	return BaseClass::GetCritDamage(pAttacker, pVictim, iOrigDmg, iWeapon);
}

void CHalfLifeMultimode::OnCritHit(CBasePlayer *pAttacker, CBaseEntity *pVictim, int iOrigDmg, int iCritDmg, int iWeapon)
{
	BaseClass::OnCritHit(pAttacker, pVictim, iOrigDmg, iCritDmg, iWeapon);

	if (GetBaseMode())
	{
		GetBaseMode()->OnCritHit(pAttacker, pVictim, iOrigDmg, iCritDmg, iWeapon);
	}
}

void CHalfLifeMultimode::OnPrimaryAttack(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	if (GetBaseMode())
	{
		GetBaseMode()->OnPrimaryAttack(pPlayer, pWeapon);
	}
}
