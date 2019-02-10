#ifndef CGAMEVERSION_H
#define CGAMEVERSION_H

class CGameVersion
{
public:
	inline CGameVersion() {}
	inline CGameVersion(const char *str) { TryParse(str); }

	bool TryParse(const char *str);		// Tries to parse game version (e.g. 1.1.35+c4ddd6b+m)
	bool TryParseTag(const char *str);	// Tries to parse Git tag (e.g. v1.2)
	inline bool IsValid() const { return m_bIsValid; }
	inline bool IsModified() const { return m_bIsModified; }
	inline const char *GetFullString() const { return m_szString; }
	inline const char *GetVersionString() const { return m_szVersion; }
	inline const char *GetCommitString() const { return m_szCommit; }
	inline void GetVersion(int &major, int &minor, int &patch) const
	{
		major = m_iMajor;
		minor = m_iMinor;
		patch = m_iPatch;
	}
	inline void RemovePatch()
	{
		m_iPatch = 0;
	}

	bool operator== (const CGameVersion &rhs) const;
	bool operator>  (const CGameVersion &rhs) const;
	bool operator>= (const CGameVersion &rhs) const;
	bool operator<  (const CGameVersion &rhs) const;
	bool operator<= (const CGameVersion &rhs) const;

private:
	bool m_bIsValid = false;
	bool m_bIsModified = false;
	char m_szString[64] = "";
	char m_szVersion[16] = "";
	char m_szCommit[16] = "";
	int m_iMajor = 0, m_iMinor = 0, m_iPatch = 0;
};

#endif