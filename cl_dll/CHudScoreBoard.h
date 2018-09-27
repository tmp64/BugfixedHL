#ifndef CHUDSCOREBOARD_H
#define CHUDSCOREBOARD_H

#include "CHudBase.h"

class CScorePanel;

class CHudScoreBoard : public CHudBase
{
public:
	CScorePanel *m_pScorePanel = nullptr;
	cvar_t* m_CvarMouseBtn = nullptr;

	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Think();
	void ShowScoreBoard();
	void HideScoreBoard(bool force = false);
	void UpdateClientInfo(int client);
	void EnableMousePointer(bool enable);
	bool IsVisible();

private:
	float m_flScoreBoardLastUpdated = 0;
};

#endif