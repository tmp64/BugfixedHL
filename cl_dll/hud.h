/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  hud.h
//
// class CHud declaration
//
// CHud handles the message, calculation, and drawing the HUD
//

#ifndef CHUD_H
#define CHUD_H

#include <list>
#include <memory>
#include "wrect.h"
#include "cl_dll.h"
#include "ammo.h"
#include <eiface.h>
#include "CHudBase.h"

#ifdef USE_VGUI2
#include <SDK_Color.h>
#endif

bool ParseColor( char *string, RGBA &rgba );

#include "voice_status.h"

//-----------------------------------------------------
// Forward declarations
//-----------------------------------------------------
class TeamFortressViewport;
class CHudSpectator;
class CHudAmmo;
class CHudAmmoSecondary;
class CHudHealth;
class CHudGeiger;
class CHudTrain;
class CHudStatusBar;
class CHudSpeedometer;
class CHudDeathNotice;
class CHudMenu;
class CHudSayText;
class CHudBattery;
class CHudFlashlight;
class CHudTextMessage;
class CHudMessage;
class CHudTimer;
class CHudScores;
class CHudStatusIcons;
class CHudCrosshair;

#ifdef USE_VGUI2
class CHudScoreBoard;
class CHudTextVgui;
#endif

//-----------------------------------------------------
// Game info structures declaration
//-----------------------------------------------------
struct extra_player_info_t 
{
	short frags;
	short deaths;
	short playerclass;
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
};

struct team_info_t 
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ping;
	short packetloss;
	short ownteam;
	short players;
	int already_drawn;
	int scores_overriden;
	int teamnumber;
};

//-----------------------------------------------------
// Game info global variables
//-----------------------------------------------------
extern hud_player_info_t	g_PlayerInfoList[MAX_PLAYERS + 1];		// player info from the engine
extern extra_player_info_t	g_PlayerExtraInfo[MAX_PLAYERS + 1];		// additional player info sent directly to the client dll
extern team_info_t			g_TeamInfo[MAX_TEAMS + 1];
extern int					g_IsSpectator[MAX_PLAYERS + 1];
extern char					g_PlayerSteamId[MAX_PLAYERS + 1][MAX_STEAMID + 1];

//-----------------------------------------------------
// CharWidths
//-----------------------------------------------------
#define MAX_BASE_CHARS 255
struct CharWidths
{
	int indexes[MAX_BASE_CHARS];
	int widths[MAX_BASE_CHARS];
	CharWidths* next;
	CharWidths()
	{
		Reset();
		next = NULL;
	}
	void Reset()
	{
		memset(indexes, 0, HLARRAYSIZE(indexes));
		memset(widths, 0, HLARRAYSIZE(widths));
	}
};

//-----------------------------------------------------
// AG hud elements
//-----------------------------------------------------
#include "aghudglobal.h"
#include "aghudcountdown.h"
#include "aghudctf.h"
#include "aghudlocation.h"
#include "aghudlongjump.h"
#include "aghudnextmap.h"
#include "aghudplayerid.h"
#include "aghudsettings.h"
#include "aghudsuddendeath.h"
#include "aghudtimeout.h"
#include "aghudvote.h"

#define HUD_ELEM_INIT_FULL(type, var) var = std::shared_ptr<type>(new type()); var->m_isDeletable = true; var->Init();
#define HUD_ELEM_INIT(x) HUD_ELEM_INIT_FULL(CHud##x, m_##x)

class CHud
{
public:
	inline HLHSPRITE GetSprite(int index)
	{
		return (index < 0) ? 0 : m_rghSprites[index];
	}
	inline wrect_t& GetSpriteRect(int index)
	{
		return m_rgrcRects[index];
	}
	int GetSpriteIndex(const char *SpriteName);	// gets a sprite index, for use in the m_rghSprites[] array
	void AddSprite(client_sprite_t *p);

