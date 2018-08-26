#ifndef __APIPROXY__
#define __APIPROXY__

#include "archtypes.h"     // DAL
#include "netadr.h"
#include "Sequence.h"

#ifndef _WIN32
#include "enums.h"
#endif

#include "cmd_function_t.h"

struct cl_enginefunc_t;
struct kbutton_t;
struct cl_entity_t;
struct client_data_t;
//Not a typo, both of these exist. - Solokiller
struct clientdata_t;
struct playermove_t;
struct usercmd_t;
struct ref_params_t;
struct mstudioevent_t;
struct local_state_t;
struct entity_state_t;
struct weapon_data_t;
struct netadr_t;
struct TEMPENTITY;
struct r_studio_interface_t;
struct engine_studio_api_t;
struct client_sprite_t;
struct cvar_t;
struct hud_player_info_t;
struct client_textmessage_t;
struct SCREENINFO;
struct wrect_t;
struct con_nprint_t;
struct edict_t;
struct event_args_t;
struct screenfade_t;
struct model_t;
typedef struct tagPOINT POINT;
struct pmtrace_t;

struct triangleapi_t;
struct efx_api_t;
struct event_api_t;
struct demo_api_t;
struct net_api_t;
struct IVoiceTweak;

typedef triangleapi_t*		pTriAPI;
typedef efx_api_t*			pEfxAPI;
typedef event_api_t*		pEventAPI;
typedef demo_api_t*			pDemoAPI;
typedef net_api_t*			pNetAPI;
typedef IVoiceTweak*		pVoiceTweak;

#define	MAX_ALIAS_NAME	32

struct cmdalias_t
{
	cmdalias_t	*next;
	char		name[MAX_ALIAS_NAME];
	char		*value;
};

//Now defined here. Who ever thought defining this somewhere else was a good idea? - Solokiller
using pfnUserMsgHook = int ( * )( const char *pszName, int iSize, void *pbuf );

using Callback_AddVisibleEntity = int ( * )( cl_entity_t* pEntity );
using Callback_TempEntPlaySound = void ( * )( TEMPENTITY* pTemp, float damp );
using pfnEventHook = void ( * )( event_args_t* args );

/**
*	Size of buffers used to store unique player IDs.
*/
const size_t PLAYERID_BUFFER_SIZE = 16;

// ********************************************************
// Functions exported by the client .dll
// ********************************************************

// Pointers to the exported client functions themselves
struct cldll_func_t
{
	/**
	*	Called to initialize the client library.
	*	@param pEngineFuncs Pointer to the engine functions interface;
	*	@param iVersion Interface version. Must match CLDLL_INTERFACE_VERSION.
	*	@return true on success, false otherwise. If iVersion does not match CLDLL_INTERFACE_VERSION, return false.
	*	@see CLDLL_INTERFACE_VERSION
	*/
	int				( *pInitFunc )					( cl_enginefunc_t* pEngineFuncs, int iVersion );

	/**
	*	Called to initialize the client library. This occurs after the engine has loaded the client and has initialized all other systems.
	*/
	void			( *pHudInitFunc )				( void );

	/**
	*	Called after a connection to a server has been established.
	*/
	int				( *pHudVidInitFunc )			( void );

	/**
	*	Called to redraw the HUD.
	*	@param flCurrentTime Current game time.
	*	@param bIsIntermission Whether we're currently in intermission or not.
	*	@return Ignored. true on success, false otherwise.
	*/
	int				( *pHudRedrawFunc )				( const float flCurrentTime, const int bIsIntermission );

	/**
	*	Called every frame while running a map.
	*	@param pCLData Client data.
	*	@param flCurrentTime Current game time.
	*	@return true if client data was updated, false otherwise.
	*/
	int				( *pHudUpdateClientDataFunc )	( client_data_t* pCLData, const float flCurrentTime );

	/**
	*	Obsolete. Is never called.
	*/
	void			( *pHudResetFunc )				( void );

	/**
	*	Run client side player movement and physics code.
	*	@param ppmove Player movement data.
	*	@param server Whether this is the server or client running movement.
	*/
	void			( *pClientMove )				( playermove_t* ppmove, const qboolean server );

	/**
	*	Initializes the client side player movement code.
	*	@param ppmove Player movement data.
	*/
	void			( *pClientMoveInit )			( playermove_t* ppmove );

	/**
	*	Gets the texture type for a given texture name.
	*	Never called by the engine itself.
	*	@param pszName Texture name.
	*	@return Texture type.
	*/
	char			( *pClientTextureType )			( const char*const pszName );

	/**
	*	Called when mouse input is activated.
	*/
	void			( *pIN_ActivateMouse )			( void );

	/**
	*	Called when mouse input is deactivated.
	*/
	void			( *pIN_DeactivateMouse )		( void );

	/**
	*	Called when a mouse event has occurred.
	*	@param mstate Bit vector containing new mouse button states.
	*/
	void			( *pIN_MouseEvent )				( const int mstate );

	/**
	*	Clears all mouse button states.
	*/
	void			( *pIN_ClearStates )			( void );

	/**
	*	Called to accumulate relative mouse movement.
	*/
	void			( *pIN_Accumulate )				( void );

	/**
	*	Creates a movement command to send to the server.
	*	@param frametime Delta time between the last and current frame.
	*	@param cmd Command to fill in.
	*	@param bActive if bActive == 1 then we are 1) not playing back demos ( where our commands are ignored ) and
	*					2 ) we have finished signing on to server
	*/
	void			( *pCL_CreateMove )				( float frametime, usercmd_t* cmd, const int bActive );

	/**
	*	@return Whether the client is currently in third person mode.
	*/
	int				( *pCL_IsThirdPerson )			( void );

	/**
	*	Gets the camera offset.
	*	@param[ out ] ofs Offset.
	*/
	void			( *pCL_GetCameraOffsets )		( Vector& ofs );

	/**
	*	Finds a key by name.
	*	@param pszName Key name.
	*	@return Key, or null if it couldn't be found.
	*/
	kbutton_t*		( *pFindKey )					( const char* const pszName );

	/**
	*	Runs camera think.
	*/
	void			( *pCamThink )					( void );

	/**
	*	Calculates view data.
	*/
	void			( *pCalcRefdef )				( ref_params_t* pparams );

	/**
	*	Called when the engine has created a client side copy of an entity.
	*	@param type Entity type. @see EntityType
	*	@param ent Entity.
	*	@param pszModelName Name of the model that the entity is using. Same as ent->model->name. Is an empty string if it has no model.
	*	@return true to add it to the list of visible entities, false to filter it out.
	*/
	int				( *pAddEntity )					( int type, cl_entity_t* ent, const char* const pszModelName );

	/**
	*	Gives us a chance to add additional entities to the render this frame.
	*/
	void			( *pCreateEntities )			( void );

	/**
	*	Lets the client draw non-transparent geometry.
	*/
	void			( *pDrawNormalTriangles )		( void );

	/**
	*	Lets the client draw transparent geometry.
	*/
	void			( *pDrawTransparentTriangles )	( void );

	/**
	*	A studiomodel event has occured while advancing an entity's frame.
	*	@param event Event.
	*	@param entity Entity whose frame is being advanced.
	*/
	void			( *pStudioEvent )				( const mstudioevent_t* event, const cl_entity_t* entity );

	/**
	*	Client calls this during prediction, after it has moved the player and updated any info changed into to->
	*	time is the current client clock based on prediction
	*	cmd is the command that caused the movement, etc
	*	bRunFuncs is true if this is the first time we've predicted this command.  If so, sounds and effects should play, otherwise, they should
	*	be ignored
	*	@param from Old state.
	*	@param to New state.
	*	@param cmd Command that was ran.
	*	@param bRunFuncs Whether to play sounds and effects.
	*	@param time Current time.
	*	@param random_seed Shared random seed.
	*/
	void			( *pPostRunCmd )				( local_state_t* from, local_state_t* to, usercmd_t* cmd, const int bRunFuncs, double time, unsigned int random_seed );

	/**
	*	Called when the client shuts down. The library is freed after this call.
	*/
	void			( *pShutdown )					( void );

	/**
	*	The server sends us our origin with extra precision as part of the clientdata structure, not during the normal
	*	playerstate update in entity_state_t.  In order for these overrides to eventually get to the appropriate playerstate
	*	structure, we need to copy them into the state structure at this point.
	*	@param state Player entity state.
	*	@param client Player client state.
	*/
	void			( *pTxferLocalOverrides )		( entity_state_t* state, const clientdata_t* client );

