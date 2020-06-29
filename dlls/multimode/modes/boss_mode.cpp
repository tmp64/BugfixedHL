#include <string>
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode/multimode_gamerules.h"
#include "game.h"
#include "boss_mode.h"

static MMConfigVar<CBossMode, int> mp_mm_boss_hp("boss_hp", 350);
static MMConfigVar<CBossMode, int> mp_mm_boss_ap("boss_ap", 300);
static MMConfigVar<CBossMode, int> mp_mm_boss_player_num("player_num", 5);
static MMConfigVar<CBossMode, int> mp_mm_boss_stats_channel("stats_channel", 1);
static MMConfigVar<CBossMode, bool> mp_mm_boss_ban_egon("ban_egon", true);
static MMConfigVar<CBossMode, int> mp_mm_boss_pl_score("pl_score", 10);
static MMConfigVar<CBossMode, int> mp_mm_boss_win_min_score("win_min_score", 5);
static MMConfigVar<CBossMode, int> mp_mm_boss_win_max_score("win_max_score", 10);
static MMConfigVar<CBossMode, int> mp_mm_boss_lose_score("lose_score", 3);

struct BossSpawnInv
{
	const char *ent_name;	// Name of the entity
	int times;				// How many times it should be given
};

struct BossPlayerScore
{
	int idx;
	float dmg;
};

// From player.cpp
constexpr float ARMOR_RATIO = 0.2;	// Armor Takes 80% of the damage
constexpr float ARMOR_BONUS = 0.5;	// Each Point of Armor is work 1/x points of health

//constexpr int BOSS_INV_COUNT = 13;

// Weapons boss will spawn with these items
static const BossSpawnInv s_BossSpawnInv[] = {
	{ "weapon_crowbar", 1 },
	{ "weapon_glock", 1 },
	{ "weapon_python", 4 },
	{ "weapon_mp5", 5 },
	{ "weapon_shotgun", 5 },
	{ "weapon_crossbow", 1 },
	{ "weapon_rpg", 3 },
	{ "weapon_hornetgun", 1 },
	{ "weapon_tripmine", 5 },
	{ "weapon_satchel", 5 },
	{ "weapon_snark", 3 },
	{ "weapon_handgrenade", 2 },
	{ "item_longjump", 1 },
};

// Weapons boss can pick up
static const int s_AllowedBossWeapons[] = {
	WEAPON_CROWBAR,
	WEAPON_GLOCK,
	WEAPON_PYTHON,
	WEAPON_MP5,
	WEAPON_SHOTGUN,
	WEAPON_CROSSBOW,
	WEAPON_RPG,
	WEAPON_HORNETGUN,
	WEAPON_TRIPMINE,
	WEAPON_SATCHEL,
	WEAPON_SNARK,
	WEAPON_HANDGRENADE,
};

static BossPlayerScore s_PlayerScores[MAX_PLAYERS + 1];

CBossMode::CBossMode() : CBaseMode()
{
	m_StatsTextParams.x = 0.05f;
	m_StatsTextParams.y = 0.4f;
	m_StatsTextParams.effect = 0;
	m_StatsTextParams.r1 = 255;
	m_StatsTextParams.g1 = 160;
	m_StatsTextParams.b1 = 0;
	m_StatsTextParams.fadeinTime = 0.0f;
	m_StatsTextParams.fadeoutTime = 0.0f;
	m_StatsTextParams.holdTime = 1.33;
	m_StatsTextParams.fxTime = 1.33;
	m_StatsTextParams.channel = (int)mp_mm_boss_stats_channel.Get();
}

ModeID CBossMode::GetModeID()
{
	return MODE_ID;
}

const char *CBossMode::GetModeName()
{
	return MODE_NAME;
}

const char *CBossMode::GetShortTitle()
{
	return "Boss Fight";
}

const char *CBossMode::GetDescription()
{
	return m_Descr;
}

void CBossMode::OnFreezeStart()
{
	ASSERT(!m_pBoss);

	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		m_TeamDmgGlowTime[i] = 0.0f;
		m_flPlayerDmg[i] = 0.0f;
	}

	snprintf(m_Descr, sizeof(m_Descr), "Players must cooperate to slay the boss.\n"
		"The boss has all weapons, %d HP and %d AP",
		(int)mp_mm_boss_hp.Get(), (int)mp_mm_boss_ap.Get());
}

void CBossMode::OnStart()
{
	ASSERT(!m_pBoss);
	if (FindNewBoss())
	{
		char buf[128];
		const char *name = STRING(m_pBoss->pev->netname);
		snprintf(buf, sizeof(buf), "^2[Multimode] ^1%s^0 is the boss!", name);
		UTIL_ColoredClientPrintAll(HUD_PRINTTALK, buf);
	}

	m_flNextScoreUpdate = 0;
}

void CBossMode::OnEnd()
{
	CalculateStats();
}

void CBossMode::OnSwitchOff()
{
	SetBoss(nullptr);
}

