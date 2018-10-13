//========= Copyright Valve Corporation, All rights reserved. ============//
//#include "cbase.h"
#include "clientsteamcontext.h"
#include "wrect.h"
#include "cl_dll.h"
#include "cl_util.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CClientSteamContext g_ClientSteamContext;
CClientSteamContext  &ClientSteamContext()
{
	return g_ClientSteamContext;
}

CSteamAPIContext *steamapicontext = &g_ClientSteamContext;

//-----------------------------------------------------------------------------
CClientSteamContext::CClientSteamContext() 
#if !defined(NO_STEAM)
:
	m_CallbackSteamServersDisconnected( this, &CClientSteamContext::OnSteamServersDisconnected ),
	m_CallbackSteamServerConnectFailure( this, &CClientSteamContext::OnSteamServerConnectFailure ),
	m_CallbackSteamServersConnected( this, &CClientSteamContext::OnSteamServersConnected )
#endif
{
	m_bActive = false;
	m_bLoggedOn = false;
	m_nAppID = 0;
}


//-----------------------------------------------------------------------------
CClientSteamContext::~CClientSteamContext()
{
}


//-----------------------------------------------------------------------------
// Purpose: Unload the steam3 engine
//-----------------------------------------------------------------------------
void CClientSteamContext::Shutdown()
{	
	if ( !m_bActive )
		return;

	m_bActive = false;
	m_bLoggedOn = false;
#if !defined( NO_STEAM )
	Clear(); // Steam API context shutdown
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the steam3 connection
//-----------------------------------------------------------------------------
void CClientSteamContext::Activate()
{
	if ( m_bActive )
		return;

	m_bActive = true;

#if !defined( NO_STEAM )
	SteamAPI_InitSafe(); // ignore failure, that will fall out later when they don't get a valid logon cookie
	SteamAPI_SetTryCatchCallbacks( false ); // We don't use exceptions, so tell steam not to use try/catch in callback handlers
	Init(); // Steam API context init
	
	UpdateLoggedOnState();
	gEngfuncs.Con_Printf( "CClientSteamContext logged on = %d\n", m_bLoggedOn );
#endif
}

void CClientSteamContext::UpdateLoggedOnState()
{
	bool bPreviousLoggedOn = m_bLoggedOn;
	m_bLoggedOn = ( SteamUser() && SteamUtils() && SteamUser()->BLoggedOn() );

	if ( !bPreviousLoggedOn && m_bLoggedOn )
	{
		// update Steam info
		m_SteamIDLocalPlayer = SteamUser()->GetSteamID();
		m_nUniverse = SteamUtils()->GetConnectedUniverse();
		m_nAppID = SteamUtils()->GetAppID();
	}

	if ( bPreviousLoggedOn != m_bLoggedOn )
	{
		// Notify any listeners of the change in logged on state
		SteamLoggedOnChange_t loggedOnChange;
		loggedOnChange.bPreviousLoggedOn = bPreviousLoggedOn;
		loggedOnChange.bLoggedOn = m_bLoggedOn;
	}
}

#if !defined(NO_STEAM)
void CClientSteamContext::OnSteamServersDisconnected( SteamServersDisconnected_t *pDisconnected )
{
	UpdateLoggedOnState();
	ConPrintf( "CClientSteamContext OnSteamServersDisconnected logged on = %d\n", m_bLoggedOn );
}

void CClientSteamContext::OnSteamServerConnectFailure( SteamServerConnectFailure_t *pConnectFailure )
{
	UpdateLoggedOnState();
	ConPrintf( "CClientSteamContext OnSteamServerConnectFailure logged on = %d\n", m_bLoggedOn );
}

void CClientSteamContext::OnSteamServersConnected( SteamServersConnected_t *pConnected )
{
	UpdateLoggedOnState();
	ConPrintf( "CClientSteamContext OnSteamServersConnected logged on = %d\n", m_bLoggedOn );
}
#endif // !defined(NO_STEAM)
