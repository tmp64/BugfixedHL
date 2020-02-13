#ifndef MULTIMODE_BIOHAZARD_MODE_H
#define MULTIMODE_DM_MODE_H
#include "basemode.h"

class CBiohazardMode : public CBaseMode
{
public:
	CBiohazardMode();

	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual void GetShortTitleColor(byte &r, byte &g, byte &b);
	virtual const char *GetDescription();

	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapons();
	virtual bool ShouldRespawnAmmo();
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual void PlayerSpawn(CBasePlayer *pPlayer);

private:
	enum class WeaponType
	{
		None = 0,
		Snark,
		Hornet
	};

	struct PlayerInfo
	{
		WeaponType wpnType = WeaponType::None;
		float flNextTimerCheck = 0;
	};

	PlayerInfo m_Players[MAX_PLAYERS];
};

#endif
