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
// hud.cpp
//
// implementation of CHud class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "hud_servers.h"
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "GameStudioModelRenderer.h"

#include "demo.h"
#include "demo_api.h"
#include "vgui_ScorePanel.h"
#include "appversion.h"
#include "memory.h"
#include "bhlcfg.h"

#include <ClientSupportsFlags.h>

//-----------------------------------------------------
// HUD elements
//-----------------------------------------------------
#include "CHudSpectator.h"
#include "CHudAmmo.h"
#include "CHudAmmoSecondary.h"
#include "CHudHealth.h"
#include "CHudGeiger.h"
#include "CHudTrain.h"
#include "CHudStatusBar.h"
#include "CHudSpeedometer.h"
#include "CHudDeathNotice.h"
#include "CHudMenu.h"
#include "CHudSayText.h"
#include "CHudBattery.h"
#include "CHudFlashlight.h"
#include "CHudTextMessage.h"
#include "CHudMessage.h"
#include "CHudTimer.h"
#include "CHudScores.h"
#include "CHudStatusIcons.h"
#include "CHudCrosshair.h"

#ifdef USE_VGUI2
#include "vgui2/CBaseViewport.h"
#include "vgui2/CHudScoreBoard.h"
#include "vgui2/CHudTextVgui.h"
#include "vgui2/CHudChat.h"
#include "clientsteamcontext.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/TextImage.h>
#include "vgui2/gameui/options/colorpicker/CTextureManager.h"
#endif

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

#ifdef USE_UPDATER
#include <CGameUpdater.h>
#include "CUpdateNotification.h"
#endif

float g_ColorBlue[3]	= { 0.6f, 0.8f, 1.0f };
float g_ColorRed[3]		= { 1.0f, 0.25f, 0.25f };
float g_ColorGreen[3]	= { 0.6f, 1.0f, 0.6f };
float g_ColorYellow[3]	= { 1.0f, 0.7f, 0.0f };
float g_ColorGrey[3]	= { 0.8f, 0.8f, 0.8f };

float *GetClientTeamColor(int clientIndex)
{
	switch (g_PlayerExtraInfo[clientIndex].teamnumber)
	{
		case 0: return NULL;
		case 1: return g_ColorBlue;
		case 2: return g_ColorRed;
		case 3: return g_ColorYellow;
		case 4: return g_ColorGreen;

		default: return g_ColorGrey;
	}
}

int g_iColorsCodes[10][3] = 
{
	{ 0xFF, 0xAA, 0x00 },	// ^0 orange / reset
	{ 0xFF, 0x00, 0x00 },	// ^1 red
	{ 0x00, 0xFF, 0x00 },	// ^2 green
	{ 0xFF, 0xFF, 0x00 },	// ^3 yellow
	{ 0x00, 0x00, 0xFF },	// ^4 blue
	{ 0x00, 0xFF, 0xFF },	// ^5 cyan
	{ 0xFF, 0x00, 0xFF },	// ^6 magenta
	{ 0x88, 0x88, 0x88 },	// ^7 grey
	{ 0xFF, 0xFF, 0xFF },	// ^8 white
							// ^9 orange / reset
};

class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex >= 0 && entindex < sizeof(g_PlayerExtraInfo)/sizeof(g_PlayerExtraInfo[0]) )
		{
			int iTeam = g_PlayerExtraInfo[entindex].teamnumber;

			if ( iTeam < 0 )
			{
				iTeam = 0;
			}

			iTeam = iTeam % iNumberOfTeamColors;

			color[0] = iTeamColors[iTeam][0];
			color[1] = iTeamColors[iTeam][1];
			color[2] = iTeamColors[iTeam][2];
		}
	}

	virtual void UpdateCursorState()
	{
		gViewPort->UpdateCursorState();
	}

	virtual int	GetAckIconHeight()
	{
		return ScreenHeight - gHUD.m_iFontHeight*3 - 6;
	}

	virtual bool			CanShowSpeakerLabels()
	{
		if( gViewPort && gViewPort->m_pScoreBoard )
			return !gViewPort->m_pScoreBoard->isVisible();
		else
			return false;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;


extern client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

extern cvar_t *sensitivity;
cvar_t *cl_lw = NULL;

void ShutdownInput (void);

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_Logo(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Logo(pszName, iSize, pbuf );
}

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ResetHUD(pszName, iSize, pbuf );
}

int __MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_InitHUD( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_ViewMode( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SetFOV( pszName, iSize, pbuf );
}

int __MsgFunc_Concuss(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Concuss( pszName, iSize, pbuf );
}

int __MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_GameMode( pszName, iSize, pbuf );
}

void __CmdFunc_About(void)
{
	ConPrintf("Bugfixed and improved Half-Life client\n");
	ConPrintf("Version: " APP_VERSION "\n");
	ConPrintf("Build options: ");
#ifdef USE_VGUI2
	ConPrintf("VGUI2 support    ");
#ifdef VGUI2_BUILD_4554
	ConPrintf("game build 4554    ");
#endif
#endif
#ifdef USE_UPDATER
	ConPrintf("auto-updater    ");
#endif
#ifdef _DEBUG
	ConPrintf("debug build    ");
#endif
	ConPrintf("\n");
	ConPrintf("Github: https://github.com/tmp64/BugfixedHL\n");
	ConPrintf("Discussion forum: http://aghl.ru/forum/viewtopic.php?f=36&t=686\n");
}

