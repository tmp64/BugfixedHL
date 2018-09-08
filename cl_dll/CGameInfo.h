#ifndef CGAMEINFO_H
#define CGAMEINFO_H

class TeamFortressViewport;

class CGameInfo
{
public:
	void UpdateScoreboard();
	inline const char *GetServerName()
	{
		return m_pServerName;
	}

private:
	char *m_pServerName = nullptr;

	friend class TeamFortressViewport;
};

extern CGameInfo gGameInfo;

#endif