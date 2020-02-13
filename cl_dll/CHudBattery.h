#ifndef CHUDBATTERY_H
#define CHUDBATTERY_H

#include "CHudBase.h"

class CHudBattery : public CHudBase
{
public:
	void Init();
	void VidInit();
	void Draw(float flTime);
	int MsgFunc_Battery(const char *pszName, int iSize, void *pbuf);

private:
	HLHSPRITE m_hSprite1;
	HLHSPRITE m_hSprite2;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	int	  m_iBat;
	float m_fFade;
	int	  m_iHeight;		// width of the battery innards
};

#endif