	/**
	*	We have received entity_state_t for this player over the network.  We need to copy appropriate fields to the
	*	playerstate structure.
	*	@param dst Destination state.
	*	@param src Source state.
	*/
	void			( *pProcessPlayerState )		( entity_state_t* dst, const entity_state_t* src );

	/**
	*	Because we can predict an arbitrary number of frames before the server responds with an update, we need to be able to copy client side prediction data in
	*	from the state that the server ack'd receiving, which can be anywhere along the predicted frame path ( i.e., we could predict 20 frames into the future and the server ack's
	*	up through 10 of those frames, so we need to copy persistent client-side only state from the 10th predicted frame to the slot the server
	*	update is occupying. )
	*	@param ps Current player entity state.
	*	@param pps Current predicted player entity state.
	*	@param pcd Current client state.
	*	@param ppcd Current predicted player entity state.
	*	@param wd Current weapon data list.
	*	@param pwd Current predicted weapon data list.
	*/
	void			( *pTxferPredictionData )		( entity_state_t* ps, const entity_state_t* pps, clientdata_t* pcd, const clientdata_t* ppcd, weapon_data_t* wd, const weapon_data_t* pwd );

	/**
	*	Called by the engine while playing back a demo. The engine wants us to parse some data from the demo stream.
	*	@param size Buffer size, in bytes.
	*	@param buffer Buffer.
	*/
	void			( *pReadDemoBuffer )			( int size, unsigned char *buffer );

	/**
	*	Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
	*	size of the response_buffer, so you must zero it out if you choose not to respond.
	*	@param net_from Address of the sender.
	*	@param args Command arguments.
	*	@param response_buffer Buffer to write responses to.
	*	@param response_buffer_size Buffer size.
	*	@return true if the packet is valid, false otherwise.
	*/
	int				( *pConnectionlessPacket )		( const netadr_t* net_from, const char *args, char *response_buffer, int *response_buffer_size );

	/**
	*	Gets hull bounds for physics.
	*	@param hullnumber Hull number. @see Hull::Hull
	*	@param[ out ] mins Minimum bounds.
	*	@param[ out ] maxs Maximum bounds.
	*	@return true if the bounds were set, false otherwise.
	*/
	int				( *pGetHullBounds )				( int hullnumber, Vector& mins, Vector& maxs );

	/**
	*	Called every frame that the client library is loaded.
	*	@param flFrameTime Time between the last and current frame.
	*/
	void			( *pHudFrame )					( const double flFrameTime );

	/**
	*	Called when a key has changed state.
	*	@param bDown Whether they key is down or not.
	*	@param keynum Key number. @see KeyNum
	*	@param pszCurrentBinding Command bound to this key.
	*	@return true to allow engine to process the key, otherwise, act on it as needed.
	*/
	int				( *pKeyEvent )					( const int bDown, int keynum, const char* pszCurrentBinding );

	/**
	*	Simulation and cleanup of temporary entities.
	*	@param flFrameTime Time between the last and current frame.
	*	@param flClientTime Current client time.
	*	@param flCLGravity Client side gravity.
	*	@param ppTempEntFree List of free temporary entities.
	*	@param ppTempEntActive List of active temporary entities.
	*	@param pAddVisibleEnt Callback to add visible entities.
	*	@param pTmpPlaySound Callback to play sounds for temporary entities.
	*/
	void			( *pTempEntUpdate )				( const double flFrameTime, const double flClientTime, const double flCLGravity, 
													  TEMPENTITY** ppTempEntFree, TEMPENTITY** ppTempEntActive,
													  Callback_AddVisibleEntity pAddVisibleEnt, Callback_TempEntPlaySound pTempPlaySound );

	/**
	*	If you specify negative numbers for beam start and end point entities, then
	*	the engine will call back into this function requesting a pointer to a cl_entity_t 
	*	object that describes the entity to attach the beam onto.
	*	
	*	Indices must start at 1, not zero.
	*	@param index Entity index.
	*	@return Entity.
	*/
	cl_entity_t*	( *pGetUserEntity )				( int index );

	/**
	*	Called when a player starts or stops talking.
	*	Possibly null on old client dlls.
	*	@param entindex Player index. 1 based. Is -1 when it's the local player, -2 if the server has acknowledged the local player is talking.
	*	@param bTalking Whether the player is currently talking or not.
	*/
	void			( *pVoiceStatus )				( int entindex, qboolean bTalking );

	/**
	*	Called when a director event message was received.
	*	Should be parsed like a user message.
	*	Possibly null on old client dlls.
	*	@param iSize Size of the buffer.
	*	@param pbuf Buffer.
	*/
	void			( *pDirectorMessage )			( int iSize, void *pbuf );

	/**
	*	Export this function for the engine to use the studio renderer class to render objects.
	*	Not used by all clients.
	*	@param version Interface version. Must be STUDIO_INTERFACE_VERSION. Return false if it doesn't match.
	*	@param[ out ] ppinterface Pointer to a pointer that should contain the studio interface.
	*	@param pstudio Engine studio interface.
	*	@return true if the requested interface was available, false otherwise.
	*	@see STUDIO_INTERFACE_VERSION
	*/
	int				( *pStudioInterface )			( int version, r_studio_interface_t** ppinterface, engine_studio_api_t* pstudio );

	/**
	*	Gets the chat input position.
	*	Not used by all clients.
	*	@param x X position.
	*	@param y Y position.
	*/
	void			( *pChatInputPosition )			( int *x, int *y );

	/**
	*	Doesn't appear to be called.
	*	Not used by all clients.
	*/
	int				( *pGetPlayerTeam )				( int iplayer );

	/**
	*	This should be CreateInterfaceFn but that means including interface.h
	*	which is a C++ file and some of the client files a C only... 
	*	so we return a void * which we then do a typecast on later.
	*
	*	Never actually called, but must be provided in order for CreateInterface to be called.
	*/
	void*			( *pClientFactory )				( void );
};

/**
*	@defgroup ClientExports Client Exports
*
*	Function type declarations for client exports
*	Now declares pointers in the interface and decltypes it into the pointer type, making it easier to see parameters in IntelliSense.
*
*	@{
*/

using INITIALIZE_FUNC					= decltype( cldll_func_t::pInitFunc );
using HUD_INIT_FUNC						= decltype( cldll_func_t::pHudInitFunc );
using HUD_VIDINIT_FUNC					= decltype( cldll_func_t::pHudVidInitFunc );
using HUD_REDRAW_FUNC					= decltype( cldll_func_t::pHudRedrawFunc );
using HUD_UPDATECLIENTDATA_FUNC			= decltype( cldll_func_t::pHudUpdateClientDataFunc );
using HUD_RESET_FUNC					= decltype( cldll_func_t::pHudResetFunc );
using HUD_CLIENTMOVE_FUNC				= decltype( cldll_func_t::pClientMove );
using HUD_CLIENTMOVEINIT_FUNC			= decltype( cldll_func_t::pClientMoveInit );
using HUD_TEXTURETYPE_FUNC				= decltype( cldll_func_t::pClientTextureType );
using HUD_IN_ACTIVATEMOUSE_FUNC			= decltype( cldll_func_t::pIN_ActivateMouse );
using HUD_IN_DEACTIVATEMOUSE_FUNC		= decltype( cldll_func_t::pIN_DeactivateMouse );
using HUD_IN_MOUSEEVENT_FUNC			= decltype( cldll_func_t::pIN_MouseEvent );
using HUD_IN_CLEARSTATES_FUNC			= decltype( cldll_func_t::pIN_ClearStates );
using HUD_IN_ACCUMULATE_FUNC			= decltype( cldll_func_t::pIN_Accumulate );
using HUD_CL_CREATEMOVE_FUNC			= decltype( cldll_func_t::pCL_CreateMove );
using HUD_CL_ISTHIRDPERSON_FUNC			= decltype( cldll_func_t::pCL_IsThirdPerson );
using HUD_CL_GETCAMERAOFFSETS_FUNC		= decltype( cldll_func_t::pCL_GetCameraOffsets );
using HUD_KB_FIND_FUNC					= decltype( cldll_func_t::pFindKey );
using HUD_CAMTHINK_FUNC					= decltype( cldll_func_t::pCamThink );
using HUD_CALCREF_FUNC					= decltype( cldll_func_t::pCalcRefdef );
using HUD_ADDENTITY_FUNC				= decltype( cldll_func_t::pAddEntity );
using HUD_CREATEENTITIES_FUNC			= decltype( cldll_func_t::pCreateEntities );
using HUD_DRAWNORMALTRIS_FUNC			= decltype( cldll_func_t::pDrawNormalTriangles );
using HUD_DRAWTRANSTRIS_FUNC			= decltype( cldll_func_t::pDrawTransparentTriangles );
using HUD_STUDIOEVENT_FUNC				= decltype( cldll_func_t::pStudioEvent );
using HUD_POSTRUNCMD_FUNC				= decltype( cldll_func_t::pPostRunCmd );
using HUD_SHUTDOWN_FUNC					= decltype( cldll_func_t::pShutdown );
using HUD_TXFERLOCALOVERRIDES_FUNC		= decltype( cldll_func_t::pTxferLocalOverrides );
using HUD_PROCESSPLAYERSTATE_FUNC		= decltype( cldll_func_t::pProcessPlayerState );
using HUD_TXFERPREDICTIONDATA_FUNC		= decltype( cldll_func_t::pTxferPredictionData );
using HUD_DEMOREAD_FUNC					= decltype( cldll_func_t::pReadDemoBuffer );
using HUD_CONNECTIONLESS_FUNC			= decltype( cldll_func_t::pConnectionlessPacket );
using HUD_GETHULLBOUNDS_FUNC			= decltype( cldll_func_t::pGetHullBounds );
using HUD_FRAME_FUNC					= decltype( cldll_func_t::pHudFrame );
using HUD_KEY_EVENT_FUNC				= decltype( cldll_func_t::pKeyEvent );
using HUD_TEMPENTUPDATE_FUNC			= decltype( cldll_func_t::pTempEntUpdate );
using HUD_GETUSERENTITY_FUNC			= decltype( cldll_func_t::pGetUserEntity );
using HUD_VOICESTATUS_FUNC				= decltype( cldll_func_t::pVoiceStatus );
using HUD_DIRECTORMESSAGE_FUNC			= decltype( cldll_func_t::pDirectorMessage );
using HUD_STUDIO_INTERFACE_FUNC			= decltype( cldll_func_t::pStudioInterface );
using HUD_CHATINPUTPOSITION_FUNC		= decltype( cldll_func_t::pChatInputPosition );
using HUD_GETPLAYERTEAM					= decltype( cldll_func_t::pGetPlayerTeam );
using CLIENTFACTORY						= decltype( cldll_func_t::pClientFactory );

