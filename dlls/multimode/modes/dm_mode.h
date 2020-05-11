#ifndef MULTIMODE_DM_MODE_H
#define MULTIMODE_DM_MODE_H
#include "multimode/basemode.h"

class CDmMode : public CBaseMode
{
public:
	CDmMode();

	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();
};

#endif
