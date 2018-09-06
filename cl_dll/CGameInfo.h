#ifndef CGAMEINFO_H
#define CGAMEINFO_H

class CGameInfo
{
public:
	typedef int(*Event_t)();

	void UpdateOnPlayerInfo();
	void TeamsUpdated();
	void TeamScoreOverriden(int team);
	void DeathMsg(int killer, int victim);
};

extern CGameInfo gGameInfo;

#endif