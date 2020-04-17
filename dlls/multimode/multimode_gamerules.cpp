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

#include "dm_mode.h"
#include "oneshot_mode.h"
#include "recoil_mode.h"
#include "wpn_drop_mode.h"
#include "warmup_mode.h"
#include "biohazard_mode.h"
#include "slowrockets_mode.h"
#include "speed_mode.h"

extern int g_multimode;

ConVar mp_mm_min_players("mp_mm_min_players", "1");
ConVar mp_mm_warmup_time("mp_mm_warmup_time", "45");
ConVar mp_mm_freeze_time("mp_mm_freeze_time", "5");
ConVar mp_mm_game_time("mp_mm_game_time", "60");
ConVar mp_mm_interm_time("mp_mm_interm_time", "5");

ConVar mp_mm_skip_warmup("mp_mm_skip_warmup", "0");
ConVar mp_mm_skip_mode("mp_mm_skip_mode", "0");

// Selects behavior when all modes have been played:
//   0: the game will continue from the first mode;
//   1: the game will go in intermission
//   2: the game will go in "endgame" state to choose a
//      new map using mapchooser_multimode AMXX plugin.
ConVar mp_mm_on_end("mp_mm_on_end", "0");

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

	m_IntermStatsTextParams.x = 0.75;
	m_IntermStatsTextParams.y = 0.4f;
	m_IntermStatsTextParams.effect = 0;
	m_IntermStatsTextParams.r1 = 255;
	m_IntermStatsTextParams.g1 = 255;
	m_IntermStatsTextParams.b1 = 255;
	m_IntermStatsTextParams.fadeinTime = 1.f;
	m_IntermStatsTextParams.fadeoutTime = 0.1f;
	m_IntermStatsTextParams.holdTime = mp_mm_interm_time.Get() - 0.6f;
	m_IntermStatsTextParams.fxTime = mp_mm_interm_time.Get() - 0.6f;

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
	m_pModes[(int)ModeID::Biohazard] = new CBiohazardMode();
	m_pModes[(int)ModeID::SlowRockets] = new CSlowRocketsMode();
	m_pModes[(int)ModeID::Speed] = new CSpeedMode();

	// Remove timelimit
	g_engfuncs.pfnCvar_DirectSet(&timelimit, "0");

	// Save maxspeed before it is modified by modes
	m_flDefaultMaxSpeed = CVAR_GET_FLOAT("sv_maxspeed");
}

CHalfLifeMultimode::~CHalfLifeMultimode()
{
	for (int i = (int)ModeID::ModeCount - 1; i >= 0; i--)
		delete m_pModes[i];
}

CHalfLifeMultimode::State CHalfLifeMultimode::GetState()
{
	return m_State;
}

void CHalfLifeMultimode::SwitchToWaiting()
{
	if (m_State == State::Waiting)
		return;

	ResetTimerUpdate();

	if (m_State == State::Warmup)
	{
		m_State = State::Waiting;
		return;
	}

	FinishCurMode();
	m_State = State::Waiting;
	m_pCurMode = m_pWarmupMode;
	BeginCurMode(false, false);
}

void CHalfLifeMultimode::SwitchToWarmup()
{
	if (m_State == State::Warmup)
		return;

	ResetTimerUpdate();
	m_flWarmupEndTime = gpGlobals->time + mp_mm_warmup_time.Get();

	if (m_State == State::Waiting)
	{
		m_State = State::Warmup;
		return;
	}

	FinishCurMode();
	m_State = State::Warmup;
	m_pCurMode = m_pWarmupMode;
	BeginCurMode(false, false);
}

void CHalfLifeMultimode::SwitchToIntermission()
{
	if (m_State == State::Intermission)
		return;

	m_State = State::Intermission;
	m_flIntermEndTime = gpGlobals->time + mp_mm_interm_time.Get();

	// Freeze all players by changing max speed
	// (for slow down effect instead of immediate freeze)
	CVAR_SET_FLOAT("sv_maxspeed", 0);

	int iPlayerCount = 0;
	int iPlayerScoresIdx = 1;
	
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			if (!IsSpectator(pPlayer))
			{
				// Disable weapons
				pPlayer->m_flNextAttack = gpGlobals->time + 99999999;	// never

				// Give god mode
				pPlayer->pev->flags |= FL_GODMODE;
			}

			// Play round end sound
			CLIENT_COMMAND(pPlayer->edict(), "spk buttons/button8.wav\n");

			// Set in score array
			s_PlayerScores[iPlayerScoresIdx].idx = i;
			s_PlayerScores[iPlayerScoresIdx].score = pPlayer->m_iMultimodeScore;
			s_PlayerScores[iPlayerScoresIdx].deaths = pPlayer->m_iMultimodeDeaths;

			iPlayerCount++;
			iPlayerScoresIdx++;
		}
	}

	// Sort scores
	qsort(s_PlayerScores + 1, iPlayerCount, sizeof(s_PlayerScores[0]), SortMultimodePlayerScore);

	// Show message
	std::string str;

	if (iPlayerCount < 2 || s_PlayerScores[1].score == s_PlayerScores[2].score)
		str = "^3Draw!^0\n\n";
	else
		str = STRING(UTIL_PlayerByIndex(s_PlayerScores[1].idx)->pev->netname) + std::string("^0 is the winner!\n\n");

	for (int i = 1, len = min(iPlayerCount, 7) ; i <= len; i++)
	{
		char buf[128];
		const char *playerName = STRING(UTIL_PlayerByIndex(s_PlayerScores[i].idx)->pev->netname);
		snprintf(buf, sizeof(buf), "%d. %s^0 - %d\n", i, playerName, s_PlayerScores[i].score);
		str += buf;
	}

	UTIL_ColoredDirectorHudMessageAll(m_IntermStatsTextParams, str.c_str(), true);
}