// TFFree Command Menu
void __CmdFunc_OpenCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
}

// TFC "special" command
void __CmdFunc_InputPlayerSpecial(void)
{
	if ( gViewPort )
	{
		gViewPort->InputPlayerSpecial();
	}
}

void __CmdFunc_CloseCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->InputSignalHideCommandMenu();
	}
}

void __CmdFunc_ForceCloseCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
}

void __CmdFunc_ToggleServerBrowser( void )
{
	if ( gViewPort )
	{
		gViewPort->ToggleServerBrowser();
	}
}

void __CmdFunc_ForceModel(void)
{
	g_StudioRenderer.ForceModelCommand();
}

void __CmdFunc_ForceColors(void)
{
	g_StudioRenderer.ForceColorsCommand();
}

void __CmdFunc_CustomTimer(void)
{
#ifdef _WIN32
	gHUD.m_Timer->CustomTimerCommand();
#endif
}

void __CmdFunc_ToggleCvar(void)
{
	int argc = gEngfuncs.Cmd_Argc();
	if (argc <= 1 || argc == 3)
	{
		gEngfuncs.Con_Printf("usage: _toggle <cvar> or _toggle <cvar> <val1> <val2> [val3] ... [valN]\n");
		return;
	}

	cvar_t *cvar = CVAR_GET_POINTER(gEngfuncs.Cmd_Argv(1));

	if (!cvar)
	{
		gEngfuncs.Con_Printf("_toggle failed: cvar '%s' not found.\n", gEngfuncs.Cmd_Argv(1));
		return;
	}

	char cmd[256];

	if (argc == 2)
	{
		sprintf(cmd, "%s %d", gEngfuncs.Cmd_Argv(1), cvar->value ? 0 : 1);
		ClientCmd(cmd);
		return;
	} else
	{
		for (int i = 2; i < argc; i++)
		{
			if (!strcmp(cvar->string, gEngfuncs.Cmd_Argv(i)))
			{
				if (i + 1 < argc) // switch cvar value to the next one
				{
					sprintf(cmd, "%s \"%s\"", gEngfuncs.Cmd_Argv(1), gEngfuncs.Cmd_Argv(i + 1));
					ClientCmd(cmd);
					return;
				} else // if we have get to the top of _toggle values list, then start from the beginning
				{
					sprintf(cmd, "%s \"%s\"", gEngfuncs.Cmd_Argv(1), gEngfuncs.Cmd_Argv(2));
					ClientCmd(cmd);
					return;
				}
			}
		}

		// if cvar value isn't equal to any values from _toggle, then set it to the first value of _toggle...
		sprintf(cmd, "%s \"%s\"", gEngfuncs.Cmd_Argv(1), gEngfuncs.Cmd_Argv(2));
		ClientCmd(cmd);
		return;
	}
}

