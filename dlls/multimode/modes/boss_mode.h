#ifndef MULTIMODE_BOSS_MODE_H
#define MULTIMODE_BOSS_MODE_H
#include "multimode/basemode.h"

class CBossMode : public CBaseMode
{
public:
	CBossMode();

	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();

	virtual void OnFreezeStart();
	virtual void OnStart();
	virtual void OnEnd();
	virtual void OnSwitchOff();
	virtual void Think();
	virtual void ClientDisconnected(edict_t *pClient);
	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);
	virtual bool PlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker);
	virtual float PlayerFallDamage(CBasePlayer *pPlayer);
	virtual bool CanHaveWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	void PlayerTookDamage(CBasePlayer *pPlayer, CBasePlayer *pAttacker, float flDamage);

private:
	hudtextparms_t m_StatsTextParams;
	char m_Descr[128];
	CBasePlayer *m_pBoss = nullptr;
	float m_TeamDmgGlowTime[MAX_PLAYERS + 1];
	float m_flNextScoreUpdate = 0;
	float m_flPlayerDmg[MAX_PLAYERS + 1];
	float m_flEffectiveHealth = -1;	// How much damage needs to be dealt to the boss to kill them
	bool m_bIsBossDead = false;

	// Selects new boss or returns false if no players connected.
	bool FindNewBoss(CBasePlayer *pIgnore = nullptr);

	void SetBoss(CBasePlayer *pPlayer);

	void EnableTeamDmgGlow(CBasePlayer *pPlayer);
	void DisableTeamDmgGlow(CBasePlayer *pPlayer);

	void CalculateStats();
	float CalculateEffHP(float hp, float ap);
};

#endif
