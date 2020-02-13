#pragma once
#include "CHudBase.h"

class CHudCrosshair : public CHudBase
{
public:
	struct custom_crosshair_cvars_t
	{
		cvar_t *enable;
		cvar_t *red;
		cvar_t *green;
		cvar_t *blue;
		cvar_t *gap;
		cvar_t *size;
		cvar_t *thickness;
#ifdef USE_VGUI2
		cvar_t *outline_thickness;
#endif
		cvar_t *dot;
		cvar_t *t;
	};
	custom_crosshair_cvars_t m_CustomCrosshair;

	CHudCrosshair();
	virtual void Init();
	virtual void Draw(float flTime);
};

