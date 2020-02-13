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
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include <string.h>
#include <exception>

#include "dllexport.h"
#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#include <interface.h>
#include "vgui_SchemeManager.h"
#include "GameStudioModelRenderer.h"
#include "CHudSpectator.h"

extern "C"
{
#include "pm_shared.h"
}

#include "hud_servers.h"
#include "vgui_int.h"
#include "interface.h"
#include "svc_messages.h"
#include "memory.h"
#include "results.h"

#ifdef USE_VGUI2
#include "clientsteamcontext.h"
#endif
#ifdef USE_UPDATER
#include <CGameUpdater.h>
#endif

cl_enginefunc_t gEngfuncs;
CHud gHUD;
TeamFortressViewport *gViewPort = NULL;

void InitInput (void);
void ShutdownInput (void);
void EV_HookEvents( void );
void IN_Commands( void );
int HUD_IsGame(const char *game);

/*
========================== 
	Initialize

Called when the DLL is first loaded.
==========================
*/
extern "C" 
{
int		_DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion );
int		_DLLEXPORT HUD_VidInit( void );
void	_DLLEXPORT HUD_Init( void );
int		_DLLEXPORT HUD_Redraw( float flTime, int intermission );
int		_DLLEXPORT HUD_UpdateClientData( client_data_t *cdata, float flTime );
void	_DLLEXPORT HUD_Reset ( void );
void	_DLLEXPORT HUD_Shutdown( void );
void	_DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server );
void	_DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove );
char	_DLLEXPORT HUD_PlayerMoveTexture( char *name );
int		_DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
int		_DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs );
void	_DLLEXPORT HUD_Frame( double time );
void	_DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
void	_DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf );
void	_DLLEXPORT IN_ActivateMouse( void );
void	_DLLEXPORT IN_DeactivateMouse( void );
void	_DLLEXPORT IN_MouseEvent( int mstate );
void	_DLLEXPORT IN_Accumulate( void );
void	_DLLEXPORT IN_ClearStates( void );
struct kbutton_s _DLLEXPORT *KB_Find(const char *name);
void	_DLLEXPORT CL_CreateMove(float frametime, struct usercmd_s *cmd, int active);
int		_DLLEXPORT HUD_Key_Event(int eventcode, int keynum, const char *pszCurrentBinding);
void	_DLLEXPORT CAM_Think(void);
int		_DLLEXPORT CL_IsThirdPerson(void);
void	_DLLEXPORT CL_CameraOffset(float *ofs);
void	_DLLEXPORT V_CalcRefdef(struct ref_params_s *pparams);
int		_DLLEXPORT HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname);
void	_DLLEXPORT HUD_CreateEntities(void);
void	_DLLEXPORT HUD_StudioEvent(const struct mstudioevent_s *event, const struct cl_entity_s *entity);
void	_DLLEXPORT HUD_TxferLocalOverrides(struct entity_state_s *state, const struct clientdata_s *client);
void	_DLLEXPORT HUD_ProcessPlayerState(struct entity_state_s *dst, const struct entity_state_s *src);
void	_DLLEXPORT HUD_TxferPredictionData(struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd);
void	_DLLEXPORT HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int(*Callback_AddVisibleEntity)(struct cl_entity_s *pEntity), void(*Callback_TempEntPlaySound)(struct tempent_s *pTemp, float damp));
struct cl_entity_s _DLLEXPORT *HUD_GetUserEntity(int index);
void	_DLLEXPORT HUD_DrawNormalTriangles(void);
void	_DLLEXPORT HUD_DrawTransparentTriangles(void);
void	_DLLEXPORT HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed);
void	_DLLEXPORT Demo_ReadBuffer(int size, unsigned char *buffer);
int		_DLLEXPORT HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);
void	_DLLEXPORT ChatInputPosition(int *x, int *y);
}

//--------------------------------------------------------
// Console hook
// Hooks Con_Printf and pfnConsolePrint and redirects them
// to Con_DPrintf during game inititalization.
//--------------------------------------------------------
typedef void (*Con_PrintfFn) (char *fmt, ...);
typedef void (*ConsolePrintFn) (const char *string);

static Con_PrintfFn g_fnEngineConPrintf = nullptr;
static ConsolePrintFn g_fnEngineConsolePrint = nullptr;

