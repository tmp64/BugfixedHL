#ifndef CHUDBASE_H
#define CHUDBASE_H

#ifndef _WIN32
#define _cdecl 
#endif

#include <list>
#include "cl_dll.h"
#include "wrect.h"
#include "cvardef.h"

#define RGB_YELLOWISH	0x00FFA000 //255,160,0
#define RGB_REDISH		0x00FF1010 //255,160,0
#define RGB_GREENISH	0x0000A000 //0,160,0

#define DHN_DRAWZERO	1
#define DHN_2DIGITS		2
#define DHN_3DIGITS		4
#define MIN_ALPHA		100
#define ALPHA_AMMO_FLASH	100
#define ALPHA_AMMO_MAX		128
#define ALPHA_POINTS_FLASH	128
#define ALPHA_POINTS_MAX	155

#define HUDELEM_ACTIVE	1

#define HUD_ACTIVE			1
#define HUD_INTERMISSION	2

#define MAX_HUD_STRING			80
#define MAX_MOTD_LENGTH			1536
#define MAX_UNICODE_MOTD_LENGTH (MAX_MOTD_LENGTH * 2)	// Some Unicode charachters take two or more bytes in UTF8
#define MAX_STEAMID				32	// 0:0:4294967295, STEAM_ID_PENDING
#define MAX_MAP_NAME			64

#define ADJUST_MENU		-5	// space correction between text lines in hud menu in pixels
#define ADJUST_MESSAGE	0	// space correction between text lines in hud messages in pixels

#define HUD_FADE_TIME 100

union RGBA {
	struct { unsigned char r, g, b, a; };
	unsigned int c;

	RGBA() { c = 0; }
	RGBA(unsigned int value) { c = value; }
	RGBA(unsigned char r1, unsigned char g1, unsigned char b1) { r = r1; g = g1; b = b1; a = 255; }
	void Set(unsigned char r1, unsigned char g1, unsigned char b1) { r = r1; g = g1; b = b1; a = 255; }

	// Colors for printing info to console
	class ConColor
	{
	public:
		static RGBA Red, Green, Yellow, Cyan;
	};
};

struct POSITION 
{
	int x, y;
};

typedef struct cvar_s cvar_t;

class CHudBase
{
public:
	POSITION  m_pos;
	int   m_type;
	int	  m_iFlags; // active, moving, 
	CHudBase();
	virtual		~CHudBase();
	virtual void Init();
	virtual void VidInit();
	virtual void Draw(float flTime);
	virtual void Think();
	virtual void Reset();
	virtual void InitHUDData();		// called every time a server is connected to

private:
	bool m_bIsIteratorValid = false;
	std::list<CHudBase *>::iterator m_ThisIterator;

	void EraseFromHudList();

	friend class CHud;
};

#endif