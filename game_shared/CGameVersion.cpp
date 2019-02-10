#include <regex>
#include <string>
#include "CGameVersion.h"

//----------------------------------------------------------------------------------------------------
// CGameVersion
//----------------------------------------------------------------------------------------------------
bool CGameVersion::TryParse(const char *str)
{
	// TODO: Use regex to search the whole string?

	m_bIsValid = false;
	strncpy(m_szString, str, sizeof(m_szString));
	m_szString[sizeof(m_szString) - 1] = '\0';

	// Format:	1.1.35+c4ddd6b+m
	//			1.1.3+345f110

	bool bIsVersionFound = false, bIsCommitFound = false;
	const char *c = str;
	int i = 0;

	// Find version
	while (*c != '\0' && i < sizeof(m_szVersion))
	{
		if (*c == '+')
		{
			bIsVersionFound = true;
			m_szVersion[i] = '\0';
			break;
		}
		if (!((*c >= '0' && *c <= '9') || *c == '.'))
			return false;	// Version contains invalid chars

		m_szVersion[i] = *c;
		c++;
		i++;
	}
	m_szVersion[sizeof(m_szVersion) - 1] = '\0';

	if (!bIsVersionFound)	// End of line or version length limit reached
		return false;

	// Use regex to parse version
	try
	{
		std::string stdstr(str);

		// Look for x.y.z
		std::regex r1("^([0-9]+)\\.([0-9]+)\\.([0-9]+)");
		std::smatch m1;
		std::regex_search(stdstr, m1, r1);
		if (m1.size() == 4)
		{
			m_iMajor = std::stoi(m1[1].str());
			m_iMinor = std::stoi(m1[2].str());
			m_iPatch = std::stoi(m1[3].str());
		}
		else
		{
			// Look for x.y
			std::regex r2("^([0-9]+)\\.([0-9]+)");
			std::smatch m2;
			std::regex_search(stdstr, m2, r2);
			if (m2.size() != 3)
				return false;	// Invalid string or something else
			m_iMajor = std::stoi(m2[1].str());
			m_iMinor = std::stoi(m2[2].str());
			m_iPatch = 0;
		}

	}
	catch (...)
	{
		return false;	// Exception thrown
	}

	// Find commit
	c++;
	i = 0;
	while (i < sizeof(m_szCommit))
	{
		if (*c == '\0' || *c == '+')
		{
			bIsCommitFound = true;
			m_szCommit[i] = '\0';
			break;
		}

		if (!((*c >= 'a' && *c <= 'f') || (*c >= '0' && *c <= '9')))
			return false;	// Only lower-case HEX chars and digits are allowed

		m_szCommit[i] = *c;
		c++;
		i++;
	}

	if (!bIsCommitFound)	// Commit length limit reached
		return false;

	// Is modified?
	if (c[0] == '+' && c[1] == 'm')
		m_bIsModified = true;

	m_bIsValid = true;
	return true;
}

bool CGameVersion::TryParseTag(const char * str)
{
	m_bIsValid = false;

	// First symbol must always be 'v'
	if (str[0] != 'v')
	{
		return false;
	}

	try
	{
		std::string stdstr(str);
		stdstr = stdstr.substr(1);	// Cut off 'v'

		// Look for x.y.z
		std::regex r1("^([0-9]+)\\.([0-9]+)\\.([0-9]+)");
		std::smatch m1;
		std::regex_search(stdstr, m1, r1);
		if (m1.size() == 4)
		{
			m_iMajor = std::stoi(m1[1].str());
			m_iMinor = std::stoi(m1[2].str());
			m_iPatch = std::stoi(m1[3].str());
		}
		else
		{
			// Look for x.y
			std::regex r2("^([0-9]+)\\.([0-9]+)");
			std::smatch m2;
			std::regex_search(stdstr, m2, r2);
			if (m2.size() != 3)
				return false;	// Invalid string or something else
			m_iMajor = std::stoi(m2[1].str());
			m_iMinor = std::stoi(m2[2].str());
			m_iPatch = 0;
		}

	}
	catch (...)
	{
		return false;	// Exception thrown
	}

	m_bIsValid = true;
	return true;
}

bool CGameVersion::operator==(const CGameVersion &rhs) const
{
	return (m_iMajor == rhs.m_iMajor) && (m_iMinor == rhs.m_iMinor) && (m_iPatch == rhs.m_iPatch);
}

bool CGameVersion::operator>(const CGameVersion &rhs) const
{
	if (m_iMajor > rhs.m_iMajor)
		return true;

	if (m_iMajor < rhs.m_iMajor)
		return false;

	if (m_iMinor > rhs.m_iMinor)
		return true;

	if (m_iMinor < rhs.m_iMinor)
		return false;

	return (m_iPatch > rhs.m_iPatch);
}

bool CGameVersion::operator>=(const CGameVersion &rhs) const
{
	if (m_iMajor > rhs.m_iMajor)
		return true;

	if (m_iMajor < rhs.m_iMajor)
		return false;

	if (m_iMinor > rhs.m_iMinor)
		return true;

	if (m_iMinor < rhs.m_iMinor)
		return false;

	return (m_iPatch >= rhs.m_iPatch);
}

bool CGameVersion::operator<(const CGameVersion &rhs) const
{
	if (m_iMajor < rhs.m_iMajor)
		return true;

	if (m_iMajor > rhs.m_iMajor)
		return false;

	if (m_iMinor < rhs.m_iMinor)
		return true;

	if (m_iMinor > rhs.m_iMinor)
		return false;

	return (m_iPatch < rhs.m_iPatch);
}

bool CGameVersion::operator<=(const CGameVersion & rhs) const
{
	if (m_iMajor < rhs.m_iMajor)
		return true;

	if (m_iMajor > rhs.m_iMajor)
		return false;

	if (m_iMinor < rhs.m_iMinor)
		return true;

	if (m_iMinor > rhs.m_iMinor)
		return false;

	return (m_iPatch <= rhs.m_iPatch);
}

