#ifndef MULTIMODE_ONESHOT_H
#define MULTIMODE_ONESHOT_H
#include "multimode/basemode.h"

class COneshotMode : public CBaseMode
{
public:
	static constexpr ModeID MODE_ID = ModeID::OneShot;
	static constexpr char MODE_NAME[] = "one_shot";

	COneshotMode();

	virtual ModeID GetModeID();
	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual void GetShortTitleColor(byte &r, byte &g, byte &b);
	virtual const char *GetDescription();

	virtual void OnFreezeStart();
	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapons();
	virtual bool ShouldRespawnAmmo();
	virtual void Think();
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);

private:

};

#endif
