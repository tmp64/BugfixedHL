#ifndef MULTIMODE_DM_MODE_H
#define MULTIMODE_DM_MODE_H
#include "multimode/basemode.h"

class CDmMode : public CBaseMode
{
public:
	static constexpr ModeID MODE_ID = ModeID::Deathmatch;
	static constexpr char MODE_NAME[] = "deathmatch";

	CDmMode();

	virtual ModeID GetModeID();
	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();
};

#endif
