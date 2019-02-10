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
	m_pClientInfo[idx].version = CGameVersion();
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