	//-----------------------------------------------------
	// HUD elements
	//-----------------------------------------------------
	std::shared_ptr<CHudAmmo>			m_Ammo	= nullptr;
	std::shared_ptr<CHudHealth>			m_Health = nullptr;
	std::shared_ptr<CHudSpectator>		m_Spectator = nullptr;
	std::shared_ptr<CHudGeiger>			m_Geiger = nullptr;
	std::shared_ptr<CHudBattery>		m_Battery = nullptr;
	std::shared_ptr<CHudTrain>			m_Train = nullptr;
	std::shared_ptr<CHudFlashlight>		m_Flash = nullptr;
	std::shared_ptr<CHudMessage>		m_Message = nullptr;
	std::shared_ptr<CHudStatusBar>		m_StatusBar = nullptr;
	std::shared_ptr<CHudSpeedometer>	m_Speedometer = nullptr;
	std::shared_ptr<CHudDeathNotice>	m_DeathNotice = nullptr;
	std::shared_ptr<CHudSayText>		m_SayText = nullptr;
	std::shared_ptr<CHudMenu>			m_Menu = nullptr;
	std::shared_ptr<CHudAmmoSecondary>	m_AmmoSecondary = nullptr;
	std::shared_ptr<CHudTextMessage>	m_TextMessage = nullptr;
	std::shared_ptr<CHudStatusIcons>	m_StatusIcons = nullptr;
	std::shared_ptr<CHudTimer>			m_Timer = nullptr;
	std::shared_ptr<CHudScores>			m_Scores = nullptr;
	std::shared_ptr<CHudCrosshair>		m_Crosshair = nullptr;
#ifdef USE_VGUI2
	std::shared_ptr<CHudScoreBoard>		m_ScoreBoard = nullptr;		// VGUI2 scoreboard
	std::shared_ptr<CHudTextVgui>		m_TextVgui = nullptr;
#endif

	//-----------------------------------------------------
	// AG HUD elements
	//-----------------------------------------------------
	AgHudGlobal			m_Global;
	AgHudCountdown		m_Countdown;
	AgHudCTF			m_CTF;
	AgHudLocation		m_Location;
	AgHudLongjump		m_Longjump;
	AgHudNextmap		m_Nextmap;
	AgHudPlayerId		m_PlayerId;
	AgHudSettings		m_Settings;
	AgHudSuddenDeath	m_SuddenDeath;
	AgHudTimeout		m_Timeout;
	AgHudVote			m_Vote;

	void Init(void);
	void Shutdown();
	void VidInit(void);
	void Think(void);
	int Redraw(float flTime, int intermission);
	int UpdateClientData(client_data_t *cdata, float time);
	void Frame(double time);

	CHud();
	~CHud();			// destructor, frees allocated memory

	//-----------------------------------------------------
	// User messages
	//-----------------------------------------------------
	int _cdecl MsgFunc_Damage(const char *pszName, int iSize, void *pbuf);
	int _cdecl MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf);
	int _cdecl MsgFunc_Logo(const char *pszName, int iSize, void *pbuf);
	int _cdecl MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf);
	void _cdecl MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf);
	void _cdecl MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf);
	int _cdecl MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf);
	int  _cdecl MsgFunc_Concuss(const char *pszName, int iSize, void *pbuf);

	// Screen information
	SCREENINFO	m_scrinfo;

	int	m_iWeaponBits;
	int	m_fPlayerDead;
	int m_iIntermission;

	// sprite indexes
	int m_HUD_number_0;

	void AddHudElem(CHudBase *elem);

	float GetSensitivity();

	HLHSPRITE	m_hsprCursor;
	float	m_flTime;	   // the current client time
	float	m_fOldTime;  // the time at which the HUD was last redrawn
	double	m_flTimeDelta; // the difference between flTime and fOldTime
	Vector	m_vecOrigin;
	Vector	m_vecAngles;
	int		m_iKeyBits;
	int		m_iHideHUDDisplay;
	int		m_iFOV;
	int		m_Teamplay;
	int		m_iRes;
#ifdef USE_VGUI2
	bool	m_bIsHtmlMotdEnabled = true;
