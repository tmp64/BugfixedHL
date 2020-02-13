// Martin Webrant (BulliT)

#ifndef __AGHUDSUDDENDEATH_H__
#define __AGHUDSUDDENDEATH_H__

class AgHudSuddenDeath : public CHudBase
{
public:
	AgHudSuddenDeath(): m_iSuddenDeath(0) { }

	void Init() override;
	void VidInit() override;
	void Draw(float flTime) override;
	void Reset() override;

	int MsgFunc_SuddenDeath(const char *pszName, int iSize, void *pbuf);

private:
	int m_iSuddenDeath;
};

#endif // __AGHUDSUDDENDEATH_H__
