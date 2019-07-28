#include <regex>
#include <string>
#include "CGameVersion.h"

//----------------------------------------------------------------------------------------------------
// CGameVersion
//----------------------------------------------------------------------------------------------------
bool CGameVersion::TryParse(const char *cstr)
{
	// TODO: Use regex to search the whole string?

	try
	{
		m_bIsValid = false;
		strncpy(m_szString, cstr, sizeof(m_szString));
		m_szString[sizeof(m_szString) - 1] = '\0';

		std::string str = cstr;
		std::vector<std::string> parts;

		// Split string by '+'
		{
			size_t from = 0;
			for (size_t i = 0; i <= str.size(); i++)
			{
				if (str[i] == '+' || str[i] == '\0')
				{
					parts.push_back(str.substr(from, i - from));
					from = i + 1;
				}
			}
		}

		if (parts.size() < 2 || parts.size() > 4)
			return false;

		// Parse version
		{
			size_t dash = parts[0].find('-');
			std::string version;
			if (dash == std::string::npos)
			{
				version = parts[0];
			}
			else
			{
				version = parts[0].substr(0, dash);
				std::string tag = parts[0].substr(dash + 1);
				strncpy(m_szTag, tag.c_str(), sizeof(m_szTag));
				m_szTag[sizeof(m_szTag) - 1] = '\0';
			}

			// Look for x.y.z
			std::regex r1("^([0-9]+)\\.([0-9]+)\\.([0-9]+)");
			std::smatch m1;
			std::regex_search(version, m1, r1);
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
				std::regex_search(version, m2, r2);
				if (m2.size() != 3)
					return false;	// Invalid string or something else
				m_iMajor = std::stoi(m2[1].str());
				m_iMinor = std::stoi(m2[2].str());
				m_iPatch = 0;
			}
		}

		// Commit hash
		if (parts[1].size() != 7)
			return false;
		for (int i = 0; i < 7; i++)
		{
			if (!((parts[1][i] >= 'a' && parts[1][i] <= 'f') || (parts[1][i] >= '0' && parts[1][i] <= '9')))
				return false;	// Only lower-case HEX digits are allowed in commit hash
		}
		strncpy(m_szCommit, parts[1].c_str(), sizeof(m_szCommit));
		m_szCommit[sizeof(m_szCommit) - 1] = '\0';

		// Build metadata
		if (parts.size() >= 3 && parts[2] != "m")
		{
			strncpy(m_szMetadata, parts[2].c_str(), sizeof(m_szMetadata));
			m_szMetadata[sizeof(m_szMetadata) - 1] = '\0';
		}
		else
		{
			m_szMetadata[0] = '\0';
		}

		// Dirty tag
		if (parts.size() >= 3 && parts[parts.size() - 1] == "m")
			m_bIsModified = true;
		else
			m_bIsModified = false;

		m_bIsValid = true;
		return true;
	}
	catch (const std::exception &)
	{
		m_bIsValid = false;
		return false;
	}

	return false;
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

	if (m_iPatch > rhs.m_iPatch)
		return true;
	if (m_iPatch < rhs.m_iPatch)
		return false;

	if (!m_szTag[0] && rhs.m_szTag[0])
		return true;
	else
		return false;
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

	if (m_iPatch > rhs.m_iPatch)
		return true;
	if (m_iPatch < rhs.m_iPatch)
		return false;

	if ((!m_szTag[0] && rhs.m_szTag[0]) || !strcmp(m_szTag, rhs.m_szTag))
		return true;
	else
		return false;
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

	if (m_iPatch < rhs.m_iPatch)
		return true;
	if (m_iPatch > rhs.m_iPatch)
		return false;

	if (m_szTag[0] && !rhs.m_szTag[0])
		return true;
	else
		return false;
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

	if ((m_szTag[0] && !rhs.m_szTag[0]) || !strcmp(m_szTag, rhs.m_szTag))
		return true;
	else
		return false;
}

