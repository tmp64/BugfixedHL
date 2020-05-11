#ifndef MULTIMODE_SLOW_ROCKETS_MODE_H
#define MULTIMODE_SLOW_ROCKETS_MODE_H
#include "multimode/basemode.h"
/**
 * Slow rockets
 *
 * Rockets are much slower (controlled by mp_mm_rocket_speed cvar), have
 * bigger explosion radius (probably shouldn't) and do more damage (160 vs 120).
 *
 * Changes to CRpgRocket:
 * - added speed limit to CRpgRocket::FollowThink;
 * - changed m_flIgniteTime to be 14 seconds in the future;
 * - that increases time before self-destruction from 1 sec to 15 secs.
 *
 * Changes to CRpg:
 * - disabled secondary attack;
 * - disabled laser spot
 */
class CSlowRocketsMode : public CBaseMode
{
public:
    static constexpr ModeID MODE_ID = ModeID::SlowRockets;
    static constexpr char MODE_NAME[] = "slow_rockets";

	CSlowRocketsMode();

    virtual ModeID GetModeID();
	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual void GetShortTitleColor(byte &r, byte &g, byte &b);
	virtual const char *GetDescription();

	virtual void OnFreezeStart();
	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapons();
	virtual bool ShouldRespawnAmmo();
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);

private:
    float m_flNextCheckTime[MAX_PLAYERS];
};

#endif
