#include <cassert>
#include "extdll.h"
#include "util.h"
#include "edict.h"
#include "cbase.h"
#include "gamerules.h"
#include "CBugfixedServer.h"
#include <appversion.h>
#include "player.h"
#include <interface.h>

static CBugfixedServer g_staticserver;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CBugfixedServer, IBugfixedServer, IBUGFIXEDSERVER_NAME, g_staticserver);

CBugfixedServer *serverapi()
{
	return &g_staticserver;
}

//----------------------------------------------------------------------------------------------------
// CBugfixedServer
//----------------------------------------------------------------------------------------------------
CBugfixedServer::CBugfixedServer()
{
	m_ServerVersion = CGameVersion(APP_VERSION);
	assert(m_ServerVersion.IsValid());
}

void CBugfixedServer::Init()
{
}

void CBugfixedServer::ClientConnect(edict_t *pEntity)
{
	int idx = ENTINDEX(pEntity);
	
	if (idx < 1 || idx > 32)
	{
		UTIL_LogPrintf("CBugfixedServer::ClientConnect: player has invalid index (%d)\n", idx);
		return;
	}

	// Reset player data
	ResetPlayerData(idx);

	// Query vars
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pl = GetClassPtr<CBasePlayer>((CBasePlayer *)pev);

	// Check if bot (QueryClientCvarValue2 crashes the server on bots)
	bool isBot = false;
	const char *auth = nullptr;
	if ((pl->pev->flags & FL_FAKECLIENT) == FL_FAKECLIENT ||
		(auth = GETPLAYERAUTHID(pl->edict())) && strcmp(auth, "BOT") == 0)
	{
		isBot = true;
	}

	if (!isBot)	// Don't query bots
	{
		g_engfuncs.pfnQueryClientCvarValue2(pEntity, "aghl_version", REQUESTID_VERSION + idx);
		g_engfuncs.pfnQueryClientCvarValue2(pEntity, "aghl_supports", REQUESTID_SUPPORTS + idx);
		g_engfuncs.pfnQueryClientCvarValue2(pEntity, "hud_colortext", REQUESTID_COLOR + idx);
	}
}

void CBugfixedServer::CvarValueCallback(const edict_t *pEnt, int requestID, const char *cvarName, const char *value)
{
	int idx = ENTINDEX(pEnt);
	if (idx < 1 || idx > gpGlobals->maxClients)
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
		m_pClientInfo[idx].supports = (bhl::E_ClientSupports)iValue;
	}
	else if (requestID > REQUESTID_COLOR && requestID <= REQUESTID_END)
	{
		// hud_colortext
		int iValue = atoi(value);
		m_pClientInfo[idx].isColorEnabled = (iValue == 1);
	}
}

/*void CBugfixedServer::Think()
{
}*/

