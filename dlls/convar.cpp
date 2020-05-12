#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "convar.h"

ConVar *ConVar::m_pFirst = nullptr;

ConVar::ConVar(const char *name, const char *defval, int flags)
{
	m_Cvar.name = (char *)name;
	m_Cvar.string = (char *)defval;
	m_Cvar.flags = flags;

	m_pNext = m_pFirst;
	m_pFirst = this;
}

float ConVar::Get()
{
	return m_Cvar.value;
}

const char *ConVar::GetString()
{
	return m_Cvar.string;
}

void ConVar::Set(const char *val)
{
	CVAR_SET_STRING(m_Cvar.name, val);
}

void ConVar::Set(float val)
{
	CVAR_SET_FLOAT(m_Cvar.name, val);
}

ConVar::operator bool()
{
	return m_Cvar.value != 0;
}

void ConVar::RegisterAll()
{
	for (ConVar *pCvar = m_pFirst; pCvar; pCvar = pCvar->m_pNext)
	{
		CVAR_REGISTER(&pCvar->m_Cvar);
	}
}
