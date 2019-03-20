#ifndef CHUDTEXTVGUI_H
#define CHUDTEXTVGUI_H
#include "CHudBase.h"

class CHudTextVgui : public CHudBase
{
public:
	virtual void Init();
	virtual void Think();

private:
	typedef int (*DrawStringFunc_t)		(int x, int y, const char* string, int r, int g, int b);
	typedef int (*DrawCharacterFunc_t)	(int x, int y, int number, int r, int g, int b);

	DrawStringFunc_t m_pEngDrawString = nullptr;
	DrawCharacterFunc_t m_pEngDrawChar = nullptr;
	cvar_t *m_pCvarVguiText = nullptr;

	bool m_bIsUseVgui = false;
};

#endif