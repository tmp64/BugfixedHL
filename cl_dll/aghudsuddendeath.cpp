// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_SuddenDeath, SuddenDeath)

void AgHudSuddenDeath::Init()
{
	HOOK_MESSAGE(SuddenDeath);

	gHUD.AddHudElem(this);

	m_iFlags = 0;
	m_iSuddenDeath = 0;
}

void AgHudSuddenDeath::VidInit()
{
}

void AgHudSuddenDeath::Reset()
{
	m_iFlags &= ~HUD_ACTIVE;
}

void AgHudSuddenDeath::Draw(float fTime)
{
	if (gHUD.m_iIntermission)
		Reset();

	int r, g, b, a;
	UnpackRGB(r, g, b, RGB_REDISH);
	a = 255 * gHUD.GetHudTransparency();
	ScaleColors(r, g, b, a);

	AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 2, ScreenWidth, "Sudden death!", r, g, b);
}

int AgHudSuddenDeath::MsgFunc_SuddenDeath(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_iSuddenDeath = READ_BYTE();

	if (m_iSuddenDeath)
		m_iFlags |= HUD_ACTIVE;
	else
		m_iFlags &= ~HUD_ACTIVE;

	return 1;
}