static void HookConsoleFunctions()
{
	assert(!g_fnEngineConPrintf && !g_fnEngineConsolePrint);
	g_fnEngineConPrintf = gEngfuncs.Con_Printf;
	g_fnEngineConsolePrint = gEngfuncs.pfnConsolePrint;

	gEngfuncs.Con_Printf = gEngfuncs.Con_DPrintf;	// Declarations are identical
	gEngfuncs.pfnConsolePrint = [](const char *string)
	{
		gEngfuncs.Con_DPrintf("%s", string);
	};
}

static void UnhookConsoleFunctions()
{
	assert(g_fnEngineConPrintf && g_fnEngineConsolePrint);
	gEngfuncs.Con_Printf = g_fnEngineConPrintf;
	gEngfuncs.pfnConsolePrint = g_fnEngineConsolePrint;
}

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int _DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = Vector(-16, -16, -36);
		maxs = Vector(16, 16, 36);
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = Vector(-16, -16, -18 );
		maxs = Vector(16, 16, 18 );
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	_DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void _DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove )
{
	PM_Init( ppmove );
}

char _DLLEXPORT HUD_PlayerMoveTexture( char *name )
{
	return PM_FindTextureType( name );
}

void _DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server )
{
	PM_Move( ppmove, server );
}

int _DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	if (iVersion != CLDLL_INTERFACE_VERSION)
		return 0;

	gEngfuncs = *pEnginefuncs;

	EV_HookEvents();

	g_iIsAg = HUD_IsGame("ag");

	HookConsoleFunctions();

	return 1;
}


/*
==========================
	HUD_VidInit

Called whenever the client connects to a server.
==========================
*/

int _DLLEXPORT HUD_VidInit( void )
{
	try
	{
		gHUD.VidInit();
		VGui_Startup();
		g_StudioRenderer.InitOnConnect();
#ifdef _WIN32
		ResultsStop();
#endif
	}
	catch (const std::exception &e)
	{
		ConPrintf(RGBA::ConColor::Red, "HUD_VidInit: C++ exception thrown.\n");
		ConPrintf(RGBA::ConColor::Red, "HUD_VidInit: %s.\n", e.what());
	}
	return 1;
}

/*
==========================
	HUD_Init

Called when the game initializes and whenever the vid_mode is changed so the HUD can reinitialize itself.
Reinitializes all the hud variables.
==========================
*/

void _DLLEXPORT HUD_Init( void )
{
	try
	{
#ifdef USE_VGUI2
		ClientSteamContext().Activate();
#endif
		InitInput();
		gHUD.Init();
		Scheme_Init();
		Memory::OnHudInit();
		SvcMessagesInit();
#ifdef _WIN32
		ResultsInit();
#endif
	}
	catch (const std::exception &e)
	{
		ConPrintf(RGBA::ConColor::Red, "HUD_Init: C++ exception thrown.\n");
		ConPrintf(RGBA::ConColor::Red, "HUD_Init: %s.\n", e.what());
	}

	UnhookConsoleFunctions();
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int _DLLEXPORT HUD_Redraw( float time, int intermission )
{
	try
	{
		gHUD.Redraw(time, intermission);
	}
	catch (const std::exception &e)
	{
		ConPrintf(RGBA::ConColor::Red, "HUD_Redraw: C++ exception thrown.\n");
		ConPrintf(RGBA::ConColor::Red, "HUD_Redraw: %s.\n", e.what());
	}

	return 1;
}


/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int _DLLEXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime )
{
	try
	{
		IN_Commands();

		return gHUD.UpdateClientData(pcldata, flTime);
	}
	catch (const std::exception &e)
	{
		ConPrintf(RGBA::ConColor::Red, "HUD_UpdateClientData: C++ exception thrown.\n");
		ConPrintf(RGBA::ConColor::Red, "HUD_UpdateClientData: %s.\n", e.what());
		return 0;
	}
}

/*
==========================
	Obsolete: HUD_Reset

Obsolete: Called at start and end of demos to restore to "non"HUD state.
Obsolete: Doesn't called anymore from the engine.
==========================
*/

void _DLLEXPORT HUD_Reset( void )
{
	gHUD.VidInit();
}

/*
==========================
	HUD_Shutdown

Called at game exit.
==========================
*/

