#include "CVoiceStatus2.h"
#include "hud.h"
#include "../game_shared/voice_status.h"

static CVoiceStatus2 g_VoiceMgr2;

bool CVoiceStatus2::IsPlayerBlocked(int iPlayerIndex)
{
	return GetClientVoiceMgr()->IsPlayerBlocked(iPlayerIndex);
}

bool CVoiceStatus2::IsPlayerAudible(int iPlayerIndex)
{
	return GetClientVoiceMgr()->IsPlayerAudible(iPlayerIndex);
}

void CVoiceStatus2::SetPlayerBlockedState(int iPlayerIndex, bool blocked)
{
	GetClientVoiceMgr()->SetPlayerBlockedState(iPlayerIndex, blocked);
}

CVoiceStatus2 *vgui2::voicemgr()
{
	return &g_VoiceMgr2;
}
