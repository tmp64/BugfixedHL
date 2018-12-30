#include <regex>
#include "extdll.h"
#include "util.h"
#include "edict.h"
#include "cbase.h"
#include "gamerules.h"
#include "CBugfixedServer.h"

#if USE_METAMOD
#include <meta_api.h>
#endif

CBugfixedServer *gBugfixedServer = nullptr;

#ifdef SERVER_DLL
void *AGHL_GetServerInterface(int version, int *srvVersion)
{
	if (srvVersion)
		*srvVersion = BUGFIXEDSERVERIFACE_VERSION;
	if (version > BUGFIXEDSERVERIFACE_VERSION)
		return nullptr;
	return gBugfixedServer;
}
#endif

//----------------------------------------------------------------------------------------------------
// CClientVersion
//----------------------------------------------------------------------------------------------------
bool CClientVersion::TryParse(const char *str)
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

//----------------------------------------------------------------------------------------------------
// CBugfixedServer
//----------------------------------------------------------------------------------------------------
CBugfixedServer::CBugfixedServer(enginefuncs_t *engfuncs, globalvars_t *globalvars)
{
	m_pEngFuncs = engfuncs;
	m_pGlobalVars = globalvars;
}

void CBugfixedServer::ClientConnect(edict_t *pEntity)
{
	int idx = ENTINDEX(pEntity);
	
	if (idx < 1 || idx > 32)
	{
#if USE_METAMOD
		LOG_ERROR(PLID, "CBugfixedServer::ClientConnect: player has invalid index (%d)", idx);
#else
		UTIL_LogPrintf("CBugfixedServer::ClientConnect: player has invalid index (%d)\n", idx);
#endif
		return;
	}

	// Reset player data
	ResetPlayerData(idx);

	// Query vars
	m_pEngFuncs->pfnQueryClientCvarValue2(pEntity, "aghl_version", REQUESTID_VERSION + idx);
	m_pEngFuncs->pfnQueryClientCvarValue2(pEntity, "aghl_supports", REQUESTID_SUPPORTS + idx);
	m_pEngFuncs->pfnQueryClientCvarValue2(pEntity, "hud_colortext", REQUESTID_COLOR + idx);
}

void CBugfixedServer::CvarValueCallback(const edict_t *pEnt, int requestID, const char *cvarName, const char *value)
{
	int idx = ENTINDEX(pEnt);
	if (idx < 1 || idx > 32)
		return;	// Invalid pEnt
	if (requestID > REQUESTID_VERSION && requestID <= REQUESTID_SUPPORTS)
	{
		// aghl_version
		m_pClientInfo[idx].version.TryParse(value);
	}
	else if (requestID > REQUESTID_SUPPORTS && requestID <= REQUESTID_COLOR)
	{
		// aghl_supports
		unsigned int iValue = strtoul(value, nullptr, 10);
		if (iValue == ULONG_MAX)
			iValue = 0;
		m_pClientInfo[idx].supports = (E_ClientSupports)iValue;
	}
	else if (requestID > REQUESTID_COLOR && requestID <= REQUESTID_END)
	{
		// hud_colortext
		int iValue = atoi(value);
		m_pClientInfo[idx].isColorEnabled = iValue == 1;
	}
}

/*void CBugfixedServer::Think()
{
}*/

void CBugfixedServer::ResetPlayerData(int idx)
{
	m_pClientInfo[idx].isColorEnabled = false;
	m_pClientInfo[idx].supports = AGHL_SUPPORTS_NONE;
	m_pClientInfo[idx].version = CClientVersion();
}

#if 0
int CBugfixedServer::strcopy(char *to, const char *from, int toSize)
{
	int i = 0;
	while (i < toSize && *from != '\0')
	{
		*to = *from;
		to++;
		from++;
		i++;
	}
	if (i == toSize)
		*(to - 1) = '\0';
	else
		*to = '\0';
	return i;
}
#endif

//----------------------------------------------------------------------------------------------------
// IBugfixedServer methods
//----------------------------------------------------------------------------------------------------
void *CBugfixedServer::GetGameRulesPtr()
{
#ifdef SERVER_DLL
	return &g_pGameRules;
#else
	return nullptr;
#endif
}

E_ClientSupports CBugfixedServer::GetPlayerSupports(int idx)
{
	if (idx < 0 || idx > 32)
		return AGHL_SUPPORTS_NONE;
	return m_pClientInfo[idx].supports;
}

bool CBugfixedServer::IsColoredTextEnabled(int idx)
{
	if (idx < 0 || idx > 32)
		return false;
	return m_pClientInfo[idx].isColorEnabled;
}

bool CBugfixedServer::GetClientVersionString(int idx, char *buf, int bufSize)
{
	if (idx < 0 || idx > 32)
		return false;
	if (!m_pClientInfo[idx].version.IsValid())
		return false;
	strncpy(buf, m_pClientInfo[idx].version.GetFullString(), bufSize);
	buf[bufSize - 1] = '\0';
	return true;
}

bool CBugfixedServer::GetClientVersion(int idx, int &major, int &minor, int &patch)
{
	if (idx < 0 || idx > 32)
		return false;
	if (!m_pClientInfo[idx].version.IsValid())
		return false;
	m_pClientInfo[idx].version.GetVersion(major, minor, patch);
	return true;
}

bool CBugfixedServer::IsClientDirty(int idx)
{
	if (idx < 0 || idx > 32)
		return false;
	if (!m_pClientInfo[idx].version.IsValid())
		return false;
	return m_pClientInfo[idx].version.IsModified();
}

bool CBugfixedServer::GetClientVersionCommit(int idx, char *buf, int bufSize)
{
	if (idx < 0 || idx > 32)
		return false;
	if (!m_pClientInfo[idx].version.IsValid())
		return false;
	strncpy(buf, m_pClientInfo[idx].version.GetCommitString(), bufSize);
	buf[bufSize - 1] = '\0';
	return true;
}
