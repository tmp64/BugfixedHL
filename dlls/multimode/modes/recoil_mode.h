#ifndef MULTIMODE_RECOIL_MODE_H
#define MULTIMODE_RECOIL_MODE_H
#include "multimode/basemode.h"

class CRecoilMode : public CBaseMode
{
public:
	static constexpr ModeID MODE_ID = ModeID::Recoil;
	static constexpr char MODE_NAME[] = "recoil";

	CRecoilMode();

	virtual ModeID GetModeID();
	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();

	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapon(const char *classname);
};

#endif
