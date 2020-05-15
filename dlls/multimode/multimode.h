#ifndef MULTIMODE_MULTIMODE_H
#define MULTIMODE_MULTIMODE_H

class CBaseMode;
class CHalfLifeMultimode;

enum class ModeID : int
{
	None = 0,
	WarmUp,
	Deathmatch,
	OneShot,
	Recoil,
	WpnDrop,
	Biohazard,
	SlowRockets,
	Speed,
	BossFight,
	ModeCount
};

/**
 * Players with this value of m_iMultimodeScore won't be shown in the intermission stats and can never be winners
 */
constexpr int MULTIMODE_NO_SCORE = -999999;

/**
 * Returns true if the server is running Multimode at the moment.
 */
bool IsRunningMultimode();

/**
 * Returns true if `mode` is being played at the moment.
 */
bool IsRunningMultimode(ModeID mode);

/**
 * Returns instance of CHalfLifeMultimode.
 * MUST only be called if IsRunningMultimode() == true.
 */
CHalfLifeMultimode *GetMultimodeGR();

/**
 * Returns GetMultimodeGR()->GetBaseMode().
 * MUST only be called if IsRunningMultimode() == true.
 */
CBaseMode *GetRunningMultimodeBase();

template <typename T>
inline T *GetRunningMultimode()
{
    return static_cast<T *>(GetRunningMultimodeBase());
}

#endif
