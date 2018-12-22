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

	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Think();
	virtual void Reset(void);
	void ShowScoreBoard();
	void HideScoreBoard(bool force = false);
	void UpdateClientInfo(int client);
	void EnableMousePointer(bool enable);
	bool IsVisible();
	void UpdateServerName();

private:
	float m_flScoreBoardLastUpdated = 0;
};

#endif