/** @} */

// ********************************************************
// Functions exported by the engine
// ********************************************************

// Pointers to the exported engine functions themselves
struct cl_enginefunc_t
{
	/**
	*	Loads a sprite by name.
	*	A maximum of 256 HUD sprites can be loaded at the same time.
	*	@param pszPicName Name of the sprite to load. Must include the sprites directory name and the extension.
	*	@return Handle to the sprite.
	*/
	HSPRITE					( *pfnSPR_Load )							( const char* const pszPicName );

	/**
	*	Gets the number of frames in the sprite.
	*	@param hPic Handle to the sprite.
	*	@return Frame count.
	*/
	int						( *pfnSPR_Frames )							( HSPRITE hPic );

	/**
	*	Gets the height of a given sprite frame.
	*	@param hPic Handle to the sprite.
	*	@param frame Frame number.
	*	@return Height in pixels.
	*/
	int						( *pfnSPR_Height )							( HSPRITE hPic, int frame );

	/**
	*	Gets the width of a given sprite frame.
	*	@param hPic Handle to the sprite.
	*	@param frame Frame number.
	*	@return Width in pixels.
	*/
	int						( *pfnSPR_Width )							( HSPRITE hPic, int frame );

	/**
	*	Sets the sprite to draw, and its color.
	*	@param hPic Handle to the sprite.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*/
	void					( *pfnSPR_Set )								( HSPRITE hPic, int r, int g, int b );

	/**
	*	Draws the current sprite as solid.
	*	@param frame Frame to draw.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param prc Optional. Defines the rectangle of the sprite frame to draw.
	*	@see pfnSPR_Set
	*/
	void					( *pfnSPR_Draw )							( int frame, int x, int y, const wrect_t* prc );

	/**
	*	Draws the current sprite with color index255 not drawn (transparent).
	*	@param frame Frame to draw.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param prc Optional. Defines the rectangle of the sprite frame to draw.
	*	@see pfnSPR_Set
	*/
	void					( *pfnSPR_DrawHoles )						( int frame, int x, int y, const wrect_t* prc );

	/**
	*	Draws the current sprite, adds the sprites RGB values to the background (additive translucency).
	*	@param frame Frame to draw.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param prc Optional. Defines the rectangle of the sprite frame to draw.
	*	@see pfnSPR_Set
	*/
	void					( *pfnSPR_DrawAdditive )					( int frame, int x, int y, const wrect_t* prc );

	/**
	*	Dets a clipping rect for HUD sprites. (0,0) is the top-left hand corner of the screen.
	*	@param x Left coordinate of the box.
	*	@param y Top coordinate of the box.
	*	@param width Width of the box.
	*	@param height Height of the box.
	*/
	void					( *pfnSPR_EnableScissor )					( int x, int y, int width, int height );

	/**
	*	Disables the scissor box.
	*/
	void					( *pfnSPR_DisableScissor )					( void );

	/**
	*	Loads a sprite list. This is a text file defining a list of HUD elements.
	*	Free the returned list with COM_FreeFile.
	*	@param pszName Name of the file to load. Should include the sprites directory and the extension.
	*	@param[ out ] piCount Optional. Pointer to a variable that will contain the number of entries in the list.
	*	@return List of sprites.
	*/
	client_sprite_t*		( *pfnSPR_GetList )							( const char* const pszName, int* piCount );

	/**
	*	Fills the given rectangle with a given color.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param width Width of the rectangle.
	*	@param height Height of the rectangle.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param a Alpha value. [ 0, 255 ].
	*/
	void					( *pfnFillRGBA )							( int x, int y, int width, int height, int r, int g, int b, int a );

	/**
	*	Gets screen info.
	*	The SCREENINFO::iSize member must be set to sizeof( SCREENINFO ).
	*	@param psrcinfo Pointer to a SCREENINFO instance that will receive the information.
	*	@return Number of bytes that have been written. 0 if nothing was written.
	*/
	int						( *pfnGetScreenInfo ) 						( SCREENINFO* pscrinfo );

	/**
	*	Sets the crosshair sprite.
	*	@param hPic Handle to the sprite.
	*	@param rc Rectangle that defines the crosshair box to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*/
	void					( *pfnSetCrosshair )						( HSPRITE hspr, wrect_t rc, int r, int g, int b );

	/**
	*	Registers a new cvar. Avoid calling with the same name more than once.
	*	@param pszName Name of the cvar. Must point to a string that will exist for the rest of the program's lifetime.
	*	@param pszValue Value to set. Can be a temporary string.
	*	@param flags CVar flags.
	*	@return Pointer to the cvar.
	*/
	cvar_t*					( *pfnRegisterVariable )					( const char* const pszName, const char* const pszValue, int flags );

	/**
	*	Gets the float value of a cvar.
	*	@param pszName CVar name.
	*	@return Value, or 0 if the cvar doesn't exist.
	*/
	float					( *pfnGetCvarFloat )						( const char* const pszName );

	/**
	*	Gets the string value of a cvar.
	*	@param pszName CVar name.
	*	@return Value, or nullptr if the cvar doesn't exist.
	*/
	const char*				( *pfnGetCvarString )						( const char* const pszName );

	/**
	*	Adds a new command.
	*	@param pszCmdName Command name. Must point to a string that will exist for the rest of the program's lifetime.
	*	@param pCallback Callback to invoke when the command is executed. If null, forwards the command to the server.
	*	@return true in all cases.
	*/
	int						( *pfnAddCommand )							( const char* const pszCmdName, CmdFunction pCallback );

	/**
	*	Hooks a user message.
	*	@param pszMsgName Name of the message. Can be a temporary string.
	*	@param pfn Callback to invoke when the message is received.
	*	@return true if the command was already registered with the same callback, false otherwise.
	*/
	int						( *pfnHookUserMsg )							( const char* const pszMsgName, pfnUserMsgHook pfn );

	/**
	*	Sends a command to the server.
	*	@param pszCmdString Command string.
	*	@return false in all cases.
	*/
	int						( *pfnServerCmd )							( const char* const pszCmdString );

	/**
	*	Enqueues a command for execution for the local client.
	*	@param pszCmdString Command string.
	*	@return true if the command was enqueued, false otherwise.
	*/
	int						( *pfnClientCmd )							( const char* const pszCmdString );

