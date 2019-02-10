#ifndef CBUGFIXEDSERVER_H
#define CBUGFIXEDSERVER_H

#include <CGameVersion.h>
#include "IBugfixedServer.h"
#include "eiface.h"
#include "progdefs.h"

#ifndef BUGFIXEDAPI_REQUEST_BASE
#define BUGFIXEDAPI_REQUEST_BASE 1
#endif

#ifdef SERVER_DLL
extern "C" DLLEXPORT void *AGHL_GetServerInterface(int version, int *srvVersion = nullptr);
#endif

class CBugfixedServer : public IBugfixedServer
{
public:
	CBugfixedServer(enginefuncs_t *engfuncs, globalvars_t *globalvars);
	void ClientConnect(edict_t *pEntity);
	void CvarValueCallback(const edict_t *pEnt, int requestID, const char *cvarName, const char *value);
	//void Think();

private:
	enum E_RequestType
	{
		REQUEST_VERSION = 0,
		REQUEST_SUPPORTS,
		REQUEST_COLOR
	};

	enum E_RequestId
	{
		REQUESTID_VERSION = BUGFIXEDAPI_REQUEST_BASE - 1,
		REQUESTID_SUPPORTS = BUGFIXEDAPI_REQUEST_BASE + 31,
		REQUESTID_COLOR = BUGFIXEDAPI_REQUEST_BASE + 63,
		REQUESTID_END = BUGFIXEDAPI_REQUEST_BASE + 95
	};

	struct bhl_client_info_t
	{
		CGameVersion version;
		E_ClientSupports supports;
		bool isColorEnabled = false;
	};

	/*struct cvar_request_t
	{
		int client_idx;
		int request;
		E_RequestType type;
	};*/

	enginefuncs_t *m_pEngFuncs = nullptr;
	globalvars_t *m_pGlobalVars = nullptr;
	bhl_client_info_t m_pClientInfo[33];
	
	void ResetPlayerData(int idx);
	//int strcopy(char *to, const char *from, int toSize);

public:
	//----------------------------------------------------------------------------------------------------
	// IBugfixedServer methods
	//----------------------------------------------------------------------------------------------------
	// Returns a pointer to g_pGameRules that contains a pointer to CGameRules
	virtual void *GetGameRulesPtr();

	virtual E_ClientSupports GetPlayerSupports(int idx);
	virtual bool IsColoredTextEnabled(int idx);

	// Client library version parsing
	virtual bool GetClientVersionString(int idx, char *buf, int bufSize);
	virtual bool GetClientVersion(int idx, int &major, int &minor, int &patch);
	virtual bool IsClientDirty(int idx);
	virtual bool GetClientVersionCommit(int idx, char *buf, int bufSize);
};

extern CBugfixedServer *gBugfixedServer;

#endif
