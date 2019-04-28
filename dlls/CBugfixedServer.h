#ifndef CBUGFIXEDSERVER_H
#define CBUGFIXEDSERVER_H

#include <CGameVersion.h>
#include <IBugfixedServer.h>
#include "eiface.h"
#include "progdefs.h"

#ifndef BUGFIXEDAPI_REQUEST_BASE
#define BUGFIXEDAPI_REQUEST_BASE 1
#endif

#ifdef SERVER_DLL
extern "C" DLLEXPORT void *AGHL_GetServerInterface(int version, int *srvVersion = nullptr);
#endif

/**
 * Implementation of IBugfixedServer
 * @see IBugfixedServer
 */
class CBugfixedServer : public bhl::IBugfixedServer
{
public:
	CBugfixedServer();
	void Init();
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
		bhl::E_ClientSupports supports;
		bool isColorEnabled = false;
	};

	bhl_client_info_t m_pClientInfo[MAX_PLAYERS + 1];
	CGameVersion m_ServerVersion;
	bhl::E_MotdType m_nMotdType = bhl::E_MotdType::All;
	
	void ResetPlayerData(int idx);
	//int strcopy(char *to, const char *from, int toSize);

public:
	//----------------------------------------------------------------------------------------------------
	// IBugfixedServer methods
	//----------------------------------------------------------------------------------------------------
	virtual void GetInterfaceVersion(int &major, int &minor);
	virtual CGameRules **GetGameRulesPtr();
	virtual const CGameVersion &GetServerVersion();
	virtual bhl::E_ClientSupports GetClientSupports(int idx);
	virtual bool GetColorSupport(int idx);
	virtual bool IsClientVersionValid(int idx);
	virtual bool GetClientVersion(int idx, CGameVersion &ver);
	virtual bool GetAutomaticMotd(bhl::E_MotdType type);
	virtual void SetAutomaticMotd(bhl::E_MotdType type, bool state);
	virtual void ShowMotdFromString(bhl::E_MotdType type, int idx, const char *str);
	virtual void ShowMotdFromFile(bhl::E_MotdType type, int idx, const char *file);
};

CBugfixedServer *serverapi();

#endif