	/**
	*	Gets player info.
	*	@param ent_num 1 based player entity index.
	*	@param pinfo Structure that will contain the player's information.
	*/
	void					( *pfnGetPlayerInfo )						( int ent_num, hud_player_info_t* pinfo );

	/**
	*	Plays a sound by name.
	*	@param pszSound Name of the sound.
	*	@param volume Volume to play at. [ 0, 1 ].
	*/
	void					( *pfnPlaySoundByName )						( const char* const pszSound, float volume );

	/**
	*	Plays a sound by index.
	*	@param iSound Index of the sound.
	*	@param volume Volume to play at. [ 0, 1 ].
	*/
	void					( *pfnPlaySoundByIndex )					( int iSound, float volume );

	/**
	*	Converts angles to directional vectors.
	*	Superseded by AngleVectors.
	*	@param vecAngles Angles.
	*	@param[ out ] forward Forward vector.
	*	@param[ out ] right Right vector.
	*	@param[ out ] up Up vector.
	*	@see AngleVectors
	*/
	void					( *pfnAngleVectors )						( const float* vecAngles, float* forward, float* right, float* up );

	/**
	*	Gets a text message, defined in titles.txt.
	*	@param pszName Text message name.
	*					Must be either a message defined in titles.txt, or one of the following:
	*					__DEMOMESSAGE__
	*					__NETMESSAGE__1
	*					__NETMESSAGE__2
	*					__NETMESSAGE__3
	*					__NETMESSAGE__4
	*	@return Text message, or null if no text message could be retrieved.
	*/
	client_textmessage_t*	( *pfnTextMessageGet )						( const char* const pszName );

	/**
	*	Draws a single character.
	*	@param x Left position.
	*	@param y Top position.
	*	@param number Character to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@return Total width of the drawn character.
	*/
	int						( *pfnDrawCharacter )						( int x, int y, int number, int r, int g, int b );

	/**
	*	Draws a string.
	*	@param x Left position.
	*	@param y Top position.
	*	@param pszString String to draw.
	*	@return Total width of the drawn string.
	*/
	int						( *pfnDrawConsoleString )					( int x, int y, const char* const pszString );

	/**
	*	Sets the text color.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*/
	void					( *pfnDrawSetTextColor )					( float r, float g, float b );

	/**
	*	Gets the length in pixels of a string if it were drawn onscreen.
	*	@param pszString String to check.
	*	@param piLength Pointer to a variable that will contain the total width of the string.
	*	@param piHeight Pointer to a variable that will contain the height of the string.
	*/
	void					( *pfnDrawConsoleStringLen )				( const char* const pszString, int* piLength, int* piHeight );

	/**
	*	Prints a string to the console.
	*	@param pszString String to print.
	*/
	void					( *pfnConsolePrint )						( const char* const pszString );

	/**
	*	Prints a string to the center of the screen.
	*	@param pszString String to print.
	*/
	void					( *pfnCenterPrint )							( const char* const pszString );

	/**
	*	@return The center of the screen's X axis.
	*/
	int						( *GetWindowCenterX )						( void );

	/**
	*	@return The center of the screen's Y axis.
	*/
	int						( *GetWindowCenterY )						( void );

	/**
	*	Gets the view angles.
	*	@param[ out ] vecAngles Will contain the view angles.
	*/
	void					( *GetViewAngles )							( float* vecAngles );

	/**
	*	Sets the view angles.
	*	@param vecAngles Angles to set.
	*/
	void					( *SetViewAngles )							( const float* vecAngles );

	/**
	*	@return The maximum number of clients that can be connected to the current server.
	*/
	int						( *GetMaxClients )							( void );

	/**
	*	Sets the value of a cvar.
	*	@param pszCVarName Name of the cvar.
	*	@param value Value to set.
	*/
	void					( *Cvar_SetValue )							( const char* const pszCVarName, float value );

	/**
	*	@return the number of arguments in the command that is currently being executed.
	*/
	int						( *Cmd_Argc )								( void );

	/**
	*	@param arg Argument index.
	*	@return Argument at the given index.
	*/
	char*					( *Cmd_Argv )								( int arg );

	/**
	*	Prints to the console.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	void					( *Con_Printf )								( const char* const pszFormat, ... );

	/**
	*	Prints to the console if developer mode is enabled.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	void					( *Con_DPrintf )							( const char* const pszFormat, ... );

	/**
	*	Prints to the notify area.
	*	@param pos Position in the notify list to set this message to. [ 0, 32 [.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	void					( *Con_NPrintf )							( const int pos, const char* const pszFormat, ... );

	/**
	*	Prints to the notify area.
	*	@param info Notify print info.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	void					( *Con_NXPrintf )							( con_nprint_t* info, char *fmt, ... );

	/**
	*	Given a key, gets the physics value.
	*	@param pszKey Key.
	*	@return Pointer to the value, or an empty string if the key couldn't be found.
	*/
	const char*				( *PhysInfo_ValueForKey )					( const char* const pszKey );

	/**
	*	Given a key, gets the server info value.
	*	@param pszKey Key.
	*	@return Pointer to the value, or an empty string if the key couldn't be found.
	*/
	const char*				( *ServerInfo_ValueForKey )					( const char* const pszKey );

	/**
	*	@return The client's maximum speed.
	*/
	float					( *GetClientMaxspeed )						( void );

	/**
	*	Checks if the given parameter was provided on the command line.
	*	@param pszParm Parameter to check.
	*	@param[ out ] ppszNext Optional. If the parameter was provided, points to the value for the given parameter.
	*	@return Parameter index in the argument vector. 0 if it wasn't found.
	*/
	int						( *CheckParm )								( const char* const pszParm, char** ppszNext );

	/**
	*	Triggers a key event.
	*	@param key Key number. @see KeyNum.
	*	@param bDown Whether the key is down or up.
	*/
	void					( *Key_Event )								( int key, const int bDown );

	/**
	*	Gets the mouse position on-screen.
	*	@param mx X position.
	*	@param my Y position.
	*/
	void					( *GetMousePosition )						( int* mx, int *my );

	/**
	*	@return Whether the player is currently noclipping.
	*/
	int						( *IsNoClipping )							( void );

	/**
	*	Note: do not call until a map has been loaded. Will access invalid memory otherwise, and return a garbage pointer.
	*	Will be valid if called after HUD_VidInit has returned.
	*	@return The entity that represents the local player.
	*/
	cl_entity_t*			( *GetLocalPlayer )							( void );

	/**
	*	@return The entity that represents the player's viewmodel.
	*/
	cl_entity_t*			( *GetViewModel )							( void );

	/**
	*	Gets an entity by index. Note: do not call until a map has been loaded. Will return a null pointer otherwise.
	*	Will be valid if called after HUD_VidInit has returned.
	*	@param idx Index. 0 based.
	*	@return The entity, or null if the index is invalid.
	*/
	cl_entity_t*			( *GetEntityByIndex )						( int idx );

	/**
	*	@return Current client time.
	*/
	float					( *GetClientTime )							( void );

	/**
	*	Calculates the current shake settings.
	*/
	void					( *V_CalcShake )							( void );

	/**
	*	Applies the shake settings.
	*	@param[ in, out ] vecOrigin Original origin. Will contain the new origin.
	*	@param[ in, out ] vecAngles Original angles. Will contain the new angles.
	*	@param flFactor Factor by which to multiply the shake.
	*/
	void					( *V_ApplyShake )							( float* vecOrigin, float* vecAngles, const float flFactor );

	/**
	*	Gets the contents of the given point.
	*	The real contents can contain water current data.
	*	@param vecPoint Point to check.
	*	@param piTruecontents The real contents.
	*	@return Contents.
	*/
	int						( *PM_PointContents )						( const float* vecPoint, int* piTruecontents );

	/**
	*	Gets the index of the water entity at the given position.
	*	@param vecPosition Position to look for the entity.
	*	@return Entity index. -1 if no water entity was found.
	*/
	int						( *PM_WaterEntity )							( const float* vecPosition );

	/**
	*	Performs a traceline.
	*	@param vecStart Starting point.
	*	@param vecEnd End point.
	*	@param flags Flags.
	*	@param usehull Hull to use. @see Hull::Hull
	*	@param ignore_pe Index of the entity to ignore. -1 if none should be ignored.
	*	@return Pointer to a statically allocated trace result instance.
	*/
	pmtrace_t*				( *PM_TraceLine )							( const float* vecStart, const float* vecEnd, int flags, int usehull, int ignore_pe );