// TFFree Command Menu Message Handlers
int __MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ValClass( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamNames( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Feign(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Feign( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Detpack( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_MOTD( pszName, iSize, pbuf );
	return 0;
}

#if defined(USE_VGUI2) && !defined(VGUI2_BUILD_4554)
int __MsgFunc_HtmlMOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_HtmlMOTD(pszName, iSize, pbuf);
	return 0;
}
#endif

int __MsgFunc_BuildSt(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_BuildSt( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_RandomPC(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_RandomPC( pszName, iSize, pbuf );
	return 0;
}
 
int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ScoreInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamScore( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Spectator(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_AllowSpec(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec( pszName, iSize, pbuf );
	return 0;
}

#ifdef USE_UPDATER
void __CmdFunc_Updater_CheckUpdates()
{
	ConPrintf("Checking for updates...\n");
	gGameUpdater->CheckForUpdates();
}

void __CmdFunc_Updater_PrintChangelog()
{
	std::string changelog = gGameUpdater->GetChangeLog();
	if (changelog.size() == 0)
	{
		ConPrintf("Changelog not loaded yet.\n");
		return;
	}

	// Con_Printf has an internal limit of 4095 characters.
	// Changelog may be longer than that so we split it up in chunks of 512 chars.
	gEngfuncs.Con_Printf("Update changelog:\n");
	size_t constexpr LOG_CHUNK_SIZE = 512;
	for (size_t i = 0; LOG_CHUNK_SIZE * i <= changelog.length(); i++)
	{
		std::string substr = changelog.substr(LOG_CHUNK_SIZE * i, LOG_CHUNK_SIZE);
		gEngfuncs.Con_Printf("%s", substr.c_str());
	}
	gEngfuncs.Con_Printf("\n");
}
#endif

// This is called every time the DLL is loaded
void CHud :: Init( void )
{
	// Init should never be called more than once
	assert(m_HudList.size() == 0);

	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( GameMode );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( ViewMode );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( Concuss );

	HOOK_COMMAND( "about", About );

	// TFFree CommandMenu
	HOOK_COMMAND( "+commandmenu", OpenCommandMenu );
	HOOK_COMMAND( "-commandmenu", CloseCommandMenu );
	HOOK_COMMAND( "ForceCloseCommandMenu", ForceCloseCommandMenu );
	HOOK_COMMAND( "special", InputPlayerSpecial );
	HOOK_COMMAND( "togglebrowser", ToggleServerBrowser );
	HOOK_COMMAND( "forcemodel", ForceModel );
	HOOK_COMMAND( "forcecolors", ForceColors );
	HOOK_COMMAND( "customtimer", CustomTimer );
	HOOK_COMMAND( "_toggle", ToggleCvar );

	HOOK_MESSAGE( ValClass );
	HOOK_MESSAGE( TeamNames );
	HOOK_MESSAGE( Feign );
	HOOK_MESSAGE( Detpack );
	HOOK_MESSAGE( MOTD );
#if defined(USE_VGUI2) && !defined(VGUI2_BUILD_4554)
	HOOK_MESSAGE( HtmlMOTD );
#endif
	HOOK_MESSAGE( BuildSt );
	HOOK_MESSAGE( RandomPC );
	HOOK_MESSAGE( ServerName );
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( TeamInfo );

	HOOK_MESSAGE( Spectator );
	HOOK_MESSAGE( AllowSpec );

	// VGUI Menus
	HOOK_MESSAGE( VGUIMenu );

	CVAR_CREATE( "hud_classautokill", "1", FCVAR_ARCHIVE );		// controls whether or not to suicide immediately on TF class switch
	CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", 0 );
	CVAR_CREATE( "cl_forceenemymodels", "", FCVAR_BHL_ARCHIVE );
	CVAR_CREATE( "cl_forceenemycolors", "", FCVAR_BHL_ARCHIVE );
	CVAR_CREATE( "cl_forceteammatesmodel", "", FCVAR_BHL_ARCHIVE );
	CVAR_CREATE( "cl_forceteammatescolors", "", FCVAR_BHL_ARCHIVE );
	m_pCvarBunnyHop = CVAR_CREATE( "cl_bunnyhop", "1", 0 );		// controls client-side bunnyhop enabling
	CVAR_CREATE( "cl_autowepswitch", "1", FCVAR_BHL_ARCHIVE | FCVAR_USERINFO );		// controls autoswitching to best weapon on pickup
	m_pCvarHideCorpses = CVAR_CREATE("cl_hidecorpses", "0", FCVAR_ARCHIVE);
	m_pCvarFixStandingCorpses = CVAR_CREATE("cl_fix_standing_corpses", "0", FCVAR_ARCHIVE);

	default_fov = CVAR_CREATE( "default_fov", "90", FCVAR_BHL_ARCHIVE);
	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	m_pCvarDim = CVAR_CREATE( "hud_dim", "1", FCVAR_BHL_ARCHIVE );
	m_pCvarColor = CVAR_CREATE( "hud_color", "255 160 0", FCVAR_BHL_ARCHIVE );
	m_pCvarColor1 = CVAR_CREATE( "hud_color1", "0 255 0", FCVAR_BHL_ARCHIVE );
	m_pCvarColor2 = CVAR_CREATE( "hud_color2", "255 160 0", FCVAR_BHL_ARCHIVE );
	m_pCvarColor3 = CVAR_CREATE( "hud_color3", "255 96 0", FCVAR_BHL_ARCHIVE );
	m_pCvarShowNextmap = CVAR_CREATE( "hud_shownextmapinscore", "1", FCVAR_BHL_ARCHIVE );	// controls whether or not to show nextmap in scoreboard table
	m_pCvarShowLoss = CVAR_CREATE( "hud_showlossinscore", "1", FCVAR_BHL_ARCHIVE );	// controls whether or not to show loss in scoreboard table
	m_pCvarShowSteamId = CVAR_CREATE( "hud_showsteamidinscore", "1", FCVAR_BHL_ARCHIVE );	// controls whether or not to show SteamId in scoreboard table
	m_pCvarColorText = CVAR_CREATE( "hud_colortext", "1", FCVAR_BHL_ARCHIVE );
	m_pCvarRDynamicEntLight = CVAR_CREATE("r_dynamic_ent_light", "1", FCVAR_BHL_ARCHIVE);
	m_pCvarVersion = CVAR_CREATE("aghl_version", APP_VERSION, 0);
	m_pCvarSupports = CVAR_CREATE("aghl_supports", "0", 0);
#if defined(USE_VGUI2) && !defined(VGUI2_BUILD_4554)
	m_pCvarEnableHtmlMotd = CVAR_CREATE("cl_enable_html_motd", "1", FCVAR_BHL_ARCHIVE);
#endif
#ifdef USE_UPDATER
	m_pCvarCheckUpdates = CVAR_CREATE("cl_check_for_updates", "1", FCVAR_BHL_ARCHIVE);
#endif
	UpdateSupportsCvar();

	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );

	m_iLogo = 0;
	m_iFOV = 0;
	m_pSpriteList = NULL;

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;

	m_hudColor.Set(255, 160, 0);
	m_hudColor1.Set(0, 255, 0);
	m_hudColor2.Set(255, 160, 0);
	m_hudColor3.Set(255, 96, 0);


	// Create all HUD elments
	m_Ammo = new CHudAmmo();
	m_Health = new CHudHealth();
	m_SayText = new CHudSayText();
	m_Spectator = new CHudSpectator();
	m_Geiger = new CHudGeiger();
	m_Train = new CHudTrain();
	m_Battery = new CHudBattery();
	m_Flash = new CHudFlashlight();
	m_Message = new CHudMessage();
	m_StatusBar = new CHudStatusBar();
	m_Speedometer = new CHudSpeedometer();
	m_DeathNotice = new CHudDeathNotice();
	m_AmmoSecondary = new CHudAmmoSecondary();
	m_TextMessage = new CHudTextMessage();
	m_StatusIcons = new CHudStatusIcons();
	m_Timer = new CHudTimer();
	m_Scores = new CHudScores();
	m_Crosshair = new CHudCrosshair();
	m_Menu = new CHudMenu();
#ifdef USE_VGUI2
	m_ScoreBoard = new CHudScoreBoard();;
	m_TextVgui = new CHudTextVgui();;
	m_Chat = new CHudChat();
#endif

	// AG HUD - enabled on all clients. Should work with miniAG as well (not confirmed)
	m_Global = new AgHudGlobal();
	m_Countdown = new AgHudCountdown();
	m_CTF = new AgHudCTF();
	m_Location = new AgHudLocation();
	m_Longjump = new AgHudLongjump();
	m_Nextmap = new AgHudNextmap();
	m_PlayerId = new AgHudPlayerId();
	m_Settings = new AgHudSettings();
	m_SuddenDeath = new AgHudSuddenDeath();
	m_Timeout = new AgHudTimeout();
	m_Vote = new AgHudVote();

	CreateClientVoiceMgr();

	// Init all HUD elements
	for (CHudBase *i : m_HudList)
		i->Init();

	// Init other stuff
	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel**)&gViewPort);
	ServersInit();

#ifdef USE_VGUI2
	vgui2::TextImage::SetColorsArrayPointer(&g_iColorsCodes);
	colorpicker::gTexMgr.Init();
	g_pViewport->ReloadScheme();
#endif

	MsgFunc_ResetHUD(0, 0, NULL);

#ifdef USE_UPDATER
	gGameUpdater = new CGameUpdater();
	gUpdateNotif = new CUpdateNotification();
	HOOK_COMMAND("update_check", Updater_CheckUpdates);
	HOOK_COMMAND("update_changelog", Updater_PrintChangelog);
#endif

	bhlcfg::Init();
}

void CHud :: Shutdown()
{
#ifdef USE_UPDATER
	if (gUpdateNotif)
	{
		delete gUpdateNotif;
		gUpdateNotif = nullptr;
	}

	if (gGameUpdater)
	{
		delete gGameUpdater;
		gGameUpdater = nullptr;
	}
#endif

#ifdef USE_VGUI2
	colorpicker::gTexMgr.Shutdown();
#endif

	bhlcfg::Shutdown();
}

void CHud::Frame(double time)
{
	while (m_NextFrameQueue.size())
	{
		auto &i = m_NextFrameQueue.front();
		i();
		m_NextFrameQueue.pop();
	}

	// Check m_rawinput cvar
#ifdef _WIN32
	static cvar_t *rawInput = gEngfuncs.pfnGetCvarPointer("m_rawinput");
	if (rawInput && rawInput->value)
	{
		ConPrintf(RGBA::ConColor::Cyan, "m_rawinput 1 is not supported. Setting m_input 2 (DirectInput) instead.\n");
		gEngfuncs.pfnClientCmd("m_rawinput 0");
		gEngfuncs.pfnClientCmd("m_input 2");
	}
#endif

#ifdef USE_UPDATER
	if (!m_bUpdatesChecked && time >= 0.05 )		// Wait for config.cfg to be executed
	{
		m_bUpdatesChecked = true;
		if (m_pCvarCheckUpdates->value)
		{
			gUpdateNotif->SetActive(true);
			gGameUpdater->CheckForUpdates();
		}
		else
			gUpdateNotif->SetActive(false);
	}
#endif

#ifdef USE_VGUI2
	// Run VGUI2 animations
	static double flTimeSum = 0;
	vgui2::GetAnimationController()->UpdateAnimations(flTimeSum);
	flTimeSum += time;

	colorpicker::gTexMgr.RunFrame();
#endif
}

// CHud constructor
CHud::CHud() : m_iSpriteCount(0)
{
	
}

// CHud destructor
// frees allocated dynamic memory
CHud :: ~CHud()
{
	delete [] m_rghSprites;
	delete [] m_rgrcRects;
	delete [] m_rgszSpriteNames;

	CharWidths* cur = m_CharWidths.next;
	CharWidths* next;
	while (cur != NULL)
	{
		next = cur->next;
		delete cur;
		cur = next;
	}

	ServersShutdown();

#ifdef USE_UPDATER
	if (gUpdateNotif)
	{
		delete gUpdateNotif;
		gUpdateNotif = nullptr;
	}

	if (gGameUpdater)
	{
		delete gGameUpdater;
		gGameUpdater = nullptr;
	}
#endif

	// Delete all HUD elements
	while (m_HudList.size() > 0)
	{
		CHudBase * i = *m_HudList.rbegin();
#ifdef USE_VGUI2
		// Do not delete VGUI2 HUD elements because CHud::Shutdown() and
		// CHud::~CHud() are called after VGUI2 subsystem is shutdown
		// Deatructors of VGUI2 panels call VGUI2 interfaces which are invalid
		if (dynamic_cast<vgui2::Panel *>(i))
		{
			i->EraseFromHudList();
		}
		else
#endif
		{
			delete i;
		}
	}
}

// GetSpriteIndex()
// searches through the sprite list loaded from hud.txt for a name matching SpriteName
// returns an index into the gHUD.m_rghSprites[] array
// returns 0 if sprite not found
int CHud :: GetSpriteIndex( const char *SpriteName )
{
	// look through the loaded sprite name list for SpriteName
	for ( int i = 0; i < m_iSpriteCount; i++ )
	{
		if ( strncmp( SpriteName, m_rgszSpriteNames + (i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH ) == 0 )
			return i;
	}

	return -1; // invalid sprite
}

void CHud :: VidInit( void )
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	CharWidths* cur = &m_CharWidths;
	while (cur != NULL)
	{
		cur->Reset();
		cur = cur->next;
	}

	// ----------
	// Load Sprites
	// ---------
	//	m_hsprFont = LoadSprite("sprites/%d_font.spr");
	m_hsprLogo = 0;
	m_hsprCursor = 0;

	if (ScreenWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	client_sprite_t *p;
	m_iSpriteCount = 0;
	// Only load hud.txt once
	if ( !m_pSpriteList )
	{
		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = SPR_GetList("sprites/hud.txt", &m_iSpriteCountAllRes);

		if (m_pSpriteList)
		{
			// count the number of sprites of the appropriate res
			m_iSpriteCountAlloc = 0;
			p = m_pSpriteList;
			for (int i = 0; i < m_iSpriteCountAllRes; i++)
			{
				if (p->iRes == m_iRes)
					m_iSpriteCountAlloc++;
				p++;
			}

			// allocated memory for sprite handle arrays
			m_iSpriteCountAlloc += RESERVE_SPRITES_FOR_WEAPONS;
			m_rghSprites = new HLHSPRITE[m_iSpriteCountAlloc];
			m_rgrcRects = new wrect_t[m_iSpriteCountAlloc];
			m_rgszSpriteNames = new char[m_iSpriteCountAlloc * MAX_SPRITE_NAME_LENGTH];
		}
	}
	// Load srpites on every VidInit
	// so make sure all the sprites have been loaded (may be we've gone through a transition, or loaded a save game)
	if (m_pSpriteList)
	{
		p = m_pSpriteList;
		for (int i = 0; i < m_iSpriteCountAllRes; i++)
		{
			if (p->iRes == m_iRes)
				AddSprite(p);
			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );

	m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;

	// VidInit all HUD elements
	for (CHudBase *i : m_HudList)
		i->VidInit();
}

void CHud::AddSprite(client_sprite_t *p)
{
	// Realloc arrays if needed
	if (m_iSpriteCount >= m_iSpriteCountAlloc)
	{
		int oldAllocCount = m_iSpriteCountAlloc;
		int newAllocCount = m_iSpriteCountAlloc + RESERVE_SPRITES_FOR_WEAPONS;
		m_iSpriteCountAlloc = newAllocCount;
		HLHSPRITE *new_rghSprites = new HLHSPRITE[newAllocCount];
		wrect_t *new_rgrcRects = new wrect_t[newAllocCount];
		char *new_rgszSpriteNames = new char[newAllocCount * MAX_SPRITE_NAME_LENGTH];
		memcpy(new_rghSprites, m_rghSprites, sizeof(HLHSPRITE) * oldAllocCount);
		memcpy(new_rgrcRects, m_rgrcRects, sizeof(wrect_t*) * oldAllocCount);
		memcpy(new_rgszSpriteNames, m_rgszSpriteNames, oldAllocCount * MAX_SPRITE_NAME_LENGTH);
		delete[] m_rghSprites;
		delete[] m_rgrcRects;
		delete[] m_rgszSpriteNames;
		m_rghSprites = new_rghSprites;
		m_rgrcRects = new_rgrcRects;
		m_rgszSpriteNames = new_rgszSpriteNames;
	}

	// Search for existing sprite
	int i = 0;
	for (i = 0; i < m_iSpriteCount; i++)
	{
		if (!_stricmp(&m_rgszSpriteNames[i * MAX_SPRITE_NAME_LENGTH], p->szName))
			return;
	}

	char sz[256];
	sprintf(sz, "sprites/%s.spr", p->szSprite);
	m_rghSprites[m_iSpriteCount] = SPR_Load(sz);
	m_rgrcRects[m_iSpriteCount] = p->rc;
	strncpy(&m_rgszSpriteNames[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH);
	m_rgszSpriteNames[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH + MAX_SPRITE_NAME_LENGTH - 1] = 0;
	m_iSpriteCount++;
}

int CHud::MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

float g_lastFOV = 0.0;

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out)
{
	int len, start, end;

	len = strlen( in );
	
	// scan backward for '.'
	end = len - 1;
	while ( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;
	
	if ( in[end] != '.' )		// no '.', copy to end
		end = len-1;
	else 
		end--;					// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if ( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else 
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );
	// Terminate it
	out[len] = 0;
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame( const char *game )
{
	const char *gamedir;
	char gd[ 1024 ];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if ( gamedir && gamedir[0] )
	{
		COM_FileBase( gamedir, gd );
		if ( !_stricmp( gd, game ) )
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
float HUD_GetFOV( void )
{
	if ( gEngfuncs.pDemoAPI->IsRecording() )
	{
		// Write it
		int i = 0;
		unsigned char buf[ 100 ];

		// Active
		*( float * )&buf[ i ] = g_lastFOV;
		i += sizeof( float );

		Demo_WriteBuffer( TYPE_ZOOM, i, buf );
	}

	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}

int CHud::MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int newfov = READ_BYTE();
	int def_fov = CVAR_GET_FLOAT( "default_fov" );

	//Weapon prediction already takes care of changing the fog. ( g_lastFOV ).
	if ( cl_lw && cl_lw->value )
		return 1;

	g_lastFOV = newfov;

	if ( newfov == 0 )
	{
		m_iFOV = def_fov;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if ( m_iFOV == def_fov )
	{  
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{  
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	return 1;
}

float CHud::GetSensitivity( void )
{
	return m_flMouseSensitivity;
}

bool ParseColor( char *string, RGBA &rgba )
{
	unsigned char r,g,b;
	char *value = string;
	while (*value == ' ') value++;
	if (*value < '0' || *value > '9') return false;
	r = atoi(value);
	value = strchr(value, ' ');
	if (value == NULL) return false;
	while (*value == ' ') value++;
	if (*value < '0' || *value > '9') return false;
	g = atoi(value);
	value = strchr(value, ' ');
	if (value == NULL) return false;
	while (*value == ' ') value++;
	if (*value < '0' || *value > '9') return false;
	b = atoi(value);
	rgba.Set(r, g, b);
	return true;
}

// hudPart: 0 - common hud, 1 - health points, 2 - armor points
void CHud::GetHudColor( int hudPart, int value, int &r, int &g, int &b )
{
	RGBA *c;
	if (hudPart == 0) { ParseColor(m_pCvarColor->string, m_hudColor); c = &m_hudColor; }
	else if (value >= 90) { ParseColor(m_pCvarColor1->string, m_hudColor1); c = &m_hudColor1; }
	else if (value >= 50 && value <= 90) { ParseColor(m_pCvarColor2->string, m_hudColor2); c = &m_hudColor2; }
	else if ((value > 25 && value < 50) || hudPart == 2) { ParseColor(m_pCvarColor3->string, m_hudColor3); c = &m_hudColor3; }
	else { r = 255; g = 0; b = 0; return; }	// UnpackRGB(r, g, b, RGB_REDISH);
	r = c->r;
	g = c->g;
	b = c->b;
}

void CHud::GetHudAmmoColor(int value, int maxvalue, int &r, int &g, int &b)
{
	RGBA *c;
	if (maxvalue == -1 || maxvalue == 0) // if you are using custom weapons, then default colors are going to be used....
	{
		ParseColor(m_pCvarColor->string, m_hudColor); c = &m_hudColor;
	}
	else if ((value * 100) / maxvalue > 90)
	{
		ParseColor(m_pCvarColor1->string, m_hudColor1); c = &m_hudColor1;
	}
	else if ((value * 100) / maxvalue > 50)
	{
		ParseColor(m_pCvarColor2->string, m_hudColor2); c = &m_hudColor2;
	}
	else if ((value * 100) / maxvalue > 20)
	{
		ParseColor(m_pCvarColor3->string, m_hudColor3); c = &m_hudColor3;
	}
	else
	{
		r = 255;
		g = 0;
		b = 0;
		return; 
	}
	r = c->r;
	g = c->g;
	b = c->b;
}

float CHud::GetHudTransparency()
{
	float hud_draw = m_pCvarDraw->value;

	if (hud_draw > 1) hud_draw = 1;
	if (hud_draw < 0) hud_draw = 0;

	return hud_draw;
}

void CHud::UpdateSupportsCvar()
{
	bhl::E_ClientSupports supports = bhl::E_ClientSupports::None;
#ifdef USE_VGUI2
	//supports |= bhl::AGHL_SUPPORTS_UNICODE_MOTD;
	SetEnumFlag(supports, bhl::E_ClientSupports::UnicodeMotd);
#ifndef VGUI2_BUILD_4554
	if (m_bIsHtmlMotdEnabled)
		SetEnumFlag(supports, bhl::E_ClientSupports::HtmlMotd);
#endif
#endif

	char buf[32];
	snprintf(buf, sizeof(buf), "aghl_supports %u", static_cast<unsigned int>(supports));
	ClientCmd(buf);
}

E_ColorCodeMode CHud::GetColorCodeMode()
{
	if (gHUD.m_pCvarColorText->value)
	{
		if (gHUD.m_pCvarColorText->value == 2)
			return COLOR_CODES_REMOVE;
		else
			return COLOR_CODES_ON;
	}
	else
		return COLOR_CODES_OFF;
}

void CHud::CallOnNextFrame(std::function<void()> f)
{
	assert(f);
	m_NextFrameQueue.push(f);
}

void GetConsoleStringSize(const char *string, int *width, int *height)
{
	if (gHUD.m_pCvarColorText->value == 0)
		gEngfuncs.pfnDrawConsoleStringLen(string, width, height);
	else
		gEngfuncs.pfnDrawConsoleStringLen(RemoveColorCodes((char*)string), width, height);
}

int DrawConsoleString(int x, int y, const char *string, float *color)
{
	// How colorcodes work in DrawConsoleString
	// 1) If float *color is set (e.g. team color), it is used, colorcodes are ignored.
	// 2) Otherwise, colorcodes ^0 and ^9 reset color to con_color.

	if (!string || !*string)
		return x;

	if (color != NULL)
		gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);
	else
		gEngfuncs.pfnDrawConsoleString(x, y, " ");	// Reset color to con_color

	if (gHUD.m_pCvarColorText->value == 0)
		return gEngfuncs.pfnDrawConsoleString(x, y, (char*)string);

	char *c1 = (char*)string;
	char *c2 = (char*)string;
	float r, g, b;
	int colorIndex;
	while (true)
	{
		// Search for next color code
		colorIndex = -1;
		while(*c2 && *(c2 + 1) && !(*c2 == '^' && *(c2 + 1) >= '0' && *(c2 + 1) <= '9'))
			c2++;
		if (*c2 == '^' && *(c2 + 1) >= '0' && *(c2 + 1) <= '9')
		{
			colorIndex = *(c2 + 1) - '0';
			*c2 = 0;
		}
		// Draw current string
		x = gEngfuncs.pfnDrawConsoleString(x, y, c1);

		if (colorIndex >= 0)
		{
			// Revert change and advance
			*c2 = '^';
			c2 += 2;
			c1 = c2;

			// Return if next string is empty
			if (!*c1)
				return x;

			// Setup color
			if (color == NULL && colorIndex <= 9 && gHUD.m_pCvarColorText->value == 1)
			{
				if (colorIndex == 0 || colorIndex == 9)
				{
					gEngfuncs.pfnDrawConsoleString(x, y, " ");	// Reset color to con_color
				}
				else
				{
					r = g_iColorsCodes[colorIndex][0] / 255.0;
					g = g_iColorsCodes[colorIndex][1] / 255.0;
					b = g_iColorsCodes[colorIndex][2] / 255.0;
					gEngfuncs.pfnDrawSetTextColor(r, g, b);
				}
			}
			else if (color != NULL)
				gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);
			continue;
		}

		// Done
		break;
	}
	return x;
}

char *RemoveColorCodes(const char *string, bool inPlace)
{
	static char buffer[1024];

	char *c1 = inPlace ? (char *)string : buffer;
	char *c2 = (char *)string;
	char *end = inPlace ? 0 : buffer + sizeof(buffer) - 1;
	while(*c2 && (inPlace || c1 < end))
	{
		if (*c2 == '^' && *(c2 + 1) >= '0' && *(c2 + 1) <= '9')
		{
			c2 += 2;
			continue;
		}
		*c1 = *c2;
		c1++;
		c2++;
	}
	*c1 = 0;

	return buffer;
}

void ConsolePrint(const char *string)
{
	if (gHUD.m_pCvarColorText->value == 0)
		gEngfuncs.pfnConsolePrint(string);
	else
		gEngfuncs.pfnConsolePrint(RemoveColorCodes(string));
}

void ConsolePrintColor(const char *string, RGBA color)
{
	RGBA oldColor = SetConsoleColor(color);
	if (gHUD.m_pCvarColorText->value == 0)
		gEngfuncs.pfnConsolePrint(string);
	else
		gEngfuncs.pfnConsolePrint(RemoveColorCodes(string));
	SetConsoleColor(oldColor);
}

void CenterPrint( const char *string )
{
	if (gHUD.m_pCvarColorText->value == 0)
		gEngfuncs.pfnCenterPrint(string);
	else
		gEngfuncs.pfnCenterPrint(RemoveColorCodes(string));
}

int CHud :: DrawHudStringColorCodes(int x, int y, const char *string, int _r, int _g, int _b)
{
	// How colorcodes work in DrawHudStringColorCodes
	// 1) Colorcodes are not ignored.
	// 2) Codes ^0 and ^9 reset color to _r, _g, _b.

	if (!string || !*string)
		return x;

	if (gHUD.m_pCvarColorText->value == 0)
		return gHUD.DrawHudString(x, y, string, _r, _g, _b);

	char *c1 = (char*)string;
	char *c2 = (char*)string;
	int r = _r, g = _g, b = _b;
	int colorIndex;
	while (true)
	{
		// Search for next color code
		colorIndex = -1;
		while (*c2 && *(c2 + 1) && !(*c2 == '^' && *(c2 + 1) >= '0' && *(c2 + 1) <= '9'))
			c2++;
		if (*c2 == '^' && *(c2 + 1) >= '0' && *(c2 + 1) <= '9')
		{
			colorIndex = *(c2 + 1) - '0';
			*c2 = 0;
		}
		// Draw current string
		x = gHUD.DrawHudString(x, y, c1, r, g, b);

		if (colorIndex >= 0)
		{
			// Revert change and advance
			*c2 = '^';
			c2 += 2;
			c1 = c2;

			// Return if next string is empty
			if (!*c1)
				return x;

			// Setup color
			if (colorIndex == 0 || colorIndex == 9)
			{
				// Reset color
				r = _r;
				g = _g;
				b = _b;
			}
			else
			{
				r = g_iColorsCodes[colorIndex][0];
				g = g_iColorsCodes[colorIndex][1];
				b = g_iColorsCodes[colorIndex][2];
			}
			continue;
		}

		// Done
		break;
	}
	return x;
}

int CHud::DrawHudStringReverseColorCodes(int x, int y, const char *string, int _r, int _g, int _b)
{
	if (!string || !*string)
		return x;

	if (gHUD.m_pCvarColorText->value == 0)
		return gHUD.DrawHudStringReverse(x, y, string, _r, _g, _b);

	// Move the string pos to the left to make it look like DrawHudStringReverse
	x -= TextMessageDrawString(ScreenWidth + 1, y, RemoveColorCodes(string), _r, _g, _b);

	return DrawHudStringColorCodes(x, y, string, _r, _g, _b);
}

void ConPrintf(const char *fmt, ...)
{
	static char str[1024];
	va_list args;
	va_start(args, fmt);

	vsnprintf(str, sizeof(str), fmt, args);
	str[sizeof(str) - 1] = '\0';
	gEngfuncs.pfnConsolePrint(str);

	va_end(args);
}

void ConPrintf(RGBA color, const char *fmt, ...)
{
	static char str[1024];

	RGBA oldcolor = SetConsoleColor(color);

	va_list args;
	va_start(args, fmt);

	vsnprintf(str, sizeof(str), fmt, args);
	str[sizeof(str) - 1] = '\0';
	gEngfuncs.pfnConsolePrint(str);

	va_end(args);

	SetConsoleColor(oldcolor);
}

// Code by voogru
// https://forums.alliedmods.net/showthread.php?t=60899?t=60899
long long ParseSteamID(const char *pszAuthID)
{
	if (!pszAuthID)
		return 0;

	int iServer = 0;
	int iAuthID = 0;

	char szAuthID[64];
	strncpy(szAuthID, pszAuthID, sizeof(szAuthID) - 1);
	szAuthID[sizeof(szAuthID) - 1] = '\0';

	char *szTmp = strtok(szAuthID, ":");
	while (szTmp = strtok(NULL, ":"))
	{
		char *szTmp2 = strtok(NULL, ":");
		if (szTmp2)
		{
			iServer = atoi(szTmp);
			iAuthID = atoi(szTmp2);
		}
	}

	if (iAuthID == 0)
		return 0;

	long long i64friendID = (long long)iAuthID * 2;

	//Friend ID's with even numbers are the 0 auth server.
	//Friend ID's with odd numbers are the 1 auth server.
	i64friendID += 76561197960265728 + iServer;

	return i64friendID;
}

// A hack to get player_info_t from the engine
// May not be correct on pre-SDL builds
engine_player_info_t *GetEnginePlayerInfo(int idx)
{
	hud_player_info_t info;
	GetPlayerInfo(idx, &info);
	if (!info.name)
		return nullptr;
	engine_player_info_t *ptr = reinterpret_cast<engine_player_info_t *>(info.name - offsetof(engine_player_info_t, name));
	return ptr;
}

long long GetPlayerSteamID64(int idx)
{
	// Only use engine_player_info_t if running an SDL build
	if (g_bNewerBuild)
	{
		engine_player_info_t *info = GetEnginePlayerInfo(idx);
		if (info->m_nSteamID / 10000000000000000 == 7)	// Check whether first digit is 7
			return info->m_nSteamID;
	}
	return ParseSteamID(g_PlayerSteamId[idx]);
}

#ifdef USE_VGUI2
bool VGUI2_IsCursorVisible()
{
	return vgui2::surface()->IsCursorVisible();
}
#endif

//-----------------------------------------------------
// Colors for ConPrintf
//-----------------------------------------------------
RGBA RGBA::ConColor::Red = RGBA(249, 54, 54);
RGBA RGBA::ConColor::Green = RGBA(77, 219, 83);
RGBA RGBA::ConColor::Yellow = RGBA(240, 205, 65);
RGBA RGBA::ConColor::Cyan = RGBA(111, 234, 247);
