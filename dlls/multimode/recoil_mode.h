#ifndef MULTIMODE_RECOIL_MODE_H
#define MULTIMODE_RECOIL_MODE_H
#include "basemode.h"

class CRecoilMode : public CBaseMode
{
public:
	CRecoilMode();

	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();

	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapon(const char *classname);
};

#endif