	/**
	*	Loads a model.
	*	@param pszModelName Name of the model to load. Starts in the game directory, must include the extension.
	*	@param[ out ] piIndex Optional. Will contain the index of the model. -1 if loading failed.
	*	@return Pointer to the model.
	*/
	model_t*				( *CL_LoadModel )							( const char* const pszModelName, int* piIndex );

	/**
	*	Creates a new visible entity.
	*	@param type Entity type. @see EntityType
	*	@param ent Entity.
	*	@return true if the entity was successfully added, false otherwise.
	*/
	int						( *CL_CreateVisibleEntity )					( int type, cl_entity_t* ent );

	/**
	*	Gets the model that is represented by the given sprite handle.
	*	@param hSprite Handle to the sprite.
	*	@return Pointer to the model, or null if the handle is invalid.
	*/
	const model_t*			( *GetSpritePointer )						( HSPRITE hSprite );

	/**
	*	Plays a sound by name at a given location.
	*	@param pszSoundName Name of the sound.
	*	@param volume Sound volume. [ 0, 1 ].
	*	@param vecOrigin Location where the sound should be played.
	*/
	void					( *pfnPlaySoundByNameAtLocation )			( const char* const pszSoundName, float volume, const float* vecOrigin );

	/**
	*	Precaches an event.
	*	@param type Type. Must be 1.
	*	@param pszName Name of the event.
	*	@return Event index, or 0 if the event couldn't be found.
	*/
	unsigned short			( *pfnPrecacheEvent )						( int type, const char* const pszName );

	/**
	*	@param flags Event flags.
	*	@param pInvoker Client that triggered the event.
	*	@param eventindex Event index. @see pfnPrecacheEvent
	*	@param delay Delay before the event should be run.
	*	@param origin If not g_vecZero, this is the origin parameter sent to the clients.
	*	@param angles If not g_vecZero, this is the angles parameter sent to the clients.
	*	@param fparam1 Float parameter 1.
	*	@param fparam2 Float parameter 2.
	*	@param iparam1 Integer parameter 1.
	*	@param iparam2 Integer parameter 2.
	*	@param bparam1 Boolean parameter 1.
	*	@param bparam2 Boolean parameter 2.
	*/
	void					( *pfnPlaybackEvent )						( int flags, const edict_t* pInvoker, unsigned short eventindex, float delay, 
																		  const float* origin, const float* angles, 
																		  float fparam1, float fparam2, 
																		  int iparam1, int iparam2, 
																		  int bparam1, int bparam2 );

	/**
	*	Sets the weapon animation and body.
	*	@param iAnim Animation index.
	*	@param body Body to set.
	*/
	void					( *pfnWeaponAnim )							( int iAnim, int body );

	/**
	*	Generates a random float number in the range [ flLow, flLow ].
	*	@param flLow Lower bound.
	*	@param flHigh Higher bound.
	*	@return Random number.
	*/
	float					( *pfnRandomFloat )							( float flLow, float flHigh );

	/**
	*	Generates a random long number in the range [ lLow, lHigh ].
	*	@param lLow Lower bound.
	*	@param lHigh Higher bound.
	*	@return Random number, or lLow if lHigh is smaller than or equal to lLow.
	*/
	int32					( *pfnRandomLong )							( int32 lLow, int32 lHigh );

	/**
	*	Adds a hook for an event.
	*	@param pszName Name of the event.
	*	@param pEventHook Hook to invoke when the event is triggered.
	*/
	void					( *pfnHookEvent )							( const char* const pszName, pfnEventHook pEventHook );

	/**
	*	@return Whether the console is currently visible.
	*/
	int						( *Con_IsVisible )							( void );

	/**
	*	@return Name of the game/mod directory.
	*/
	const char*				( *pfnGetGameDirectory )					( void );

	/**
	*	Gets a cvar by name.
	*	@param pszName Name of the cvar.
	*	@return Pointer to the cvar, or null if it couldn't be found.
	*/
	cvar_t*					( *pfnGetCvarPointer )						( const char* const pszName );

	/**
	*	Gets the name of the key that is bound to the given command.
	*	@param pszBinding Command.
	*	@return Key name, or "<UNKNOWN KEYNUM>" if it couldn't be found.
	*/
	const char*				( *Key_LookupBinding )						( const char* const pszBinding );

	/**
	*	@return The name of the level that is currently loaded. Has the format "maps/%s.bsp", where %s is the level name.
	*/
	const char*				( *pfnGetLevelName )						( void );

	/**
	*	Gets the current screen fade settings.
	*	@param fade Structure that will contain the result.
	*/
	void					( *pfnGetScreenFade )						( screenfade_t* fade );

	/**
	*	Sets the current screen fade settings.
	*	@param fade Structure that contains the new settings.
	*/
	void					( *pfnSetScreenFade )						( const screenfade_t* fade );

	/**
	*	@return The root VGUI1 panel to use for the viewport.
	*/
	void*					( *VGui_GetPanel )							( void );

	/**
	*	Paints the VGUI1 viewport background.
	*	Only safe to call from inside subclass of Panel::paintBackground.
	*	@param extents Viewport extents. Contains x1 y1 x2 y2 coordinates.
	*/
	void					( *VGui_ViewportPaintBackground )			( int extents[ 4 ] );

	/**
	*	Loads a file.
	*	@param pszPath Path to the file.
	*	@param usehunk Hunk to use. Must always be 5.
	*	@param[ out ] piLength Optional. Length of the file, in bytes. 0 if the file couldn't be loaded.
	*	@return Pointer to buffer, or null if the file couldn't be loaded.
	*/
	byte*					( *COM_LoadFile )							( const char* const pszPath, int usehunk, int* piLength );

	/**
	*	Parses the given data.
	*	@param pszData Data to parse.
	*	@param[ out ] pszToken Destination buffer for the token. Should be at least 1024 characters large.
	*	@return Pointer to the next character to parse.
	*/
	const char*				( *COM_ParseFile )							( const char* pszData, char* pszToken );

	/**
	*	Frees the given buffer. Calls free() on it.
	*	@param pBuffer Buffer to free. Can be null, in which case nothing is done.
	*/
	void					( *COM_FreeFile )							( void* pBuffer );

	/**
	*	Triangle API. Used to draw 3D geometry.
	*/
	triangleapi_t*		pTriAPI;

	/**
	*	Effects API.
	*/
	efx_api_t*			pEfxAPI;

	/**
	*	Event API.
	*/
	event_api_t*		pEventAPI;

	/**
	*	Demo API.
	*/
	demo_api_t*			pDemoAPI;

	/**
	*	Networking API.
	*/
	net_api_t*			pNetAPI;

	/**
	*	Voice Tweak API.
	*/
	IVoiceTweak*		pVoiceTweak;

	/**
	*	@return Whether this client is in spectator only mode (HLTV).
	*/
	int						 ( *IsSpectateOnly )						( void );

	/**
	*	Loads a map sprite. Either a TGA or BMP file is required.
	*	@param pszFileName Name of the file.
	*	@return Pointer to the model, or null if the model could not be loaded.
	*/
	model_t*				 ( *LoadMapSprite )							( const char* pszFileName );

	/**
	*	Adds a directory to the PLATFORM search path.
	*	@param pszBaseDir Directory to add.
	*	@param pszAppName Ignored.
	*/
	void					 ( *COM_AddAppDirectoryToSearchPath )		( const char* const pszBaseDir, const char* const pszAppName );

	/**
	*	Converts a relative path to an absolute path.
	*	@param pszFileName Name of the file whose path to make absolute.
	*	@param[ out ] pszNameOutBuffer Destination buffer.
	*	@param nameOutBufferSize Size of the destination buffer, in bytes.
	*/
	int						 ( *COM_ExpandFilename )					( const char* const pszFileName, char* pszNameOutBuffer, int nameOutBufferSize );

	/**
	*	Given a key, gets the info value.
	*	@param playerNum Player number. 1 based.
	*	@param pszKey Key.
	*	@return Pointer to the value, or an empty string if the key couldn't be found.
	*/
	const char*				 ( *PlayerInfo_ValueForKey )				( int playerNum, const char *key );

	/**
	*	Sets the value for a key in the local player's info key buffer.
	*	@param pszKey Key whose value to set.
	*	@param pszValue Value to set.
	*/
	void					 ( *PlayerInfo_SetValueForKey )				( const char* const pszKey, const char* const pszValue );

