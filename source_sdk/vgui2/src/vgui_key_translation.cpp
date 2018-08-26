//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#if defined( WIN32 ) && !defined( _X360 )
#define NOMINMAX
#include <wtypes.h>
#include <winuser.h>
#include "xbox/xboxstubs.h"
#endif
#include "tier0/dbg.h"
#include "vgui_key_translation.h"
#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif
#ifdef POSIX
#define VK_RETURN -1
#endif

#include "vgui/ISystem.h"
#include "vgui_controls/Controls.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

vgui2::KeyCode KeyCode_VirtualKeyToVGUI( int key )
{
	// Some tools load vgui for localization and never use input
	if ( !vgui2::system() )
		return vgui2::KEY_NONE;
	return vgui2::system()->KeyCode_VirtualKeyToVGUI( key );
}

int KeyCode_VGUIToVirtualKey( vgui2::KeyCode code )
{
	// Some tools load vgui for localization and never use input
	if ( !vgui2::system() )
		return VK_RETURN;

	return vgui2::system()->KeyCode_VGUIToVirtualKey( code );
}
