#ifndef CHUDTRAIN_H
#define CHUDTRAIN_H

#include "CHudBase.h"

class CHudTrain : public CHudBase
{
public:
	void Init();
	void VidInit();
	void Draw(float flTime);
	int MsgFunc_Train(const char *pszName, int iSize, void *pbuf);

private:
	HLHSPRITE m_hSprite;
	int m_iPos;

};

#endif