void CBossMode::Think()
{
	if (m_flNextScoreUpdate <= gpGlobals->time && (
		GetMultimodeGR()->GetState() == CHalfLifeMultimode::State::Game || 
		GetMultimodeGR()->GetState() == CHalfLifeMultimode::State::Intermission))
	{
		// Show stats
		int iPlayerCount = 0;

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
			if (pPlayer && pPlayer != m_pBoss)
			{
				s_PlayerScores[iPlayerCount + 1].idx = i;
				s_PlayerScores[iPlayerCount + 1].dmg = m_flPlayerDmg[i];

				iPlayerCount++;
			}
		}

		if (iPlayerCount > 0)
		{
			qsort(s_PlayerScores + 1, iPlayerCount, sizeof(s_PlayerScores[0]), [](const void *plhs, const void *prhs)
			{
				BossPlayerScore &lhs = *(BossPlayerScore *)plhs;
				BossPlayerScore &rhs = *(BossPlayerScore *)prhs;

				if (lhs.dmg > rhs.dmg)
					return -1;
				if (lhs.dmg < rhs.dmg)
					return 1;

				return 0;
			});

			std::string str = "^1Damage stats:^0\n";

			for (int i = 1, len = min(iPlayerCount, 5); i <= len; i++)
			{
				char buf[128];
				const char *playerName = STRING(UTIL_PlayerByIndex(s_PlayerScores[i].idx)->pev->netname);
				snprintf(buf, sizeof(buf), "%d. %s^0 - %.0f\n", i, playerName, s_PlayerScores[i].dmg);
				str += buf;
			}

			UTIL_ColoredHudMessageAll(m_StatsTextParams, str.c_str());
		}

		m_flNextScoreUpdate = gpGlobals->time + 0.5f;
	}
}

void CBossMode::ClientDisconnected(edict_t *pClient)
{
	m_flPlayerDmg[ENTINDEX(pClient)] = 0;

	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pClient);
	if (pPlayer && pPlayer == m_pBoss)
	{
		// Boss has left, find new boss.
		if (FindNewBoss(pPlayer))
		{
			char buf[128];
			const char *name = STRING(m_pBoss->pev->netname);
			snprintf(buf, sizeof(buf), "^2[Multimode] ^0Boss has left the game. ^1%s^0 is the new boss!", name);
			UTIL_ColoredClientPrintAll(HUD_PRINTTALK, buf);
		}
	}
}

void CBossMode::PlayerSpawn(CBasePlayer *pPlayer)
{
	if (GetMultimodeGR()->GetState() == CHalfLifeMultimode::State::Game && !pPlayer->IsSpectator())
	{
		// m_pBoss should only be null if the server is empty
		if (!m_pBoss)
		{
			// Set newly spawned player as the boss
			SetBoss(pPlayer);

			char buf[128];
			snprintf(buf, sizeof(buf), "^2[Multimode] ^1%s^0 is the new boss!", STRING(pPlayer->pev->netname));
		}
	}
}

void CBossMode::PlayerThink(CBasePlayer *pPlayer)
{
	float &time = m_TeamDmgGlowTime[pPlayer->entindex()];
	if (time != 0.0f && time <= gpGlobals->time)
		DisableTeamDmgGlow(pPlayer);
}

void CBossMode::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	if (pVictim == m_pBoss)
	{
		m_bIsBossDead = true;
		GetMultimodeGR()->FinishMode();
	}
}

bool CBossMode::PlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	if (pAttacker->IsPlayer())
	{
		CBasePlayer *pAttackerPl = (CBasePlayer *)pAttacker;

		if (pPlayer == pAttacker || pAttacker == m_pBoss || pPlayer == m_pBoss)
		{
			return true;
		}
		else
		{
			EnableTeamDmgGlow(pPlayer);

			// Play sound
			if (!pAttackerPl->m_bIsBot)
			{
				CLIENT_COMMAND(pAttackerPl->edict(), "spk weapons/electro5.wav\n");
			}

			return false;
		}
	}
	else
	{
		return true;
	}
}

float CBossMode::PlayerFallDamage(CBasePlayer *pPlayer)
{
	if (pPlayer == m_pBoss)
		return 0;
	else
		return -1;
}

bool CBossMode::CanHaveWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	if (pWeapon->m_iId == WEAPON_EGON && mp_mm_boss_ban_egon.Get())
		return false;

	if (pPlayer == m_pBoss)
	{
		// Boss can only pick up specific weapons (so they are not OP AF)
		for (int i = 0; i < HLARRAYSIZE(s_AllowedBossWeapons); i++)
		{
			if (s_AllowedBossWeapons[i] == pWeapon->m_iId)
				return true;
		}

		return false;
	}
	else
	{
		return true;
	}
}

void CBossMode::PlayerTookDamage(CBasePlayer *pPlayer, CBasePlayer *pAttacker, float flDamage)
{
	if (pPlayer == m_pBoss && pAttacker && pAttacker != m_pBoss)
	{
		m_flPlayerDmg[pAttacker->entindex()] += flDamage;
	}
}

