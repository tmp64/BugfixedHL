#ifndef MULTIMODE_SPEED_MODE_H
#define MULTIMODE_SPEED_MODE_H
#include "multimode/basemode.h"

class CSpeedMode : public CBaseMode
{
public:
	static constexpr ModeID MODE_ID = ModeID::Speed;
	static constexpr char MODE_NAME[] = "speed";

	CSpeedMode();

	virtual ModeID GetModeID();
	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();

	virtual void OnStart();
	virtual void OnEnd();
	virtual void PlayerThink(CBasePlayer *pPlayer);

private:
	float m_flOldMaxSpeed = 0;
};

#endif
