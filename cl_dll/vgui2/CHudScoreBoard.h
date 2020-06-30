#ifndef CHUDSCOREBOARD_H
#define CHUDSCOREBOARD_H

#include "CHudBase.h"

class CScorePanel;

class CHudScoreBoard : public CHudBase
{
public:
	CScorePanel *m_pScorePanel = nullptr;
	cvar_t* m_CvarMouseBtn = nullptr;
	cvar_t* m_CvarAvatars = nullptr;
	cvar_t* m_CvarLoss = nullptr;
	cvar_t* m_CvarEffSort = nullptr;
	cvar_t* m_CvarEffType = nullptr;
	cvar_t* m_CvarEffPercent = nullptr;
	cvar_t* m_CvarShowSteamId = nullptr;
	cvar_t* m_CvarShowEff = nullptr;
	cvar_t *m_CvarSize = nullptr;
	cvar_t *m_CvarSpacingNormal = nullptr;
	cvar_t *m_CvarSpacingCompact = nullptr;

	void Init();
	void VidInit();
	void Draw(float flTime);
	void Think();
	virtual void Reset(void);
	void ShowScoreBoard();
	void HideScoreBoard(bool force = false);
	void UpdateClientInfo(int client);
	void EnableMousePointer(bool enable);
	bool IsVisible();
	void UpdateServerName();
	void InitHUDData();

	// Sends the key event to the scoreboard
	// Returns true if event was handled and should not be passed to the engine (HUD_Key_Event returns 0)
	bool HandleKeyEvent(int down, int keynum, const char *pszCurrentBinding);

private:
	float m_flScoreBoardLastUpdated = 0;
};

#endif