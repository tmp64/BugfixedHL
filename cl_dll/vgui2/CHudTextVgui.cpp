#include "CHudTextVgui.h"
#include "hud.h"
#include "cl_util.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Controls.h>

vgui2::HFont g_HudTextVgui_TextFont = 0;

static int DrawStringVGUI2(int _x, int _y, const char* string, int r, int g, int b)
{
	wchar_t buf[1024];
	vgui2::localize()->ConvertANSIToUnicode(string, buf, sizeof(buf));
	vgui2::surface()->DrawSetTextColor(r, g, b, 255);
	vgui2::surface()->DrawSetTextFont(g_HudTextVgui_TextFont);
	int x = _x;
	for (wchar_t *wsz = buf; *wsz != 0; wsz++)
	{
		wchar_t ch = *wsz;
		if (ch != ' ')
		{
			// render the character
			vgui2::surface()->DrawSetTextPos(x, _y);
			vgui2::surface()->DrawUnicodeChar(ch);
		}
		x += vgui2::surface()->GetCharacterWidth(g_HudTextVgui_TextFont, ch);
	}
	return x - _x;
}

static int DrawCharVGUI2(int x, int y, int number, int r, int g, int b)
{
	vgui2::surface()->DrawSetTextColor(r, g, b, 255);
	vgui2::surface()->DrawSetTextFont(g_HudTextVgui_TextFont);
	vgui2::surface()->DrawSetTextPos(x, y);
	vgui2::surface()->DrawUnicodeChar((wchar_t)number);
	return vgui2::surface()->GetCharacterWidth(g_HudTextVgui_TextFont, (wchar_t)number);
}

void CHudTextVgui::Init()
{
	m_pEngDrawString = gEngfuncs.pfnDrawString;
	m_pEngDrawChar = gEngfuncs.pfnDrawCharacter;
	m_pCvarVguiText = CVAR_CREATE("hud_textvgui", "0", FCVAR_ARCHIVE);
	m_iFlags |= HUD_ACTIVE;
}

void CHudTextVgui::Think(void)
{
	if (m_bIsUseVgui && !m_pCvarVguiText->value)
	{
		// Disable VGUI2 text
		gEngfuncs.pfnDrawString = m_pEngDrawString;
		gEngfuncs.pfnDrawCharacter = m_pEngDrawChar;
		m_bIsUseVgui = false;
		ConPrintf("VGUI2 HUD text is disabled. Restart the game.\n");
	}
	else if (!m_bIsUseVgui && m_pCvarVguiText->value)
	{
		// Enable VGUI2 text
		gEngfuncs.pfnDrawString = DrawStringVGUI2;
		gEngfuncs.pfnDrawCharacter = DrawCharVGUI2;
		m_bIsUseVgui = true;
		ConPrintf("VGUI2 HUD text is enabled! Restart the game.\n");
	}
}
