#include <cassert>
#include <cstring>
#include "CGameVersion.h"


CGameVersion::CGameVersion()
{
	memset(&m_SemVer, 0, sizeof(m_SemVer));
}

CGameVersion::CGameVersion(IGameVersion *copy) : CGameVersion()
{
	assert(copy->IsValid());

	// Copy m_SemVer
	auto fnCopyCStr = [](const char *from, char *&to)
	{
		if (!from)
			return;
		size_t len = strlen(from);
		to = (char *)malloc(len + 1);
		memcpy(to, from, len + 1);
	};

	m_SemVer.major = copy->GetMajor();
	m_SemVer.minor = copy->GetMinor();
	m_SemVer.patch = copy->GetPatch();

	char buf[128];
	copy->GetBuildMetadata(buf, sizeof(buf));
	fnCopyCStr(buf, m_SemVer.metadata);
	copy->GetTag(buf, sizeof(buf));
	fnCopyCStr(buf, m_SemVer.prerelease);

	// Copy other
	m_bIsValid = copy->IsValid();
	copy->GetBranch(buf, sizeof(buf));
	m_Branch = buf;
	copy->GetCommitHash(buf, sizeof(buf));
	m_CommitHash = buf;
	m_bIsDirty = copy->IsDirtyBuild();
}

CGameVersion::CGameVersion(const char *pszVersion) : CGameVersion()
{
	TryParse(pszVersion);
}

CGameVersion::~CGameVersion()
{
	semver_free(&m_SemVer);
}

bool CGameVersion::TryParse(const char *pszVersion)
{
	m_bIsValid = false;

	// Parse with semver.c
	semver_free(&m_SemVer);
	if (semver_parse(pszVersion, &m_SemVer))
		return false;
	m_bIsValid = true;

	// Clear metadata
	m_Branch.clear();
	m_CommitHash.clear();
	m_bIsDirty = false;

	// Prase metadata
	std::string metadata(m_SemVer.metadata);
	size_t branchDot = metadata.find('.');
	if (branchDot != std::string::npos)
	{
		// Get branch
		m_Branch = metadata.substr(0, branchDot);

		// Get commit hash
		size_t hashDot = metadata.find('.');
		size_t hashLen = hashDot;
		if (hashLen != std::string::npos)
		{
			hashLen = hashLen - branchDot - 1;
		}

		m_CommitHash = metadata.substr(branchDot + 1, hashLen);

		// Check branch for valid chars
		bool hashValid = !m_CommitHash.empty();
		for (char c : m_CommitHash)
		{
			if (!(
				(c >= '0' && c <= '9') ||
				(c >= 'a' && c <= 'f')
				))
			{
				hashValid = false;
				break;
			}
		}

		if (!hashValid)
		{
			m_CommitHash.clear();
		}
		else
		{
			m_bIsDirty =
				hashDot != std::string::npos &&
				hashDot == metadata.size() - 2 &&
				metadata[metadata.size() + 1] == 'm';
		}
	}
	
	return true;
}

//-------------------------------------------------------------------
// IGameVersion overrides
//-------------------------------------------------------------------
void CGameVersion::DeleteThis()
{
	delete this;
}

bool CGameVersion::IsValid() const
{
	return m_bIsValid;
}

int CGameVersion::ToInt() const
{
	assert(IsValid());
	return semver_numeric(const_cast<semver_t *>(&m_SemVer));
}

/*IGameVersion *CGameVersion::MakeCopy() const
{
	assert(IsValid());
	CGameVersion *p = new CGameVersion();

	// Copy m_SemVer
	auto fnCopyCStr = [](const char *from, char *&to)
	{
		if (!from)
			return;
		size_t len = strlen(from);
		to = (char *)malloc(len + 1);
		memcpy(to, from, len + 1);
	};

	p->m_SemVer.major = m_SemVer.minor;
	p->m_SemVer.minor = m_SemVer.minor;
	p->m_SemVer.patch = m_SemVer.patch;
	fnCopyCStr(m_SemVer.metadata, p->m_SemVer.metadata);
	fnCopyCStr(m_SemVer.prerelease, p->m_SemVer.prerelease);

	// Copy other
	p->m_bIsValid = m_bIsValid;
	p->m_Branch = m_Branch;
	p->m_CommitHash = m_CommitHash;
	p->m_bIsDirty = m_bIsDirty;

	return p;
}*/

void CGameVersion::GetVersion(int &major, int &minor, int &patch) const
{
	assert(IsValid());
	major = GetMajor();
	minor = GetMinor();
	patch = GetPatch();
}

int CGameVersion::GetMajor() const
{
	assert(IsValid());
	return m_SemVer.major;
}

int CGameVersion::GetMinor() const
{
	assert(IsValid());
	return m_SemVer.minor;
}

int CGameVersion::GetPatch() const
{
	assert(IsValid());
	return m_SemVer.patch;
}

bool CGameVersion::GetTag(char *buf, int size) const
{
	assert(IsValid());

	if (!m_SemVer.prerelease)
		return false;

	strncpy(buf, m_SemVer.prerelease, size);
	buf[size - 1] = '\0';
	return true;
}

bool CGameVersion::GetBuildMetadata(char *buf, int size) const
{
	assert(IsValid());

	if (!m_SemVer.metadata)
		return false;

	strncpy(buf, m_SemVer.metadata, size);
	buf[size - 1] = '\0';
	return true;
}

bool CGameVersion::GetBranch(char *buf, int size) const
{
	assert(IsValid());

	if (m_Branch.empty())
		return false;

	strncpy(buf, m_Branch.c_str(), size);
	buf[size - 1] = '\0';
	return true;
}

bool CGameVersion::GetCommitHash(char *buf, int size) const
{
	assert(IsValid());

	if (m_CommitHash.empty())
		return false;

	strncpy(buf, m_CommitHash.c_str(), size);
	buf[size - 1] = '\0';
	return true;
}

bool CGameVersion::IsDirtyBuild() const
{
	assert(IsValid());
	return m_bIsDirty;
}

bool CGameVersion::operator==(const CGameVersion &rhs) const
{
	return semver_eq(m_SemVer, rhs.m_SemVer);
}

bool CGameVersion::operator!=(const CGameVersion &rhs) const
{
	return semver_neq(m_SemVer, rhs.m_SemVer);
}

bool CGameVersion::operator>(const CGameVersion &rhs) const
{
	return semver_gt(m_SemVer, rhs.m_SemVer);
}

bool CGameVersion::operator<(const CGameVersion &rhs) const
{
	return semver_lt(m_SemVer, rhs.m_SemVer);
}

bool CGameVersion::operator>=(const CGameVersion &rhs) const
{
	return semver_gte(m_SemVer, rhs.m_SemVer);
}

bool CGameVersion::operator<=(const CGameVersion &rhs) const
{
	return semver_lte(m_SemVer, rhs.m_SemVer);
}
