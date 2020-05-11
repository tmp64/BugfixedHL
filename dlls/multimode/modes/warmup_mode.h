#ifndef MULTIMODE_WARMUP_MODE_H
#define MULTIMODE_WARMUP_MODE_H
#include "multimode/basemode.h"

class CWarmupMode : public CBaseMode
{
public:
	CWarmupMode();

	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();

	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapons();
	virtual bool ShouldRespawnAmmo();
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);
};

#endif
