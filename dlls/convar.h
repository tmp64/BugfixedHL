#ifndef CONVAR_H
#define CONVAR_H
#include <cvardef.h>

class ConVar
{
public:
	ConVar(const char *name, const char *defval, int flags = 0);
	ConVar(const ConVar &) = delete;
	ConVar &operator=(const ConVar &) = delete;

	float Get();
	const char *GetString();
	void Set(const char *val);
	void Set(float val);

	explicit operator bool();

	static void RegisterAll();

private:
	cvar_t m_Cvar;
	ConVar *m_pNext = nullptr;
	static ConVar *m_pFirst;
};

#endif
