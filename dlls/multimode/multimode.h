#ifndef MULTIMODE_MULTIMODE_H
#define MULTIMODE_MULTIMODE_H

class CBaseMode;
class CHalfLifeMultimode;

enum class ModeID : int
{
	None = 0,
	DM,
	OneShot,
	Recoil,
	WpnDrop,
	Biohazard,
	SlowRockets,
	Speed,
	Boss,
	ModeCount
};

// Players with this value of m_iMultimodeScore won't be shown in the intermission stats and can never be winners
constexpr int MULTIMODE_NO_SCORE = -999999;

bool IsRunningMultimode();
bool IsRunningMultimode(ModeID mode);
CHalfLifeMultimode *GetMultimodeGR();

#endif
