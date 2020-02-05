#ifndef MULTIMODE_WON_DROP_MODE_H
#define MULTIMODE_WON_DROP_MODE_H
#include "basemode.h"

class CWpnDropMode : public CBaseMode
{
public:
	CWpnDropMode();

	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();

	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapons();
	virtual bool ShouldRespawnAmmo();
	virtual void OnPrimaryAttack(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual void PlayerThink(CBasePlayer *pPlayer);

private:
	float m_flGiveWeaponTime[MAX_PLAYERS];
};

#endif
