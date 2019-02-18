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
//  cdll_int.h
//
// 4-23-98  
// JOHN:  client dll interface declarations
//

#ifndef CDLL_INT_H
#define CDLL_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"
#include "archtypes.h"
#include "Sequence.h"
#include "wrect.h"

// this file is included by both the engine and the client-dll,
// so make sure engine declarations aren't done twice

typedef int HLHSPRITE;	// handle to a graphic

#define SCRINFO_SCREENFLASH 1
#define SCRINFO_STRETCHED	2

typedef struct SCREENINFO_s
{
	int		iSize;
	int		iWidth;
	int		iHeight;
	int		iFlags;
	int		iCharHeight;
	short	charWidths[256];
} SCREENINFO;


typedef struct client_data_s
{
	// fields that cannot be modified  (ie. have no effect if changed)
	vec3_t origin;

	// fields that can be changed by the cldll
	vec3_t viewangles;
	int		iWeaponBits;
	float	fov;	// field of view
} client_data_t;

typedef struct client_sprite_s
{
	char szName[64];
	char szSprite[64];
	int hspr;
	int iRes;
	wrect_t rc;
} client_sprite_t;

typedef struct hud_player_info_s
{
	char *name;
	short ping;
	byte thisplayer;  // TRUE if this is the calling player

	byte spectator;	// set via Spectator user messages, so work only on server with them
	byte packetloss;

	char *model;
	short topcolor;
	short bottomcolor;

	// The variable below is supposed to be SteamID64 of the player
	// but it is only set if the game is CS1.6 or CSCZ
	// The game may crash if you start the client DLL from 
	// one of these games without that padding variable
	uint64 padding;

} hud_player_info_t;

// Reverse-engineered from DWARF debug data on Linux
typedef struct engine_player_info_s
{
	int userid;
	char userinfo[256];
	char name[32];
	int spectator;
	int ping;
	int packet_loss;
	char model[64];
	int topcolor;
	int bottomcolor;
	int renderframe;
	int gaitsequence;
	int gaitframe;
	int gaityaw;
	int prevgaitorigin[3];
	char customdata[0xA4];
	char hashedcdkey[16];
	uint64 m_nSteamID;
} engine_player_info_t;