	/**
	*	Gets the given player's unique ID.
	*	@param iPlayer 1 based player index.
	*	@param[ out ] playerID Will contain the player's unique ID.
	*	@return true on success, false otherwise.
	*/
	qboolean				 ( *GetPlayerUniqueID )						( int iPlayer, char playerID[ PLAYERID_BUFFER_SIZE ] );

	/**
	*	Obsolete.
	*	@return 0 in all cases.
	*/
	int						 ( *GetTrackerIDForPlayer )					( int playerSlot );

	/**
	*	Obsolete.
	*	@return 0 in all cases.
	*/
	int						 ( *GetPlayerForTrackerID )					( int trackerID );

	/**
	*	Sends a command to the server unreliably.
	*	@param pszCmdString Command string.
	*	@return true if the message was written, false otherwise.
	*/
	int						 ( *pfnServerCmdUnreliable )				( const char* const pszCmdString );

	/**
	*	Gets the mouse position.
	*	@param ppt Structure that will contain the mouse position.
	*/
	void					 ( *pfnGetMousePos )						( POINT* ppt );

	/**
	*	Sets the mouse position.
	*	@param x X position.
	*	@param y Y position.
	*/
	void					 ( *pfnSetMousePos )						( int x, int y );

	/**
	*	Obsolete.
	*/
	void					 ( *pfnSetMouseEnable )						( qboolean fEnable );

	/**
	*	@return The first cvar in the list.
	*/
	cvar_t*					 ( *GetFirstCvarPtr )						( void );

	/**
	*	@return The first command function handle.
	*/
	cmd_function_t*			 ( *GetFirstCmdFunctionHandle )				( void );

	/**
	*	Gets the next command function handle.
	*	@param cmdhandle Handle to the command function just before the handle to get.
	*	@return Next handle, or 0 if it was the last handle.
	*/
	cmd_function_t*			 ( *GetNextCmdFunctionHandle )				( cmd_function_t* cmdhandle );

	/**
	*	Gets the command function name.
	*	@param cmdhandle Handle to the command.
	*	@return Command name.
	*/
	const char*				( *GetCmdFunctionName )					( cmd_function_t* cmdhandle );

	/**
	*	@return The old client time.
	*/
	float					 ( *hudGetClientOldTime )					( void );

	/**
	*	@return Server gravity value. Only valid if this is a listen server.
	*/
	float					 ( *hudGetServerGravityValue )				( void );

	/**
	*	Gets a model by index.
	*	@param index Model index. Must be valid.
	*	@return Model pointer.
	*/
	model_t*				 ( *hudGetModelByIndex )					( int index );

	/**
	*	Sets the filter mode.
	*	Filtering will essentially overlay a single color on top of the entire viewport, with the exception of the HUD.
	*	This works like Counter-Strike's night vision (though that uses a different implementation).
	*	@param bMode Whether to filter or not.
	*/
	void					 ( *pfnSetFilterMode )						( int bMode );

	/**
	*	Sets the filter color.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*/
	void					 ( *pfnSetFilterColor )						( float r, float g, float b );

	/**
	*	Sets the filter brightness.
	*	@param brightness Brightness.
	*/
	void					 ( *pfnSetFilterBrightness )				( float brightness );

	/**
	*	Gets the sequence that has the given entry name.
	*	@param pszFileName Ignored.
	*	@param pszEntryName Entry name.
	*	@return Sequence, or null if no such sequence exists.
	*/
	sequenceEntry_t*		 ( *pfnSequenceGet )						( const char* const pszFileName, const char* const pszEntryName );

	/**
	*	Draws the current sprite as solid.
	*	@param frame Frame to draw.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param prc Optional. Defines the rectangle of the sprite frame to draw.
	*	@param src glBlendFunc source value.
	*	@param dest glBlendFunc destination value.
	*	@param w Overrides the sprite frame's width.
	*	@param h Overrides the sprite frame's height.
	*	@see pfnSPR_Set
	*/
	void					 ( *pfnSPR_DrawGeneric )					( int frame, int x, int y, const wrect_t* prc, int src, int dest, int w, int h );

	/**
	*	Picks a sentence from the given group.
	*	@param pszGroupName Group from which to select a sentence.
	*	@param pickMethod Ignored.
	*	@param piPicked If not null, this is set to the index of the sentence that was picked.
	*	@return Sentence that was picked, or null if there is no group by that name, or no sentences in the group.
	*/
	sentenceEntry_t*		 ( *pfnSequencePickSentence )				( const char* const pszGroupName, int pickMethod, int* piPicked );
	
	/**
	*	Draws a complete string.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param pszString String to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@return Total width of the string.
	*/
	int						 ( *pfnDrawString )							( int x, int y, const char* const pszString, int r, int g, int b );

	/**
	*	Draws a complete string in reverse.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param pszString String to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@return Total width of the string.
	*/
	int						 ( *pfnDrawStringReverse )					( int x, int y, const char* const pszString, int r, int g, int b );

	/**
	*	Given a key, gets the info value for the local player.
	*	@param pszKey Key.
	*	@return Pointer to the value, or an empty string if the key couldn't be found.
	*/
	const char*				 ( *LocalPlayerInfo_ValueForKey )			( const char* const pszKey );

	/**
	*	Draws a single character using the specified font.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param ch Character to draw.
	*	@param font Font to use.
	*	@return Total width of the character.
	*/
	int						 ( *pfnVGUI2DrawCharacter )					( int x, int y, int ch, unsigned int font );

	/**
	*	Draws a single character using the specified font, using additive rendering.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param ch Character to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param font Font to use.
	*	@return Total width of the character.
	*/
	int						 ( *pfnVGUI2DrawCharacterAdd )				( int x, int y, int ch, int r, int g, int b, unsigned int font );

	/**
	*	Gets the approximate wave play length of the given file.
	*	@param pszFileName Name of the file to query.
	*	@return Approximate wave play length.
	*/
	unsigned int			 ( *COM_GetApproxWavePlayLength )			( const char* const pszFileName );

	/**
	*	Gets the career UI, if it exists.
	*	@return Career UI. Cast to ICareerUI*. Can be null.
	*/
	void*					 ( *pfnGetCareerUI )						( void );

	/**
	*	Sets a cvar's string value.
	*	@param pszCVarName CVar name.
	*	@param pszValue Value to set.
	*/
	void					 ( *Cvar_Set )								( const char* const pszCVarName, const char* const pszValue );

	/**
	*	@return Whether this is a Condition Zero career match.
	*/
	int						 ( *pfnIsCareerMatch )						( void );

	/**
	*	Plays a sound by name, with pitch. Uses the bot channel CHAN_BOT.
	*	@param pszSoundName Name of the sound to play.
	*	@param volume Volume. [ 0, 1 ].
	*	@param pitch Pitch. [ 0, 255 ].
	*/
	void					 ( *pfnPlaySoundVoiceByName )				( const char* const pszSoundName, float volume, int pitch );

	/**
	*	Sets a music track to play.
	*	@param pszFileName Name of the file to play.
	*	@param bLooping Whether the track should look or not.
	*/
	void					 ( *pfnPrimeMusicStream )					( const char* const pszFileName, const int bLooping );

	/**
	*	@return The absolute time since the last call to GetAbsoluteTime.
	*/
	double					 ( *GetAbsoluteTime )						( void );

	/**
	*	Processes the tutor message decay buffer.
	*	@param pBuffer Buffer.
	*	@param bufferLength Size of the buffer, in bytes.
	*/
	void					 ( *pfnProcessTutorMessageDecayBuffer )		( int* pBuffer, int bufferLength );

	/**
	*	Constructs the tutor message decay buffer.
	*	@param pBuffer Buffer.
	*	@param bufferLength Size of the buffer, in bytes.
	*/
	void					 ( *pfnConstructTutorMessageDecayBuffer )	( int* pBuffer, int bufferLength );

	/**
	*	Resets tutor message decay data.
	*/
	void					 ( *pfnResetTutorMessageDecayData )			( void );

	/**
	*	Plays a sound by name, with pitch.
	*	@param pszSoundName Name of the sound to play.
	*	@param volume Volume. [ 0, 1 ].
	*	@param pitch Pitch. [ 0, 255 ].
	*/
	void					 ( *pfnPlaySoundByNameAtPitch )				( const char* const pszSoundName, float volume, int pitch );

	/**
	*	Fills the given rectangle with a given color.
	*	Blends with existing pixel data.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param width Width of the rectangle.
	*	@param height Height of the rectangle.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param a Alpha value. [ 0, 255 ].
	*/
	void					 ( *pfnFillRGBABlend )						( int x, int y, int width, int height, int r, int g, int b, int a );

