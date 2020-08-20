#ifndef CHUDSAYTEXT_H
#define CHUDSAYTEXT_H

#include "CHudBase.h"

extern "C" void DLLEXPORT ChatInputPosition(int *x, int *y);

class CHudSayText : public CHudBase
{
public:
#ifdef USE_VGUI2
	cvar_t *m_pCvarOldChat = nullptr;
#endif

	void Init();
	void InitHUDData();
	void VidInit();
	void Draw(float flTime);
	int MsgFunc_SayText(const char *pszName, int iSize, void *pbuf);
	void SayTextPrint(const char *pszBuf, int iBufSize, int clientIndex = -1);
	void EnsureTextFitsInOneLineAndWrapIfHaveTo(int line);
	friend class CHudSpectator;

private:
	struct cvar_s *	m_HUD_saytext;
	struct cvar_s *	m_HUD_saytext_time;
	cvar_t	*m_pCvarConSayColor;
	cvar_t	*m_pCvarOldInputPos = nullptr;
	cvar_t  *m_pCvarSound = nullptr;

	friend void ChatInputPosition(int *x, int *y);
	friend class CBaseHudChat;
};

#endif