typedef struct cl_enginefuncs_s
{
	// sprite handlers
	HLHSPRITE						( *pfnSPR_Load )			( const char *szPicName );
	int							( *pfnSPR_Frames )			( HLHSPRITE hPic );
	int							( *pfnSPR_Height )			( HLHSPRITE hPic, int frame );
	int							( *pfnSPR_Width )			( HLHSPRITE hPic, int frame );
	void						( *pfnSPR_Set )				( HLHSPRITE hPic, int r, int g, int b );
	void						( *pfnSPR_Draw )			( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_DrawHoles )		( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_DrawAdditive )	( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_EnableScissor )	( int x, int y, int width, int height );
	void						( *pfnSPR_DisableScissor )	( void );
	client_sprite_t				*( *pfnSPR_GetList )			( char *psz, int *piCount );

	// screen handlers
	void						( *pfnFillRGBA )			( int x, int y, int width, int height, int r, int g, int b, int a );
	int							( *pfnGetScreenInfo ) 		( SCREENINFO *pscrinfo );
	void						( *pfnSetCrosshair )		( HLHSPRITE hspr, wrect_t rc, int r, int g, int b );

	// cvar handlers
	struct cvar_s				*( *pfnRegisterVariable )	( char *szName, char *szValue, int flags );
	float						( *pfnGetCvarFloat )		( char *szName );
	char*						( *pfnGetCvarString )		( char *szName );

	// command handlers
	int							( *pfnAddCommand )			( char *cmd_name, void (*function)(void) );
	int							( *pfnHookUserMsg )			( char *szMsgName, pfnUserMsgHook pfn );
	int							( *pfnServerCmd )			( char *szCmdString );
	int							( *pfnClientCmd )			( char *szCmdString );

	void						( *pfnGetPlayerInfo )		( int ent_num, hud_player_info_t *pinfo );

	// sound handlers
	void						( *pfnPlaySoundByName )		( char *szSound, float volume );
	void						( *pfnPlaySoundByIndex )	( int iSound, float volume );

	// vector helpers
	void						( *pfnAngleVectors )		( const float * vecAngles, float * forward, float * right, float * up );

	// text message system
	client_textmessage_t		*( *pfnTextMessageGet )		( const char *pName );
	int							( *pfnDrawCharacter )		( int x, int y, int number, int r, int g, int b );
	int							( *pfnDrawConsoleString )	( int x, int y, char *string );
	void						( *pfnDrawSetTextColor )	( float r, float g, float b );
	void						( *pfnDrawConsoleStringLen )(  const char *string, int *length, int *height );

	void						( *pfnConsolePrint )		( const char *string );
	void						( *pfnCenterPrint )			( const char *string );


// Added for user input processing
	int							( *GetWindowCenterX )		( void );
	int							( *GetWindowCenterY )		( void );
	void						( *GetViewAngles )			( float * );
	void						( *SetViewAngles )			( float * );
	int							( *GetMaxClients )			( void );
	void						( *Cvar_SetValue )			( char *cvar, float value );

	int       					(*Cmd_Argc)					(void);	
	char						*( *Cmd_Argv )				( int arg );
	void						( *Con_Printf )				( char *fmt, ... );
	void						( *Con_DPrintf )			( char *fmt, ... );
	void						( *Con_NPrintf )			( int pos, char *fmt, ... );
	void						( *Con_NXPrintf )			( struct con_nprint_s *info, char *fmt, ... );

	const char					*( *PhysInfo_ValueForKey )	( const char *key );
	const char					*( *ServerInfo_ValueForKey )( const char *key );
	float						( *GetClientMaxspeed )		( void );
	int							( *CheckParm )				( char *parm, char **ppnext );
	void						( *Key_Event )				( int key, int down );
	void						( *GetMousePosition )		( int *mx, int *my );
	int							( *IsNoClipping )			( void );

	struct cl_entity_s			*( *GetLocalPlayer )		( void );
	struct cl_entity_s			*( *GetViewModel )			( void );
	struct cl_entity_s			*( *GetEntityByIndex )		( int idx );

	float						( *GetClientTime )			( void );
	void						( *V_CalcShake )			( void );
	void						( *V_ApplyShake )			( float *origin, float *angles, float factor );

	int							( *PM_PointContents )		( float *point, int *truecontents );
	int							( *PM_WaterEntity )			( float *p );
	struct pmtrace_s			*( *PM_TraceLine )			( float *start, float *end, int flags, int usehull, int ignore_pe );

	struct model_s				*( *CL_LoadModel )			( const char *modelname, int *index );
	int							( *CL_CreateVisibleEntity )	( int type, struct cl_entity_s *ent );

	const struct model_s *		( *GetSpritePointer )		( HLHSPRITE hSprite );
	void						( *pfnPlaySoundByNameAtLocation )	( char *szSound, float volume, float *origin );

	unsigned short				( *pfnPrecacheEvent )		( int type, const char* psz );
	void						( *pfnPlaybackEvent )		( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	void						( *pfnWeaponAnim )			( int iAnim, int body );
	float						( *pfnRandomFloat )			( float flLow, float flHigh );
	int32						( *pfnRandomLong )			( int32 lLow, int32 lHigh );
	void						( *pfnHookEvent )			( char *name, void ( *pfnEvent )( struct event_args_s *args ) );
	int							(*Con_IsVisible)			();
	const char					*( *pfnGetGameDirectory )	( void );
	struct cvar_s				*( *pfnGetCvarPointer )		( const char *szName );
	const char					*( *Key_LookupBinding )		( const char *pBinding );
	const char					*( *pfnGetLevelName )		( void );
	void						( *pfnGetScreenFade )		( struct screenfade_s *fade );
	void						( *pfnSetScreenFade )		( struct screenfade_s *fade );
	void                        *( *VGui_GetPanel )         ( );
	void                         ( *VGui_ViewportPaintBackground ) (int extents[4]);

	byte*						(*COM_LoadFile)				( char *path, int usehunk, int *pLength );
	char*						(*COM_ParseFile)			( char *data, char *token );
	void						(*COM_FreeFile)				( void *buffer );
		
	struct triangleapi_s		*pTriAPI;
	struct efx_api_s			*pEfxAPI;
	struct event_api_s			*pEventAPI;
	struct demo_api_s			*pDemoAPI;
	struct net_api_s			*pNetAPI;
	struct IVoiceTweak_s		*pVoiceTweak;

	// returns 1 if the client is a spectator only (connected to a proxy), 0 otherwise or 2 if in dev_overview mode
	int							( *IsSpectateOnly ) ( void );
	struct model_s				*( *LoadMapSprite )			( const char *filename );

	// file search functions
	void						( *COM_AddAppDirectoryToSearchPath ) ( const char *pszBaseDir, const char *appName );
	int							( *COM_ExpandFilename)				 ( const char *fileName, char *nameOutBuffer, int nameOutBufferSize );

	// User info
	// playerNum is in the range (1, MaxClients)
	// returns NULL if player doesn't exit
	// returns "" if no value is set
	const char					*( *PlayerInfo_ValueForKey )( int playerNum, const char *key );
	void						( *PlayerInfo_SetValueForKey )( const char *key, const char *value );

	// Gets a unique ID for the specified player. This is the same even if you see the player on a different server.
	// iPlayer is an entity index, so client 0 would use iPlayer=1.
	// Returns false if there is no player on the server in the specified slot.
	qboolean					(*GetPlayerUniqueID)(int iPlayer, char playerID[16]);

	// TrackerID access
	int							(*GetTrackerIDForPlayer)(int playerSlot);
	int							(*GetPlayerForTrackerID)(int trackerID);

	// Same as pfnServerCmd, but the message goes in the unreliable stream so it can't clog the net stream
	// (but it might not get there).
	int							( *pfnServerCmdUnreliable )( char *szCmdString );

	void						( *pfnGetMousePos )( struct tagPOINT *ppt );
	void						( *pfnSetMousePos )( int x, int y );
	void						( *pfnSetMouseEnable )( qboolean fEnable );

	int		pfnUnks001[1];

	struct	CommandLink *		( *pfnGetCommandsList )		( void );

	int		pfnUnks002[11];

	int							( *pfnDrawString )			( int x, int y, const char* string, int r, int g, int b );
	int							( *pfnDrawStringReverse )	( int x, int y, const char* string, int r, int g, int b );

	int		pfnUnks003[17];
} cl_enginefunc_t;

#ifndef IN_BUTTONS_H
#include "in_buttons.h"
#endif

#define CLDLL_INTERFACE_VERSION		7

extern void ClientDLL_Init( void ); // from cdll_int.c
extern void ClientDLL_Shutdown( void );
extern void ClientDLL_HudInit( void );
extern void ClientDLL_HudVidInit( void );
extern void	ClientDLL_UpdateClientData( void );
extern void ClientDLL_Frame( double time );
extern void ClientDLL_HudRedraw( int intermission );
extern void ClientDLL_MoveClient( struct playermove_s *ppmove );
extern void ClientDLL_ClientMoveInit( struct playermove_s *ppmove );
extern char ClientDLL_ClientTextureType( char *name );

extern void ClientDLL_CreateMove( float frametime, struct usercmd_s *cmd, int active );
extern void ClientDLL_ActivateMouse( void );
extern void ClientDLL_DeactivateMouse( void );
extern void ClientDLL_MouseEvent( int mstate );
extern void ClientDLL_ClearStates( void );
extern int ClientDLL_IsThirdPerson( void );
extern void ClientDLL_GetCameraOffsets( float *ofs );
extern int ClientDLL_GraphKeyDown( void );
extern struct kbutton_s *ClientDLL_FindKey( const char *name );
extern void ClientDLL_CAM_Think( void );
extern void ClientDLL_IN_Accumulate( void );
extern void ClientDLL_CalcRefdef( struct ref_params_s *pparams );
extern int ClientDLL_AddEntity( int type, struct cl_entity_s *ent );
extern void ClientDLL_CreateEntities( void );

extern void ClientDLL_DrawNormalTriangles( void );
extern void ClientDLL_DrawTransparentTriangles( void );
extern void ClientDLL_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
extern void ClientDLL_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
extern void ClientDLL_TxferLocalOverrides( struct entity_state_s *state, const struct clientdata_s *client );
extern void ClientDLL_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src );
extern void ClientDLL_TxferPredictionData ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
extern void ClientDLL_ReadDemoBuffer( int size, unsigned char *buffer );
extern int ClientDLL_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
extern int ClientDLL_GetHullBounds( int hullnumber, float *mins, float *maxs );