void _DLLEXPORT HUD_Shutdown( void )
{
	try
	{
		gHUD.Shutdown();
		ShutdownInput();
#ifdef USE_VGUI2
		ClientSteamContext().Shutdown();
#endif
	}
	catch (const std::exception &e)
	{
		ConPrintf(RGBA::ConColor::Red, "HUD_Shutdown: C++ exception thrown.\n");
		ConPrintf(RGBA::ConColor::Red, "HUD_Shutdown: %s.\n", e.what());
	}
}

/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void _DLLEXPORT HUD_Frame( double time )
{
	try
	{
		gHUD.Frame(time);
		Memory::OnFrame();
#ifdef _WIN32
		ResultsFrame(time);
#endif
#ifdef USE_UPDATER
		gGameUpdater->Frame();
#endif

		ServersThink(time);

		GetClientVoiceMgr()->Frame(time);
	}
	catch (const std::exception &e)
	{
		ConPrintf(RGBA::ConColor::Red, "HUD_Frame: C++ exception thrown.\n");
		ConPrintf(RGBA::ConColor::Red, "HUD_Frame: %s.\n", e.what());
	}
}


/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void _DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);
}

/*
==========================
HUD_DirectorEvent

Called when a director event message was received
==========================
*/

void _DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf )
{
	 gHUD.m_Spectator->DirectorMessage( iSize, pbuf );
}


#ifdef USE_VGUI2
/**
*	This function is only called in pre-SteamPipe builds
*/
extern "C" DLLEXPORT void* ClientFactory()
{
#ifdef WIN32
	return CreateInterface;
#else
	return nullptr;		// Not called on Linux and CreateInterface causes a compilation error
#endif
}
#endif

#include "../public/cl_dll/IGameClientExports.h"

//-----------------------------------------------------------------------------
// Purpose: Exports functions that are used by the gameUI for UI dialogs
//-----------------------------------------------------------------------------
class CClientExports : public IGameClientExports
{
public:
	// returns the name of the server the user is connected to, if any
	virtual const char *GetServerHostName()
	{
		if (gHUD.GetServerName())
		{
			return gHUD.GetServerName();
		}
		return "";
	}

	// ingame voice manipulation
	virtual bool IsPlayerGameVoiceMuted(int playerIndex)
	{
		if (GetClientVoiceMgr())
			return GetClientVoiceMgr()->IsPlayerBlocked(playerIndex);
		return false;
	}

	virtual void MutePlayerGameVoice(int playerIndex)
	{
		if (GetClientVoiceMgr())
		{
			GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, true);
		}
	}

	virtual void UnmutePlayerGameVoice(int playerIndex)
	{
		if (GetClientVoiceMgr())
		{
			GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, false);
		}
	}
};

EXPOSE_SINGLE_INTERFACE(CClientExports, IGameClientExports, GAMECLIENTEXPORTS_INTERFACE_VERSION);

extern "C" void DLLEXPORT F(void *pv)
{
	cldll_func_t *pcldll_func = (cldll_func_t *)pv;

	cldll_func_t cldll_func =
	{
	Initialize,
	HUD_Init,
	HUD_VidInit,
	HUD_Redraw,
	HUD_UpdateClientData,
	HUD_Reset,
	HUD_PlayerMove,
	HUD_PlayerMoveInit,
	HUD_PlayerMoveTexture,
	IN_ActivateMouse,
	IN_DeactivateMouse,
	IN_MouseEvent,
	IN_ClearStates,
	IN_Accumulate,
	CL_CreateMove,
	CL_IsThirdPerson,
	CL_CameraOffset,
	KB_Find,
	CAM_Think,
	V_CalcRefdef,
	HUD_AddEntity,
	HUD_CreateEntities,
	HUD_DrawNormalTriangles,
	HUD_DrawTransparentTriangles,
	HUD_StudioEvent,
	HUD_PostRunCmd,
	HUD_Shutdown,
	HUD_TxferLocalOverrides,
	HUD_ProcessPlayerState,
	HUD_TxferPredictionData,
	Demo_ReadBuffer,
	HUD_ConnectionlessPacket,
	HUD_GetHullBounds,
	HUD_Frame,
	HUD_Key_Event,
	HUD_TempEntUpdate,
	HUD_GetUserEntity,
	HUD_VoiceStatus,
	HUD_DirectorMessage,
	HUD_GetStudioModelInterface,
	ChatInputPosition,
	nullptr,	// pGetPlayerTeam
#ifdef USE_VGUI2
	ClientFactory
#endif
	};

	*pcldll_func = cldll_func;
}
