#ifndef MULTIMODE_SPEED_MODE_H
#define MULTIMODE_SPEED_MODE_H
#include "basemode.h"

class CSpeedMode : public CBaseMode
{
public:
	CSpeedMode();

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
