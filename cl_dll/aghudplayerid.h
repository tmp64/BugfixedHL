// Martin Webrant (BulliT)

#ifndef __AGHUDPLAYERID_H__
#define __AGHUDPLAYERID_H__

class AgHudPlayerId : public CHudBase
{
public:
	AgHudPlayerId() : m_flTurnoff(0), m_iPlayer(0), m_bTeam(false), m_iHealth(0), m_iArmour(0), m_pCvarHudPlayerId(NULL) { }

	void Init() override;
	void VidInit() override;
	void Draw(float flTime) override;
	void Reset() override;

	int MsgFunc_PlayerId(const char *pszName, int iSize, void *pbuf);

private:
	float m_flTurnoff;
	int m_iPlayer;
	bool m_bTeam;
	int m_iHealth;
	int m_iArmour;

	cvar_t *m_pCvarHudPlayerId;
};

#endif // __AGHUDPLAYERID_H__