extern void ClientDLL_VGui_ConsolePrint(const char* text);

extern int ClientDLL_Key_Event( int down, int keynum, const char *pszCurrentBinding );
extern void ClientDLL_TempEntUpdate( double ft, double ct, double grav, struct tempent_s **ppFreeTE, struct tempent_s **ppActiveTE, int ( *addTEntity )( struct cl_entity_s *pEntity ), void ( *playTESound )( struct tempent_s *pTemp, float damp ) );
extern struct cl_entity_s *ClientDLL_GetUserEntity( int index );
extern void ClientDLL_VoiceStatus(int entindex, qboolean bTalking);
extern void ClientDLL_DirectorMessage( int iSize, void *pbuf );


// ********************************************************
// Functions exported by the client .dll
// ********************************************************

// Function type declarations for client exports
typedef int (*INITIALIZE_FUNC)	( struct cl_enginefuncs_s*, int );
typedef void (*HUD_INIT_FUNC)		( void );
typedef int (*HUD_VIDINIT_FUNC)	( void );
typedef int (*HUD_REDRAW_FUNC)	( float, int );
typedef int (*HUD_UPDATECLIENTDATA_FUNC) ( struct client_data_s*, float );
typedef void (*HUD_RESET_FUNC)    ( void );
typedef void (*HUD_CLIENTMOVE_FUNC)( struct playermove_s *ppmove, qboolean server );
typedef void (*HUD_CLIENTMOVEINIT_FUNC)( struct playermove_s *ppmove );
typedef char (*HUD_TEXTURETYPE_FUNC)( char *name );
typedef void (*HUD_IN_ACTIVATEMOUSE_FUNC) ( void );
typedef void (*HUD_IN_DEACTIVATEMOUSE_FUNC)		( void );
typedef void (*HUD_IN_MOUSEEVENT_FUNC)		( int mstate );
typedef void (*HUD_IN_CLEARSTATES_FUNC)		( void );
typedef void (*HUD_IN_ACCUMULATE_FUNC ) ( void );
typedef void (*HUD_CL_CREATEMOVE_FUNC)		( float frametime, struct usercmd_s *cmd, int active );
typedef int (*HUD_CL_ISTHIRDPERSON_FUNC) ( void );
typedef void (*HUD_CL_GETCAMERAOFFSETS_FUNC )( float *ofs );
typedef struct kbutton_s * (*HUD_KB_FIND_FUNC) ( const char *name );
typedef void ( *HUD_CAMTHINK_FUNC )( void );
typedef void ( *HUD_CALCREF_FUNC ) ( struct ref_params_s *pparams );
typedef int	 ( *HUD_ADDENTITY_FUNC ) ( int type, struct cl_entity_s *ent, const char *modelname );
typedef void ( *HUD_CREATEENTITIES_FUNC ) ( void );
typedef void ( *HUD_DRAWNORMALTRIS_FUNC ) ( void );
typedef void ( *HUD_DRAWTRANSTRIS_FUNC ) ( void );
typedef void ( *HUD_STUDIOEVENT_FUNC ) ( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
typedef void ( *HUD_POSTRUNCMD_FUNC ) ( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
typedef void ( *HUD_SHUTDOWN_FUNC ) ( void );
typedef void ( *HUD_TXFERLOCALOVERRIDES_FUNC )( struct entity_state_s *state, const struct clientdata_s *client );
typedef void ( *HUD_PROCESSPLAYERSTATE_FUNC )( struct entity_state_s *dst, const struct entity_state_s *src );
typedef void ( *HUD_TXFERPREDICTIONDATA_FUNC ) ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
typedef void ( *HUD_DEMOREAD_FUNC ) ( int size, unsigned char *buffer );
typedef int ( *HUD_CONNECTIONLESS_FUNC )( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
typedef	int	( *HUD_GETHULLBOUNDS_FUNC ) ( int hullnumber, float *mins, float *maxs );
typedef void (*HUD_FRAME_FUNC)		( double );
typedef int (*HUD_KEY_EVENT_FUNC ) ( int eventcode, int keynum, const char *pszCurrentBinding );
typedef void (*HUD_TEMPENTUPDATE_FUNC) ( double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int ( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ),	void ( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
typedef struct cl_entity_s *(*HUD_GETUSERENTITY_FUNC ) ( int index );
typedef void (*HUD_VOICESTATUS_FUNC)(int entindex, qboolean bTalking);
typedef void (*HUD_DIRECTORMESSAGE_FUNC)( int iSize, void *pbuf );
typedef int ( *HUD_STUDIO_INTERFACE_FUNC )( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio );
typedef void (*HUD_CHATINPUTPOSITION_FUNC)( int *x, int *y );
typedef int (*HUD_GETPLAYERTEAM)(int iplayer);
typedef void *(*CLIENTFACTORY)(); // this should be CreateInterfaceFn but that means including interface.h
									// which is a C++ file and some of the client files a C only... 
									// so we return a void * which we then do a typecast on later.


// Pointers to the exported client functions themselves
typedef struct
{
	INITIALIZE_FUNC						pInitFunc;
	HUD_INIT_FUNC						pHudInitFunc;
	HUD_VIDINIT_FUNC					pHudVidInitFunc;
	HUD_REDRAW_FUNC						pHudRedrawFunc;
	HUD_UPDATECLIENTDATA_FUNC			pHudUpdateClientDataFunc;
	HUD_RESET_FUNC						pHudResetFunc;
	HUD_CLIENTMOVE_FUNC					pClientMove;
	HUD_CLIENTMOVEINIT_FUNC				pClientMoveInit;
	HUD_TEXTURETYPE_FUNC				pClientTextureType;
	HUD_IN_ACTIVATEMOUSE_FUNC			pIN_ActivateMouse;
	HUD_IN_DEACTIVATEMOUSE_FUNC			pIN_DeactivateMouse;
	HUD_IN_MOUSEEVENT_FUNC				pIN_MouseEvent;
	HUD_IN_CLEARSTATES_FUNC				pIN_ClearStates;
	HUD_IN_ACCUMULATE_FUNC				pIN_Accumulate;
	HUD_CL_CREATEMOVE_FUNC				pCL_CreateMove;
	HUD_CL_ISTHIRDPERSON_FUNC			pCL_IsThirdPerson;
	HUD_CL_GETCAMERAOFFSETS_FUNC		pCL_GetCameraOffsets;
	HUD_KB_FIND_FUNC					pFindKey;
	HUD_CAMTHINK_FUNC					pCamThink;
	HUD_CALCREF_FUNC					pCalcRefdef;
	HUD_ADDENTITY_FUNC					pAddEntity;
	HUD_CREATEENTITIES_FUNC				pCreateEntities;
	HUD_DRAWNORMALTRIS_FUNC				pDrawNormalTriangles;
	HUD_DRAWTRANSTRIS_FUNC				pDrawTransparentTriangles;
	HUD_STUDIOEVENT_FUNC				pStudioEvent;
	HUD_POSTRUNCMD_FUNC					pPostRunCmd;
	HUD_SHUTDOWN_FUNC					pShutdown;
	HUD_TXFERLOCALOVERRIDES_FUNC		pTxferLocalOverrides;
	HUD_PROCESSPLAYERSTATE_FUNC			pProcessPlayerState;
	HUD_TXFERPREDICTIONDATA_FUNC		pTxferPredictionData;
	HUD_DEMOREAD_FUNC					pReadDemoBuffer;
	HUD_CONNECTIONLESS_FUNC				pConnectionlessPacket;
	HUD_GETHULLBOUNDS_FUNC				pGetHullBounds;
	HUD_FRAME_FUNC						pHudFrame;
	HUD_KEY_EVENT_FUNC					pKeyEvent;
	HUD_TEMPENTUPDATE_FUNC				pTempEntUpdate;
	HUD_GETUSERENTITY_FUNC				pGetUserEntity;
	HUD_VOICESTATUS_FUNC				pVoiceStatus;		// Possibly null on old client dlls.
	HUD_DIRECTORMESSAGE_FUNC			pDirectorMessage;	// Possibly null on old client dlls.
	HUD_STUDIO_INTERFACE_FUNC			pStudioInterface;	// Not used by all clients
	HUD_CHATINPUTPOSITION_FUNC			pChatInputPosition;	// Not used by all clients
	HUD_GETPLAYERTEAM					pGetPlayerTeam; // Not used by all clients
	CLIENTFACTORY						pClientFactory;
} cldll_func_t;

#ifdef __cplusplus
}
#endif

#endif // CDLL_INT_H