	/**
	*	@return The app ID.
	*/
	int						 ( *pfnGetAppID )							( void );

	/**
	*	@return The list of command aliases.
	*/
	cmdalias_t*				 ( *pfnGetAliasList )						( void );

	/**
	*	Gets the accumulated mouse delta. The delta is reset in this call only.
	*	@param x X offset.
	*	@param y Y offset.
	*/
	void					 ( *pfnVguiWrap2_GetMouseDelta )			( int* x, int* y );
};

/**
*	@defgroup ClientEngineExports Client Engine Exports
*
*	Function type declarations for engine exports
*	Now declares pointers in the interface and decltypes it into the pointer type, making it easier to see parameters in IntelliSense.
*
*	@{
*/

using pfnEngSrc_pfnSPR_Load_t							= decltype( cl_enginefunc_t::pfnSPR_Load );
using pfnEngSrc_pfnSPR_Frames_t							= decltype( cl_enginefunc_t::pfnSPR_Frames );
using pfnEngSrc_pfnSPR_Height_t							= decltype( cl_enginefunc_t::pfnSPR_Height );
using pfnEngSrc_pfnSPR_Width_t							= decltype( cl_enginefunc_t::pfnSPR_Width );
using pfnEngSrc_pfnSPR_Set_t							= decltype( cl_enginefunc_t::pfnSPR_Set );
using pfnEngSrc_pfnSPR_Draw_t							= decltype( cl_enginefunc_t::pfnSPR_Draw );
using pfnEngSrc_pfnSPR_DrawHoles_t						= decltype( cl_enginefunc_t::pfnSPR_DrawHoles );
using pfnEngSrc_pfnSPR_DrawAdditive_t					= decltype( cl_enginefunc_t::pfnSPR_DrawAdditive );
using pfnEngSrc_pfnSPR_EnableScissor_t					= decltype( cl_enginefunc_t::pfnSPR_EnableScissor );
using pfnEngSrc_pfnSPR_DisableScissor_t					= decltype( cl_enginefunc_t::pfnSPR_DisableScissor );
using pfnEngSrc_pfnSPR_GetList_t						= decltype( cl_enginefunc_t::pfnSPR_GetList );
using pfnEngSrc_pfnFillRGBA_t							= decltype( cl_enginefunc_t::pfnFillRGBA );
using pfnEngSrc_pfnGetScreenInfo_t						= decltype( cl_enginefunc_t::pfnGetScreenInfo );
using pfnEngSrc_pfnSetCrosshair_t						= decltype( cl_enginefunc_t::pfnSetCrosshair );
using pfnEngSrc_pfnRegisterVariable_t					= decltype( cl_enginefunc_t::pfnRegisterVariable );
using pfnEngSrc_pfnGetCvarFloat_t						= decltype( cl_enginefunc_t::pfnGetCvarFloat );
using pfnEngSrc_pfnGetCvarString_t						= decltype( cl_enginefunc_t::pfnGetCvarString );
using pfnEngSrc_pfnAddCommand_t							= decltype( cl_enginefunc_t::pfnAddCommand );
using pfnEngSrc_pfnHookUserMsg_t						= decltype( cl_enginefunc_t::pfnHookUserMsg );
using pfnEngSrc_pfnServerCmd_t							= decltype( cl_enginefunc_t::pfnServerCmd );
using pfnEngSrc_pfnClientCmd_t							= decltype( cl_enginefunc_t::pfnClientCmd );
using pfnEngSrc_pfnGetPlayerInfo_t						= decltype( cl_enginefunc_t::pfnGetPlayerInfo );
using pfnEngSrc_pfnPlaySoundByName_t					= decltype( cl_enginefunc_t::pfnPlaySoundByName );
using pfnEngSrc_pfnPlaySoundByIndex_t					= decltype( cl_enginefunc_t::pfnPlaySoundByIndex );
using pfnEngSrc_pfnAngleVectors_t						= decltype( cl_enginefunc_t::pfnAngleVectors );
using pfnEngSrc_pfnTextMessageGet_t						= decltype( cl_enginefunc_t::pfnTextMessageGet );
using pfnEngSrc_pfnDrawCharacter_t						= decltype( cl_enginefunc_t::pfnDrawCharacter );
using pfnEngSrc_pfnDrawConsoleString_t					= decltype( cl_enginefunc_t::pfnDrawConsoleString );
using pfnEngSrc_pfnDrawSetTextColor_t					= decltype( cl_enginefunc_t::pfnDrawSetTextColor );
using pfnEngSrc_pfnDrawConsoleStringLen_t				= decltype( cl_enginefunc_t::pfnDrawConsoleStringLen );
using pfnEngSrc_pfnConsolePrint_t						= decltype( cl_enginefunc_t::pfnConsolePrint );
using pfnEngSrc_pfnCenterPrint_t						= decltype( cl_enginefunc_t::pfnCenterPrint );
using pfnEngSrc_GetWindowCenterX_t						= decltype( cl_enginefunc_t::GetWindowCenterX );
using pfnEngSrc_GetWindowCenterY_t						= decltype( cl_enginefunc_t::GetWindowCenterY );
using pfnEngSrc_GetViewAngles_t							= decltype( cl_enginefunc_t::GetViewAngles );
using pfnEngSrc_SetViewAngles_t							= decltype( cl_enginefunc_t::SetViewAngles );
using pfnEngSrc_GetMaxClients_t							= decltype( cl_enginefunc_t::GetMaxClients );
using pfnEngSrc_Cvar_SetValue_t							= decltype( cl_enginefunc_t::Cvar_SetValue );
using pfnEngSrc_Cmd_Argc_t								= decltype( cl_enginefunc_t::Cmd_Argc );
using pfnEngSrc_Cmd_Argv_t								= decltype( cl_enginefunc_t::Cmd_Argv );
using pfnEngSrc_Con_Printf_t							= decltype( cl_enginefunc_t::Con_Printf );
using pfnEngSrc_Con_DPrintf_t							= decltype( cl_enginefunc_t::Con_DPrintf );
using pfnEngSrc_Con_NPrintf_t							= decltype( cl_enginefunc_t::Con_NPrintf );
using pfnEngSrc_Con_NXPrintf_t							= decltype( cl_enginefunc_t::Con_NXPrintf );
using pfnEngSrc_PhysInfo_ValueForKey_t					= decltype( cl_enginefunc_t::PhysInfo_ValueForKey );
using pfnEngSrc_ServerInfo_ValueForKey_t				= decltype( cl_enginefunc_t::ServerInfo_ValueForKey );
using pfnEngSrc_GetClientMaxspeed_t						= decltype( cl_enginefunc_t::GetClientMaxspeed );
using pfnEngSrc_CheckParm_t								= decltype( cl_enginefunc_t::CheckParm );
using pfnEngSrc_Key_Event_t								= decltype( cl_enginefunc_t::Key_Event );
using pfnEngSrc_GetMousePosition_t						= decltype( cl_enginefunc_t::GetMousePosition );
using pfnEngSrc_IsNoClipping_t							= decltype( cl_enginefunc_t::IsNoClipping );
using pfnEngSrc_GetLocalPlayer_t						= decltype( cl_enginefunc_t::GetLocalPlayer );
using pfnEngSrc_GetViewModel_t							= decltype( cl_enginefunc_t::GetViewModel );
using pfnEngSrc_GetEntityByIndex_t						= decltype( cl_enginefunc_t::GetEntityByIndex );
using pfnEngSrc_GetClientTime_t							= decltype( cl_enginefunc_t::GetClientTime );
using pfnEngSrc_V_CalcShake_t							= decltype( cl_enginefunc_t::V_CalcShake );
using pfnEngSrc_V_ApplyShake_t							= decltype( cl_enginefunc_t::V_ApplyShake );
using pfnEngSrc_PM_PointContents_t						= decltype( cl_enginefunc_t::PM_PointContents );

