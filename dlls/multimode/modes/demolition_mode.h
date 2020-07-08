#ifndef MULTIMODE_DEMOLITION_MODE_H
#define MULTIMODE_DEMOLITION_MODE_H
#include "multimode/basemode.h"

class CDemolitionMode : public CBaseMode
{
public:
	static constexpr ModeID MODE_ID = ModeID::Deathmatch;
	static constexpr char MODE_NAME[] = "demolition";

	CDemolitionMode();

	virtual ModeID GetModeID();
	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();

	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual void PlayerSpawn(CBasePlayer *pPlayer);

	virtual bool ShouldRespawnWeapons();
	virtual bool ShouldRespawnAmmo();
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);
};

#endif
