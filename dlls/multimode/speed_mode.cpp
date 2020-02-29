#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode_gamerules.h"
#include "game.h"
#include "speed_mode.h"

ConVar mp_mm_speed_maxspeed("mp_mm_speed_maxspeed", "600", 0);
ConVar mp_mm_speed_accel("mp_mm_speed_accel", "50", 0);

CSpeedMode::CSpeedMode() : CBaseMode()
{
}

const char *CSpeedMode::GetModeName()
{
	return "SpeedMode";
}

const char *CSpeedMode::GetShortTitle()
{
	return "Ludicrous Speed";
}

const char *CSpeedMode::GetDescription()
{
	return "Light speed too slow. We're gonna have to go right to Ludicrous Speed!";
}

void CSpeedMode::OnStart()
{
	m_flOldMaxSpeed = CVAR_GET_FLOAT("sv_maxspeed");
	CVAR_SET_FLOAT("sv_maxspeed", mp_mm_speed_maxspeed.Get());
}

void CSpeedMode::OnEnd()
{
	CVAR_SET_FLOAT("sv_maxspeed", m_flOldMaxSpeed);
}

void CSpeedMode::PlayerThink(CBasePlayer *pPlayer)
{
	if (GetMultimodeGR()->GetState() == CHalfLifeMultimode::State::Game)
	{
		float angle = pPlayer->edict()->v.v_angle[1];
		angle = angle * M_PI / 180.f;
		float x = cos(angle);
		float y = sin(angle);
		Vector addVel = Vector(x, y, 0);
		addVel = addVel * mp_mm_speed_accel.Get();

		Vector vel = pPlayer->edict()->v.velocity + addVel;
		float len = vel.Length2D();

		if (vel.Length2D() > mp_mm_speed_maxspeed.Get())
		{
			vel[0] *= mp_mm_speed_maxspeed.Get() / len;
			vel[1] *= mp_mm_speed_maxspeed.Get() / len;
		}

		pPlayer->edict()->v.velocity = vel;
	}
}