void CBugfixedServer::ResetPlayerData(int idx)
{
	m_pClientInfo[idx].isColorEnabled = false;
	m_pClientInfo[idx].supports = bhl::E_ClientSupports::None;
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
void CBugfixedServer::GetInterfaceVersion(int &major, int &minor)
{
	major = IBUGFIXEDSERVER_MAJOR;
	minor = IBUGFIXEDSERVER_MINOR;
}

CGameRules **CBugfixedServer::GetGameRulesPtr()
{
	return &g_pGameRules;
}

const CGameVersion &CBugfixedServer::GetServerVersion()
{
	return m_ServerVersion;
}

bhl::E_ClientSupports CBugfixedServer::GetClientSupports(int idx)
{
	if (idx < 1 || idx > gpGlobals->maxClients)
	{
		UTIL_LogPrintf("CBugfixedServer::GetClientSupports(): invalid player id (%d)\n", idx);
		return bhl::E_ClientSupports::None;
	}
	return m_pClientInfo[idx].supports;
}

bool CBugfixedServer::GetColorSupport(int idx)
{
	if (idx < 1 || idx > gpGlobals->maxClients)
	{
		UTIL_LogPrintf("CBugfixedServer::GetColorSupport(): invalid player id (%d)\n", idx);
		return false;
	}
	return m_pClientInfo[idx].isColorEnabled;
}

bool CBugfixedServer::IsClientVersionValid(int idx)
{
	if (idx < 1 || idx > gpGlobals->maxClients)
	{
		UTIL_LogPrintf("CBugfixedServer::IsClientVersionValid(): invalid player id (%d)\n", idx);
		return false;
	}
	return m_pClientInfo[idx].version.IsValid();
}

bool CBugfixedServer::GetClientVersion(int idx, CGameVersion &ver)
{
	if (idx < 1 || idx > gpGlobals->maxClients)
	{
		UTIL_LogPrintf("CBugfixedServer::IsClientVersionValid(): invalid player id (%d)\n", idx);
		return false;
	}
	if (!m_pClientInfo[idx].version.IsValid())
		return false;
	ver = m_pClientInfo[idx].version;
	return true;
}

bool CBugfixedServer::GetAutomaticMotd(bhl::E_MotdType type)
{
	return IsEnumFlagSet(m_nMotdType, type);
}

void CBugfixedServer::SetAutomaticMotd(bhl::E_MotdType type, bool state)
{
	if (state)
		SetEnumFlag(m_nMotdType, type);
	else
		ClearEnumFlag(m_nMotdType, type);
}

void CBugfixedServer::ShowMotdFromString(bhl::E_MotdType type, int idx, const char *cstr)
{
	if (idx < 1 || idx > gpGlobals->maxClients)
	{
		UTIL_LogPrintf("CBugfixedServer::ShowMotdFromString(): invalid player id (%d)\n", idx);
		return;
	}

	CHalfLifeMultiplay *pRules = dynamic_cast<CHalfLifeMultiplay *>(g_pGameRules);
	assert(pRules);
	if (!pRules)
	{
		UTIL_LogPrintf("CBugfixedServer::ShowMotdFromString(): g_pGameRules is not CHalfLifeMultiplay\n");
		return;
	}

	char *str = const_cast<char *>(cstr);
	edict_t *pEdict = INDEXENT(idx);

	if (type == bhl::E_MotdType::Plain)
	{
		pRules->SendMOTDToClient(pEdict, str);
	}
	else if (type == bhl::E_MotdType::Unicode)
	{
		pRules->SendUnicodeMOTDToClient(pEdict, str);
	}
	else if (type == bhl::E_MotdType::Html)
	{
		pRules->SendHtmlMOTDToClient(pEdict, str);
	}
}

void CBugfixedServer::ShowMotdFromFile(bhl::E_MotdType type, int idx, const char *file)
{
	if (idx < 1 || idx > gpGlobals->maxClients)
	{
		UTIL_LogPrintf("CBugfixedServer::ShowMotdFromFile(): invalid player id (%d)\n", idx);
		return;
	}

	CHalfLifeMultiplay *pRules = dynamic_cast<CHalfLifeMultiplay *>(g_pGameRules);
	assert(pRules);
	if (!pRules)
	{
		UTIL_LogPrintf("CBugfixedServer::ShowMotdFromFile(): g_pGameRules is not CHalfLifeMultiplay\n");
		return;
	}

	edict_t *pEdict = INDEXENT(idx);

	bool result = false;

	if (type == bhl::E_MotdType::Plain)
	{
		result = pRules->SendMOTDFileToClient(pEdict, file);
	}
	else if (type == bhl::E_MotdType::Unicode)
	{
		result = pRules->SendUnicodeMOTDFileToClient(pEdict, file);
	}
	else if (type == bhl::E_MotdType::Html)
	{
		result = pRules->SendHtmlMOTDFileToClient(pEdict, file);
	}

	if (!result)
		UTIL_LogPrintf("CBugfixedServer::ShowMotdFromFile(): file not found: %s\n", file);
}