bool CBossMode::FindNewBoss(CBasePlayer *pIgnore)
{
	SetBoss(nullptr);

	CBasePlayer *players[MAX_PLAYERS + 1];
	int playerCount = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (pPlayer && pPlayer != pIgnore && !pPlayer->IsSpectator())
		{
			players[playerCount] = pPlayer;
			playerCount++;
		}
	}

	if (playerCount > 0)
	{
		SetBoss(players[RANDOM_LONG(0, playerCount - 1)]);
		return true;
	}
	else
	{
		return false;
	}
}

void CBossMode::SetBoss(CBasePlayer *pPlayer)
{
	if (m_pBoss)
	{
		m_pBoss->pev->renderfx = kRenderFxNone;
		m_pBoss->pev->rendercolor = Vector(0, 0, 0);
		m_pBoss->pev->renderamt = 0;
		m_pBoss->pev->max_health = 100;
		m_flEffectiveHealth = -1;
		m_pBoss = nullptr;
	}

	if (pPlayer)
	{
		ASSERT(!pPlayer->IsSpectator());
			
		pPlayer->pev->renderfx = kRenderFxGlowShell;
		pPlayer->pev->rendercolor = Vector(240, 38, 38);	// Red color
		pPlayer->pev->renderamt = 64;
		pPlayer->pev->max_health = mp_mm_boss_hp.Get();
		pPlayer->pev->health = mp_mm_boss_hp.Get();
		pPlayer->pev->armorvalue = mp_mm_boss_ap.Get();

		pPlayer->RemoveAllItems(false);
		
		// Give weapons
		for (int i = 0; i < HLARRAYSIZE(s_BossSpawnInv); i++)
		{
			for (int j = 0; j < s_BossSpawnInv[i].times; j++)
			{
				pPlayer->GiveNamedItem(s_BossSpawnInv[i].ent_name);
			}
		}

		m_flEffectiveHealth = CalculateEffHP(pPlayer->pev->health, pPlayer->pev->armorvalue);
		m_bIsBossDead = false;
		m_pBoss = pPlayer;

		UTIL_LogPrintf("[Multimode] %s is the boss. HP: %.0f, AP: %.0f, EffHP: %.0f",
			STRING(pPlayer->pev->netname), pPlayer->pev->health, pPlayer->pev->armorvalue, m_flEffectiveHealth);
	}
}

void CBossMode::EnableTeamDmgGlow(CBasePlayer *pPlayer)
{
	ASSERT(pPlayer != m_pBoss);
	m_TeamDmgGlowTime[pPlayer->entindex()] = gpGlobals->time + 0.75f;
	pPlayer->pev->renderfx = kRenderFxGlowShell;
	pPlayer->pev->rendercolor = Vector(3, 252, 15);	// Green
	pPlayer->pev->renderamt = 48;
}

void CBossMode::DisableTeamDmgGlow(CBasePlayer *pPlayer)
{
	m_TeamDmgGlowTime[pPlayer->entindex()] = 0.0f;
	pPlayer->pev->renderfx = kRenderFxNone;
	pPlayer->pev->rendercolor = Vector(0, 0, 0);
	pPlayer->pev->renderamt = 0;
}

void CBossMode::CalculateStats()
{
	// Find the player who dealt the most damage
	float maxDamage = 0;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (pPlayer && pPlayer != m_pBoss)
		{
			maxDamage = max(maxDamage, m_flPlayerDmg[i]);
		}
	}

	// Award points to players
	int maxPlayerScore = 0;
	if (maxDamage > 0)
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
			if (pPlayer && pPlayer != m_pBoss && (!pPlayer->IsSpectator() || m_flPlayerDmg[i] != 0.0f))
			{
				int score = (int)((m_flPlayerDmg[i] / maxDamage) * mp_mm_boss_pl_score.Get());
				maxPlayerScore = max(maxPlayerScore, score);
				pPlayer->m_iMultimodeScore = score;
				pPlayer->AddPoints(pPlayer->m_iMultimodeScore, false);
			}
		}
	}

	// Award points to the boss based on remaining hp
	if (m_pBoss)
	{
		if (!m_bIsBossDead)
		{
			// Boss has won
			float fr = CalculateEffHP(m_pBoss->pev->health, m_pBoss->pev->armorvalue) / m_flEffectiveHealth;
			if (fr < 0.0f)
				fr = 0.0f;
			else if (fr > 1.0f)
				fr = 1.0f;

			int minScore = mp_mm_boss_win_min_score.Get();
			int maxScore = mp_mm_boss_win_max_score.Get();
			int addScore = (int)(minScore + (maxScore - minScore) * fr);

			// Winning boss must get at least one point more than the best player
			addScore = max(maxPlayerScore + 1, addScore);

			m_pBoss->m_iMultimodeScore += addScore;
			m_pBoss->AddPoints(addScore, false);
		}
		else
		{
			// Boss has lost
			// Hide from intermission stats
			m_pBoss->m_iMultimodeScore = MULTIMODE_NO_SCORE;

			// Award some points
			m_pBoss->AddPoints((int)mp_mm_boss_lose_score.Get(), false);
		}
	}
}

float CBossMode::CalculateEffHP(float hp, float ap)
{
	return hp + ap / ARMOR_BONUS;
}
