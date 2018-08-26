#ifndef ENGINE_SERVER_SERVER_H
#define ENGINE_SERVER_SERVER_H

/**
*	Reverse engineered engine data structures.
*/

struct consistency_t
{
	const char* filename;
	int issound;
	int orig_index;
	int value;
	int check_type;
	Vector mins;
	Vector maxs;
};

struct event_t
{
	short index;
	byte padding[ 2 ];

	const char* filename;
	int filesize;
	const char* pszScript;
};

enum server_state_t
{
	ss_loading,
	ss_active
};

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;

#define MAX_LIGHTSTYLES 64

/**
*	For ForceUnmodified
*/
#define MAX_CONSISTENCY 512
#define MAX_MODELS 512
#define MAX_EVENTS 256
#define MAX_SOUNDS 512
#define MAX_GENERIC 512
/**
*	Player spray paint.
*/
#define MAX_DECALS 256

#define MAX_RESOURCES ( MAX_MODELS + MAX_SOUNDS + MAX_DECALS )

#define MAX_DATAGRAM 4000

#define MAX_MULTICAST 1024

#define MAX_SIGNON 32768

/**
*	Reverse engineered from the engine. This is the server data structure that contains a bunch of state. - Solokiller
*/
struct server_t
{
	qboolean active;
	qboolean paused;
	qboolean loadgame;

	double time;
	double oldtime;

	int lastcheck; // number of last checked client
	double lastchecktime;

	char name[ MAX_QPATH ];
	char oldname[ MAX_QPATH ];
	char startspot[ MAX_QPATH ];
	char modelname[ MAX_QPATH ];

	/**
	*	This is worldspawn. Contains the entity data string.
	*/
	model_t* worldmodel;

	CRC32_t worldmapCRC;

	byte clientdllmd5[ 16 ];

	/**
	*	Resource download list. Could be used to trim the files needed for download?
	*/
	resource_t resourcelist[ MAX_RESOURCES ];
	int num_resources;

	consistency_t consistency_list[ MAX_CONSISTENCY ];
	int num_consistency;

	/**
	*	List of models that have been precached.
	*/
	const char* model_precache[ MAX_MODELS ];
	model_t* models[ MAX_MODELS ];
	byte model_precache_flags[ MAX_MODELS ];

	event_t event_precache[ MAX_EVENTS ];

	const char* sound_precache[ MAX_SOUNDS ];

	short sound_precache_hashedlookup[ ( MAX_SOUNDS * 2 ) - 1 ];

	byte padding[ 2 ];

	qboolean sound_precache_hashedlookup_built;

	const char* generic_precache[ MAX_GENERIC ];
	char generic_precache_names[ MAX_GENERIC ][ MAX_QPATH ];

	int num_generic_names;

	const char* lightstyles[ MAX_LIGHTSTYLES ];

	int num_edicts;
	int max_edicts;

	edict_t* edicts;

	/*entity_state_t*/void* baselines;
	/*extra_baselines_t*/void* instance_baselines;

	server_state_t state;

	sizebuf_t datagram;
	byte datagram_buf[ MAX_DATAGRAM ];

	sizebuf_t reliable_datagram;
	byte reliable_datagram_buf[ MAX_DATAGRAM ];

	sizebuf_t multicast;
	byte multicast_buf[ MAX_MULTICAST ];

	sizebuf_t spectator;
	byte spectator_buf[ MAX_MULTICAST ];

	sizebuf_t signon;
	byte signon_data[ MAX_SIGNON ];
};

/**
*	This gets the server data structure from the engine. It's a complete hack, and only works with the current GoldSource build, but that's unlikely to change.
*	Doesn't work with Xash.
*	- Solokiller
*	@return server_t instance.
*/
inline server_t* SV_GetServer()
{
	byte* pStartSpot = ( byte* ) STRING( gpGlobals->startspot );

	return ( server_t* ) ( pStartSpot - offsetof( server_t, startspot ) );
}

#endif //ENGINE_SERVER_SERVER_H