using pfnEngSrc_PM_WaterEntity_t						= decltype( cl_enginefunc_t::PM_WaterEntity );
using pfnEngSrc_PM_TraceLine_t							= decltype( cl_enginefunc_t::PM_TraceLine );
using pfnEngSrc_CL_LoadModel_t							= decltype( cl_enginefunc_t::CL_LoadModel );
using pfnEngSrc_CL_CreateVisibleEntity_t				= decltype( cl_enginefunc_t::CL_CreateVisibleEntity );
using pfnEngSrc_GetSpritePointer_t						= decltype( cl_enginefunc_t::GetSpritePointer );
using pfnEngSrc_pfnPlaySoundByNameAtLocation_t			= decltype( cl_enginefunc_t::pfnPlaySoundByNameAtLocation );
using pfnEngSrc_pfnPrecacheEvent_t						= decltype( cl_enginefunc_t::pfnPrecacheEvent );
using pfnEngSrc_pfnPlaybackEvent_t						= decltype( cl_enginefunc_t::pfnPlaybackEvent );
using pfnEngSrc_pfnWeaponAnim_t							= decltype( cl_enginefunc_t::pfnWeaponAnim );
using pfnEngSrc_pfnRandomFloat_t						= decltype( cl_enginefunc_t::pfnRandomFloat );
using pfnEngSrc_pfnRandomLong_t							= decltype( cl_enginefunc_t::pfnRandomLong );
using pfnEngSrc_pfnHookEvent_t							= decltype( cl_enginefunc_t::pfnHookEvent );
using pfnEngSrc_Con_IsVisible_t							= decltype( cl_enginefunc_t::Con_IsVisible );
using pfnEngSrc_pfnGetGameDirectory_t					= decltype( cl_enginefunc_t::pfnGetGameDirectory );
using pfnEngSrc_pfnGetCvarPointer_t						= decltype( cl_enginefunc_t::pfnGetCvarPointer );
using pfnEngSrc_Key_LookupBinding_t						= decltype( cl_enginefunc_t::Key_LookupBinding );
using pfnEngSrc_pfnGetLevelName_t						= decltype( cl_enginefunc_t::pfnGetLevelName );
using pfnEngSrc_pfnGetScreenFade_t						= decltype( cl_enginefunc_t::pfnGetScreenFade );
using pfnEngSrc_pfnSetScreenFade_t						= decltype( cl_enginefunc_t::pfnSetScreenFade );
using pfnEngSrc_VGui_GetPanel_t							= decltype( cl_enginefunc_t::VGui_GetPanel );
using pfnEngSrc_VGui_ViewportPaintBackground_t			= decltype( cl_enginefunc_t::VGui_ViewportPaintBackground );
using pfnEngSrc_COM_LoadFile_t							= decltype( cl_enginefunc_t::COM_LoadFile );
using pfnEngSrc_COM_ParseFile_t							= decltype( cl_enginefunc_t::COM_ParseFile );
using pfnEngSrc_COM_FreeFile_t							= decltype( cl_enginefunc_t::COM_FreeFile );

using pfnEngSrc_IsSpectateOnly_t						= decltype( cl_enginefunc_t::IsSpectateOnly );
using pfnEngSrc_LoadMapSprite_t							= decltype( cl_enginefunc_t::LoadMapSprite );
using pfnEngSrc_COM_AddAppDirectoryToSearchPath_t		= decltype( cl_enginefunc_t::COM_AddAppDirectoryToSearchPath );
using pfnEngSrc_COM_ExpandFilename_t					= decltype( cl_enginefunc_t::COM_ExpandFilename );
using pfnEngSrc_PlayerInfo_ValueForKey_t				= decltype( cl_enginefunc_t::PlayerInfo_ValueForKey );
using pfnEngSrc_PlayerInfo_SetValueForKey_t				= decltype( cl_enginefunc_t::PlayerInfo_SetValueForKey );
using pfnEngSrc_GetPlayerUniqueID_t						= decltype( cl_enginefunc_t::GetPlayerUniqueID );
using pfnEngSrc_GetTrackerIDForPlayer_t					= decltype( cl_enginefunc_t::GetTrackerIDForPlayer );
using pfnEngSrc_GetPlayerForTrackerID_t					= decltype( cl_enginefunc_t::GetPlayerForTrackerID );
using pfnEngSrc_pfnServerCmdUnreliable_t				= decltype( cl_enginefunc_t::pfnServerCmdUnreliable );
using pfnEngSrc_GetMousePos_t							= decltype( cl_enginefunc_t::pfnGetMousePos );
using pfnEngSrc_SetMousePos_t							= decltype( cl_enginefunc_t::pfnSetMousePos );
using pfnEngSrc_SetMouseEnable_t						= decltype( cl_enginefunc_t::pfnSetMouseEnable );
using pfnEngSrc_GetFirstCVarPtr_t						= decltype( cl_enginefunc_t::GetFirstCvarPtr );
using pfnEngSrc_GetFirstCmdFunctionHandle_t				= decltype( cl_enginefunc_t::GetFirstCmdFunctionHandle );
using pfnEngSrc_GetNextCmdFunctionHandle_t				= decltype( cl_enginefunc_t::GetNextCmdFunctionHandle );
using pfnEngSrc_GetCmdFunctionName_t					= decltype( cl_enginefunc_t::GetCmdFunctionName );
using pfnEngSrc_GetClientOldTime_t						= decltype( cl_enginefunc_t::hudGetClientOldTime );
using pfnEngSrc_GetServerGravityValue_t					= decltype( cl_enginefunc_t::hudGetServerGravityValue );

using pfnEngSrc_GetModelByIndex_t						= decltype( cl_enginefunc_t::hudGetModelByIndex );
using pfnEngSrc_pfnSetFilterMode_t						= decltype( cl_enginefunc_t::pfnSetFilterMode );
using pfnEngSrc_pfnSetFilterColor_t						= decltype( cl_enginefunc_t::pfnSetFilterColor );
using pfnEngSrc_pfnSetFilterBrightness_t				= decltype( cl_enginefunc_t::pfnSetFilterBrightness );
using pfnEngSrc_pfnSequenceGet_t						= decltype( cl_enginefunc_t::pfnSequenceGet );
using pfnEngSrc_pfnSPR_DrawGeneric_t					= decltype( cl_enginefunc_t::pfnSPR_DrawGeneric );
using pfnEngSrc_pfnSequencePickSentence_t				= decltype( cl_enginefunc_t::pfnSequencePickSentence );
using pfnEngSrc_pfnDrawString_t							= decltype( cl_enginefunc_t::pfnDrawString );
using pfnEngSrc_pfnDrawStringReverse_t					= decltype( cl_enginefunc_t::pfnDrawStringReverse );
using pfnEngSrc_LocalPlayerInfo_ValueForKey_t			= decltype( cl_enginefunc_t::LocalPlayerInfo_ValueForKey );
using pfnEngSrc_pfnVGUI2DrawCharacter_t					= decltype( cl_enginefunc_t::pfnVGUI2DrawCharacter );
using pfnEngSrc_pfnVGUI2DrawCharacterAdd_t				= decltype( cl_enginefunc_t::pfnVGUI2DrawCharacterAdd );
using pfnEngSrc_COM_GetApproxWavePlayLength				= decltype( cl_enginefunc_t::COM_GetApproxWavePlayLength );
using pfnEngSrc_pfnGetCareerUI_t						= decltype( cl_enginefunc_t::pfnGetCareerUI );
using pfnEngSrc_Cvar_Set_t								= decltype( cl_enginefunc_t::Cvar_Set );
using pfnEngSrc_pfnIsPlayingCareerMatch_t				= decltype( cl_enginefunc_t::pfnIsCareerMatch );
using pfnEngSrc_pfnPlaySoundVoiceByName_t				= decltype( cl_enginefunc_t::pfnPlaySoundVoiceByName );
using pfnEngSrc_pfnPrimeMusicStream_t					= decltype( cl_enginefunc_t::pfnPrimeMusicStream );
using pfnEngSrc_GetAbsoluteTime_t						= decltype( cl_enginefunc_t::GetAbsoluteTime );
using pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t		= decltype( cl_enginefunc_t::pfnProcessTutorMessageDecayBuffer );
using pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t	= decltype( cl_enginefunc_t::pfnConstructTutorMessageDecayBuffer );
using pfnEngSrc_pfnResetTutorMessageDecayData_t			= decltype( cl_enginefunc_t::pfnResetTutorMessageDecayData );
using pfnEngSrc_pfnPlaySoundByNameAtPitch_t				= decltype( cl_enginefunc_t::pfnPlaySoundByNameAtPitch );
using pfnEngSrc_pfnFillRGBABlend_t						= decltype( cl_enginefunc_t::pfnFillRGBABlend );
using pfnEngSrc_pfnGetAppID_t							= decltype( cl_enginefunc_t::pfnGetAppID );
using pfnEngSrc_pfnGetAliases_t							= decltype( cl_enginefunc_t::pfnGetAliasList );
using pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t			= decltype( cl_enginefunc_t::pfnVguiWrap2_GetMouseDelta );

/** @} */

#endif // __APIPROXY__
