#ifndef IBUGFIXEDAPI_H
#define IBUGFIXEDAPI_H

typedef void *(*GetServerInterfaceFunc)(int version, int *srvVersion);

enum E_ClientSupports : unsigned int
{
	AGHL_SUPPORTS_NONE = 0,
	AGHL_SUPPORTS_UNICODE_MOTD = (1 << 0),
	AGHL_SUPPORTS_HTML_MOTD = (1 << 1)
};

inline E_ClientSupports operator|(E_ClientSupports lhs, E_ClientSupports rhs)
{
	return (E_ClientSupports)((unsigned int)lhs | (unsigned int)rhs);
}

inline E_ClientSupports operator|=(E_ClientSupports &lhs, E_ClientSupports rhs)
{
	return (lhs = (E_ClientSupports)((unsigned int)lhs | (unsigned int)rhs));
}

class IBugfixedServer001
{
public:
	virtual ~IBugfixedServer001() {}

	// Returns a pointer to g_pGameRules that contains a pointer to CGameRules
	virtual void *GetGameRulesPtr() = 0;

	virtual E_ClientSupports GetPlayerSupports(int idx) = 0;
	virtual bool IsColoredTextEnabled(int idx) = 0;

	// Client library version parsing
	virtual bool GetClientVersionString(int idx, char *buf, int bufSize) = 0;
	virtual bool GetClientVersion(int idx, int &major, int &minor, int &patch) = 0;
	virtual bool IsClientDirty(int idx) = 0;
	virtual bool GetClientVersionCommit(int idx, char *buf, int bufSize) = 0;
};

typedef IBugfixedServer001 IBugfixedServer;
#define BUGFIXEDSERVERIFACE_VERSION 001

#endif
