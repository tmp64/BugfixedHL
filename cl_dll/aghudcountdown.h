// Martin Webrant (BulliT)

#ifndef __AGHUDCOUNTDOWN_H__
#define __AGHUDCOUNTDOWN_H__

class AgHudCountdown : public CHudBase
{
public:
	AgHudCountdown(): m_btCountdown(0) { }

	void Init() override;
	void VidInit() override;
	void Draw(float flTime) override;
	void Reset() override;

	int MsgFunc_Countdown(const char *pszName, int iSize, void *pbuf);

private:
	char m_btCountdown;
	char m_szPlayer1[32];
	char m_szPlayer2[32];
};

#endif // __AGHUDCOUNTDOWN_H__
