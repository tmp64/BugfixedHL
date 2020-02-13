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
	ModeCount
};

bool IsRunningMultimode();
bool IsRunningMultimode(ModeID mode);
CHalfLifeMultimode *GetMultimodeGR();

#endif
