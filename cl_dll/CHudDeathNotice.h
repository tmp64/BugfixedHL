#ifndef CHUDDEATHNOTICE_H
#define CHUDDEATHNOTICE_H

#include "CHudBase.h"

class CHudDeathNotice : public CHudBase
{
public:
	void Init();
	void InitHUDData();
	void VidInit();
	void Draw(float flTime);
	int MsgFunc_DeathMsg(const char *pszName, int iSize, void *pbuf);

private:
	int m_HUD_d_skull;  // sprite index of skull icon
	cvar_t *m_pCvarKillSnd;
};

#endif