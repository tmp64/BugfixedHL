#ifndef CGAMEINFO_H
#define CGAMEINFO_H

#include <SDK_Color.h>

class TeamFortressViewport;

class CGameInfo
{
public:
	void UpdateScoreboard();

	inline const char *GetServerName()
	{
		return m_pServerName;
	}

	inline SDK_Color CGameInfo::GetTeamColor(int team)
	{
		return m_pTeamColors[team % (sizeof(m_pTeamColors) / sizeof(SDK_Color))];
	}

private:
	char *m_pServerName = nullptr;

	SDK_Color m_pTeamColors[5] = {
		SDK_Color(216, 216, 216, 255),	// "Off" white (default)
		SDK_Color(125, 165, 210, 255),	// Blue
		SDK_Color(200, 90, 70, 255),	// Red
		SDK_Color(225, 205, 45, 255),	// Yellow
		SDK_Color(145, 215, 140, 255)	// Green
	};

	friend class TeamFortressViewport;
};

extern CGameInfo gGameInfo;

#endif