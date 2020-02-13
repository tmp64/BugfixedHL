#ifndef CHUDAMMO_H
#define CHUDAMMO_H

#include "CHudBase.h"
#include "ammo.h"

class CHudCrosshair;

class CHudAmmo : public CHudBase
{
public:
	void Init();
	void VidInit();
	void Draw(float flTime);
	void Think();
	void Reset();
	int DrawWList(float flTime);
	void UpdateCrosshair();
	int MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoPickup(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_WeapPickup(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ItemPickup(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf);

	int GetMaxClip(char* weaponname);
	int GetMaxSlot() { return m_iMaxSlot; }
	void SlotInput(int iSlot);
	void _cdecl UserCmd_Slot1();
	void _cdecl UserCmd_Slot2();
	void _cdecl UserCmd_Slot3();
	void _cdecl UserCmd_Slot4();
	void _cdecl UserCmd_Slot5();
	void _cdecl UserCmd_Slot6();
	void _cdecl UserCmd_Slot7();
	void _cdecl UserCmd_Slot8();
	void _cdecl UserCmd_Slot9();
	void _cdecl UserCmd_Slot10();
	void _cdecl UserCmd_Close();
	void _cdecl UserCmd_NextWeapon();
	void _cdecl UserCmd_PrevWeapon();

private:
	float	m_fFade;
	WEAPON	*m_pWeapon;
	int		m_fOnTarget;
	int		m_HUD_bucket0;
	int		m_HUD_selection;
	int		m_iMaxSlot;	// There are 5 (0-4) slots by default and they can extend to 6. This will be used to draw additional weapon bucket(s) on a hud.

	cvar_t	*m_pCvarHudWeapon;

	friend class CHudCrosshair;
};

#endif