#ifndef CGAMEVERSION_H
#define CGAMEVERSION_H

/**
 * @brief CGameVersion contains parsed version string.
 * E.g. "1.2.35+c4ddd6b+m" ->
 * major = 1
 * minor = 2
 * patch = 35
 * commit = "c4ddd6b"
 * modified = true (+m)
 *
 * This class is completely non-virtual.
 * Any changes to sections marked with [API-BREAKING] may/will break compatibility
 * with the server interface.
 */
class CGameVersion
{
public:
	inline CGameVersion() {}
	inline CGameVersion(const char *str) { TryParse(str); }

	bool TryParse(const char *str);		// Tries to parse game version (e.g. 1.1.35+c4ddd6b+m)
	bool TryParseTag(const char *str);	// Tries to parse Git tag (e.g. v1.2)
	inline bool IsValid() const;
	inline bool IsModified() const;
	inline const char *GetFullString() const;
	inline const char *GetVersionString() const;
	inline const char *GetCommitString() const;
	inline void GetVersion(int &major, int &minor, int &patch) const;
	inline void RemovePatch();

	bool operator== (const CGameVersion &rhs) const;
	bool operator>  (const CGameVersion &rhs) const;
	bool operator>= (const CGameVersion &rhs) const;
	bool operator<  (const CGameVersion &rhs) const;
	bool operator<= (const CGameVersion &rhs) const;

private:
	//-----------------[API-BREAKING V1.0 BEGIN]-----------------
	bool m_bIsValid = false;
	bool m_bIsModified = false;
	char m_szString[64] = "";
	char m_szVersion[16] = "";
	char m_szCommit[16] = "";
	int m_iMajor = 0, m_iMinor = 0, m_iPatch = 0;
	//-----------------[API-BREAKING V1.0 END]-----------------
};

inline bool CGameVersion::IsValid() const
{
	return m_bIsValid;
}

inline bool CGameVersion::IsModified() const
{
	return m_bIsModified;
}

inline const char *CGameVersion::GetFullString() const
{
	return m_szString;
}

inline const char *CGameVersion::GetVersionString() const
{
	return m_szVersion;
}

inline const char *CGameVersion::GetCommitString() const
{
	return m_szCommit;
}

inline void CGameVersion::GetVersion(int &major, int &minor, int &patch) const
{
	major = m_iMajor;
	minor = m_iMinor;
	patch = m_iPatch;
}

inline void CGameVersion::RemovePatch()
{
	m_iPatch = 0;
}

#endif