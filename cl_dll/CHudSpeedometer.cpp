#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "CHudSpeedometer.h"
#include <cmath>

void CHudSpeedometer::Init()
{
	m_iFlags = HUD_ACTIVE;
	
	m_pCvarSpeedometer = CVAR_CREATE("hud_speedometer", "0", FCVAR_BHL_ARCHIVE);
}

void CHudSpeedometer::VidInit()
{
}

void CHudSpeedometer::Draw(float time)
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || gEngfuncs.IsSpectateOnly())
		return;

	if (m_pCvarSpeedometer->value == 0.0f)
		return;

	if (m_iOldSpeed != m_iSpeed)
		m_fFade = HUD_FADE_TIME;

	int r, g, b;
	float a;

	if (gHUD.m_pCvarDim->value == 0)
		a = MIN_ALPHA + ALPHA_AMMO_MAX;
	else if (m_fFade > 0)
	{
		// Fade the health number back to dim
		m_fFade -= (gHUD.m_flTimeDelta * 20);
		if (m_fFade <= 0)
			m_fFade = 0;
		a = MIN_ALPHA + (m_fFade / HUD_FADE_TIME) * ALPHA_AMMO_FLASH;
	} else
		a = MIN_ALPHA;

	a *= gHUD.GetHudTransparency();
	gHUD.GetHudColor(0, 0, r, g, b);
	ScaleColors(r, g, b, a);
	gHUD.DrawHudNumberCentered(ScreenWidth / 2, ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2, m_iSpeed, r, g, b);

	m_iOldSpeed = m_iSpeed;
}

void CHudSpeedometer::UpdateSpeed(const float velocity[2])
{
	m_iSpeed = std::round(std::hypot(velocity[0], velocity[1]));
}
