// Martin Webrant (BulliT)

#ifndef __AGHUDLONGJUMP_H__
#define __AGHUDLONGJUMP_H__

class AgHudLongjump : public CHudBase
{
public:
	AgHudLongjump() : m_flTurnoff(0) { }

	void Init() override;
	void VidInit() override;
	void Draw(float flTime) override;
	void Reset() override;

	int MsgFunc_Longjump(const char *pszName, int iSize, void *pbuf);

private:
	float m_flTurnoff;
};

#endif // __AGHUDLONGJUMP_H__
