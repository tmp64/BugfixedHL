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

	bool TryParse(const char *cstr);		// Tries to parse game version (e.g. 1.3.1-dev+abababa+ci.master.228+m)
	bool TryParseTag(const char *str);	// Tries to parse Git tag (e.g. v1.2)
	inline bool IsValid() const;
	inline bool IsModified() const;
	inline const char *GetFullString() const;
	inline const char *GetCommitString() const;
	inline void GetVersion(int &major, int &minor, int &patch) const;
	inline void RemovePatch();

	bool operator== (const CGameVersion &rhs) const;
	bool operator>  (const CGameVersion &rhs) const;
	bool operator>= (const CGameVersion &rhs) const;
	bool operator<  (const CGameVersion &rhs) const;
	bool operator<= (const CGameVersion &rhs) const;

private:
	//-----------------[API-BREAKING V2.0 BEGIN]-----------------
	bool m_bIsValid = false;
	bool m_bIsModified = false;
	char m_szString[256] = "";		// e.g. 1.3.1-dev+abababa+ci.master.228
	char m_szTag[32] = "";			// e.g. dev
	char m_szCommit[16] = "";		// e.g. abababa
	char m_szMetadata[128] = "";	// e.g. ci.master.228
	int m_iMajor = 0, m_iMinor = 0, m_iPatch = 0;
	//-----------------[API-BREAKING V2.0 END]-----------------
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