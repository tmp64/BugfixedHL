#ifndef CVOICESTATUS_H
#define CVOICESTATUS_H

class CVoiceStatus2
{
public:
	// returns true if the target client has been banned
	// playerIndex is of range [1; maxplayers]
	bool	IsPlayerBlocked(int iPlayerIndex);

	// returns false if the player can't hear the other client due to game rules (eg. the other team)
	bool    IsPlayerAudible(int iPlayerIndex);

	// blocks the target client from being heard
	void	SetPlayerBlockedState(int iPlayerIndex, bool blocked);
};

namespace vgui2
{
	CVoiceStatus2 *voicemgr();
}

#endif