#endif
	cvar_t	*m_pCvarBunnyHop;
	cvar_t	*m_pCvarStealMouse;
	cvar_t	*m_pCvarDraw;
	cvar_t	*m_pCvarDim;
	cvar_t	*m_pCvarShowNextmap;
	cvar_t	*m_pCvarShowLoss;
	cvar_t	*m_pCvarShowSteamId;
	cvar_t	*m_pCvarColorText;
	cvar_t	*m_pCvarRDynamicEntLight;
	cvar_t	*m_pCvarVersion;
	cvar_t	*m_pCvarSupports;
#ifdef USE_VGUI2
	cvar_t	*m_pCvarEnableHtmlMotd;
#endif
#ifdef USE_UPDATER
	cvar_t	*m_pCvarCheckUpdates;
#endif

	int m_iFontHeight;
	int DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b);
	int DrawHudNumber(int x, int y, int number, int r, int g, int b);
	int DrawHudNumberCentered(int x, int y, int number, int r, int g, int b);
	int DrawHudString(int x, int y, const char *szString, int r, int g, int b);
	int DrawHudStringReverse(int xpos, int ypos, const char *szString, int r, int g, int b);
	int DrawHudStringColorCodes(int x, int y, const char *string, int _r, int _g, int _b);
	int DrawHudStringReverseColorCodes(int x, int y, const char *string, int _r, int _g, int _b);
	int DrawHudNumberString(int xpos, int ypos, int iNumber, int r, int g, int b);
	int GetNumWidth(int iNumber, int iFlags);
	int GetHudCharWidth(int c);
	int CalculateCharWidth(int c);
	void GetHudColor(int hudPart, int value, int &r, int &g, int &b);
	float GetHudTransparency();
	void UpdateSupportsCvar();

	inline const char *GetServerName()
	{
		return m_szServerName;
	}

	inline void SetServerNamePtr(const char *ptr)	// Called in TeamFortressViewport
	{
		m_szServerName = ptr;
	}

#ifdef USE_VGUI2
	inline SDK_Color GetTeamColor(int team)
	{
		return m_pTeamColors[team % HLARRAYSIZE(m_pTeamColors)];
	}
#endif

private:
	std::list<CHudBase *>	m_HudList;
	HLHSPRITE				m_hsprLogo;
	int						m_iLogo;
	client_sprite_t			*m_pSpriteList;
	int						m_iSpriteCount;
	int						m_iSpriteCountAlloc;
	int						m_iSpriteCountAllRes;
	float					m_flMouseSensitivity;
	int						m_iConcussionEffect;
	CharWidths				m_CharWidths;

	cvar_t	*m_pCvarColor;
	cvar_t	*m_pCvarColor1;
	cvar_t	*m_pCvarColor2;
	cvar_t	*m_pCvarColor3;

	RGBA	m_hudColor;
	RGBA	m_hudColor1;
	RGBA	m_hudColor2;
	RGBA	m_hudColor3;

	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	HLHSPRITE *m_rghSprites;	/*[HUD_SPRITE_COUNT]*/			// the sprites loaded from hud.txt
	wrect_t *m_rgrcRects;	/*[HUD_SPRITE_COUNT]*/
	char *m_rgszSpriteNames; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

	struct cvar_s *default_fov;
	const char *m_szServerName = nullptr;	// Points to gViewPort->m_szServerName

#ifdef USE_VGUI2
	// Team colors for VGUI2
	SDK_Color m_pTeamColors[5] = {
		SDK_Color(216, 216, 216, 255),	// "Off" white (default)
		SDK_Color(125, 165, 210, 255),	// Blue
		SDK_Color(200, 90, 70, 255),	// Red
		SDK_Color(225, 205, 45, 255),	// Yellow
		SDK_Color(145, 215, 140, 255)	// Green
	};
#endif
#ifdef USE_UPDATER
	bool m_bUpdatesChecked = false;
#endif
};

extern CHud gHUD;
extern TeamFortressViewport *gViewPort;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;

#endif