void CHalfLifeMultimode::SwitchToNextMode()
{
	if (m_State == State::Intermission)
	{
		// Restore speed
		CVAR_SET_FLOAT("sv_maxspeed", m_flDefaultMaxSpeed);

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

	FinishCurMode();

	m_CurModeId = (ModeID)((int)m_CurModeId + 1);
	if (m_CurModeId == ModeID::ModeCount)
	{
		switch ((int)mp_mm_on_end.Get())
		{
		case 0:
		{
			m_CurModeId = (ModeID)1;
			break;
		}
		case 1:
		{
			m_State = State::FinalIntermission;
			GoToIntermission();
			return;
		}
		case 2:
		{
			int votetime = CVAR_GET_FLOAT("amx_multimode_votetime");

			if (votetime <= 0)
			{
				GoToIntermission();
				return;
			}

			g_engfuncs.pfnCvar_DirectSet(&timelimit, UTIL_VarArgs("%f", (gpGlobals->time + votetime) / 60.0));
			SwitchToEndgame();
			return;
		}
		}
	}

	// Skip commented modes
	while (!m_pModes[(int)m_CurModeId])
		m_CurModeId = (ModeID)((int)m_CurModeId + 1);

	m_pCurMode = m_pModes[(int)m_CurModeId];

	m_State = State::Game;
	BeginCurMode(true, true);
}

void CHalfLifeMultimode::SwitchToEndgame()
{
	if (m_State == State::Endgame)
		return;

	FinishCurMode();
	ResetTimerUpdate();
	m_State = State::Endgame;
	m_pCurMode = m_pWarmupMode;
	BeginCurMode(false, false);

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

void CHalfLifeMultimode::BeginCurMode(bool bEnableFreezeTime, bool bShowModeInfo)
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
		else if (!strcmp("monster_snark", classname) ||
			!strcmp("hornet", classname))
		{
			CBaseMonster *pEnt = (CBaseMonster *)CBaseEntity::Instance(pEdict);
			pEnt->Killed(nullptr, GIB_NEVER);
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
	ResetTimerUpdate();

	if (m_State != State::Waiting && m_State != State::Warmup)
	{
		if (bShowModeInfo)
		{
			// Show mode info message
			m_pCurMode->GetShortTitleColor(m_ModeTitleTextParams.r1, m_ModeTitleTextParams.g1, m_ModeTitleTextParams.b1);
			UTIL_DirectorHudMessageAll(m_ModeTitleTextParams, m_pCurMode->GetShortTitle(), true);
			UTIL_DirectorHudMessageAll(m_ModeInfoTextParams, m_pCurMode->GetDescription(), true);
		}

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

			pPlayer->pev->flags &= ~FL_FROZEN;
			pPlayer->pev->flags &= ~FL_GODMODE;

			if (!IsSpectator(pPlayer))
			{
				pPlayer->m_flNextAttack = 0;
				pPlayer->m_iMultimodeScore = 0;
				pPlayer->m_iMultimodeDeaths = 0;
			}
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

void CHalfLifeMultimode::ThinkWaiting()
{
	if (gpGlobals->time >= m_flNextTimerUpdate)
	{
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
}

void CHalfLifeMultimode::ThinkWarmup()
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
}

void CHalfLifeMultimode::ThinkFreezeTime()
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
}

void CHalfLifeMultimode::ThinkGame()
{
	// Update timer
	if (gpGlobals->time >= m_flNextTimerUpdate)
	{
		char buf[128];
		int seconds = m_flEndTime - gpGlobals->time;
		snprintf(buf, sizeof(buf), "%d:%02d", seconds / 60, seconds % 60);
		UTIL_DirectorHudMessageAll(m_TimerTextParams, buf, false);

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
		SwitchToIntermission();
	}
}

void CHalfLifeMultimode::ThinkIntermission()
{
	if (gpGlobals->time >= m_flIntermEndTime)
	{
		SwitchToNextMode();
	}
}

void CHalfLifeMultimode::Think()
{
	BaseClass::Think();
	
	if (m_pCurMode)
	{
		m_pCurMode->Think();
	}

	// Handle skip cvars
	if (mp_mm_skip_warmup)
	{
		mp_mm_skip_warmup.Set(0.0);
		if (m_State == State::Warmup)
			SwitchToNextMode();
	}
	else if (mp_mm_skip_mode.Get() > 0)
	{
		mp_mm_skip_mode.Set(mp_mm_skip_mode.Get() - 1);
		SwitchToNextMode();
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
		ThinkWaiting();
		break;
	}
	case State::Warmup:
	{
		ThinkWarmup();
		break;
	}
	case State::FreezeTime:
	{
		ThinkFreezeTime();
		break;
	}
	case State::Game:
	{
		ThinkGame();
		break;
	}
	case State::Intermission:
	{
		ThinkIntermission();
		break;
	}
	case State::Endgame:
	case State::FinalIntermission:
	{
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

		pPlayer->pev->flags |= FL_FROZEN;
		pPlayer->pev->flags |= FL_GODMODE;

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
	if (m_State == State::Intermission)
		return false;

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

void CHalfLifeMultimode::ResetTimerUpdate()
{
	m_flNextTimerUpdate = gpGlobals->time + 0.0;
}

bool IsRunningMultimode()
{
	return g_multimode;
}

bool IsRunningMultimode(ModeID mode)
{
	return (g_multimode && static_cast<CHalfLifeMultimode *>(g_pGameRules)->m_CurModeId == mode);
}

CHalfLifeMultimode *GetMultimodeGR()
{
	ASSERT(g_multimode);
	return static_cast<CHalfLifeMultimode *>(g_pGameRules);
}
