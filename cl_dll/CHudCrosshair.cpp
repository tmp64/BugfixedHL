#include "CHudCrosshair.h"
#include "CHudAmmo.h"
#include "hud.h"
#include "cl_util.h"

#ifdef USE_VGUI2
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#endif

CHudCrosshair::CHudCrosshair()
{
}

void CHudCrosshair::Init()
{
	m_CustomCrosshair.enable = CVAR_CREATE("cl_crosshair_custom", "0", FCVAR_ARCHIVE);
	m_CustomCrosshair.red = CVAR_CREATE("cl_crosshair_red", "0", FCVAR_ARCHIVE);
	m_CustomCrosshair.green = CVAR_CREATE("cl_crosshair_green", "255", FCVAR_ARCHIVE);
	m_CustomCrosshair.blue = CVAR_CREATE("cl_crosshair_blue", "255", FCVAR_ARCHIVE);
	m_CustomCrosshair.gap = CVAR_CREATE("cl_crosshair_gap", "8", FCVAR_ARCHIVE);
	m_CustomCrosshair.size = CVAR_CREATE("cl_crosshair_size", "6", FCVAR_ARCHIVE);
	m_CustomCrosshair.thickness = CVAR_CREATE("cl_crosshair_thickness", "2", FCVAR_ARCHIVE);
#ifdef USE_VGUI2
	m_CustomCrosshair.outline_thickness = CVAR_CREATE("cl_crosshair_outline_thickness", "0", FCVAR_ARCHIVE);
#endif
	m_CustomCrosshair.dot = CVAR_CREATE("cl_crosshair_dot", "0", FCVAR_ARCHIVE);
	m_CustomCrosshair.t = CVAR_CREATE("cl_crosshair_t", "0", FCVAR_ARCHIVE);

	m_iFlags |= HUD_ACTIVE; //!!!
}

void CHudCrosshair::Draw(float flTime)
{
	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return;

	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return;

	if (!(m_iFlags & HUD_ACTIVE))
		return;

	if (!gHUD.m_Ammo->m_pWeapon)
		return;

	// Draw custom crosshair if enabled
	if (m_CustomCrosshair.enable->value && !(gHUD.m_Ammo->m_fOnTarget &&gHUD.m_Ammo->m_pWeapon->hAutoaim))
	{
		int cx = ScreenWidth / 2;
		int cy = ScreenHeight / 2;
		int r = m_CustomCrosshair.red->value;
		int g = m_CustomCrosshair.green->value;
		int b = m_CustomCrosshair.blue->value;
		int a = 255;
		int gap = m_CustomCrosshair.gap->value / 2;
		int thick = m_CustomCrosshair.thickness->value;
		int size = m_CustomCrosshair.size->value;
		bool t = m_CustomCrosshair.t->value;

		// Outline can only be drawn using vgui2 because FillRGBA can't draw anything in black
#ifdef USE_VGUI2
		int outline = m_CustomCrosshair.outline_thickness->value;

		// Draw outline
		if (outline > 0)
		{
			// Don't read that if you don't want eye cancer
			vgui2::surface()->DrawSetColor(0, 0, 0, a);
			vgui2::surface()->DrawFilledRect(cx + gap - outline, cy - thick / 2 - outline,
				cx + gap - outline + size + outline * 2, cy - thick / 2 - outline + thick + outline * 2);
			vgui2::surface()->DrawFilledRect(cx - gap - size - outline, cy - thick / 2 - outline,
				cx - gap - size - outline + size + outline * 2, cy - thick / 2 - outline + thick + outline * 2);
			vgui2::surface()->DrawFilledRect(cx - thick / 2 - outline, cy + gap - outline,
				cx - thick / 2 - outline + thick + outline * 2, cy + gap - outline + size + outline * 2);
			vgui2::surface()->DrawFilledRect(cx - thick / 2 - outline, cy - gap - size - outline,
				cx - thick / 2 - outline + thick + outline * 2, cy - gap - size - outline + size + outline * 2);
		}
#endif

#ifdef USE_VGUI2
		vgui2::surface()->DrawSetColor(r, g, b, a);
#endif

		// Draw dot
		if (m_CustomCrosshair.dot->value)
		{
#ifdef USE_VGUI2
			vgui2::surface()->DrawFilledRect(cx - thick / 2, cy - thick / 2, thick + cx - thick / 2, thick + cy - thick / 2);
#else
			FillRGBA(cx - thick / 2, cy - thick / 2, thick, thick, r, g, b, a);
#endif
		}

		// Draw crosshair
#ifdef USE_VGUI2
		// Use vgui2::ISurface to get rid of nasty color blending caused by FillRGBA
		vgui2::surface()->DrawFilledRect(cx + gap, cy - thick / 2, cx + gap + size, cy - thick / 2 + thick);
		vgui2::surface()->DrawFilledRect(cx - gap - size, cy - thick / 2, cx - gap, cy - thick / 2 + thick);
		vgui2::surface()->DrawFilledRect(cx - thick / 2, cy + gap, cx - thick / 2 + thick, cy + gap + size);
		if (!t)
			vgui2::surface()->DrawFilledRect(cx - thick / 2, cy - gap - size, cx - thick / 2 + thick, cy - gap);
#else
		FillRGBA(cx + gap, cy - thick / 2, size, thick, r, g, b, a);
		FillRGBA(cx - gap - size, cy - thick / 2, size, thick, r, g, b, a);
		FillRGBA(cx - thick / 2, cy + gap, thick, size, r, g, b, a);
		if (!t)
			FillRGBA(cx - thick / 2, cy - gap - size, thick, size, r, g, b, a);
#endif
	}
}
