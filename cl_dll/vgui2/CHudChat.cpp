//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "hud.h"
#include "cl_util.h"
#include "CHudChat.h"
#include "CHudTextMessage.h"
#include "vgui/ILocalize.h"

DECLARE_MESSAGE_PTR(m_Chat, SayText);

//=====================
//CHudChatLine
//=====================

void CHudChatLine::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
}

//=====================
//CHudChatInputLine
//=====================

void CHudChatInputLine::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//=====================
//CHudChat
//=====================

CHudChat::CHudChat() : BaseClass()
{
	
}

void CHudChat::CreateChatInputLine( void )
{
	m_pChatInput = new CHudChatInputLine( this, "ChatInputLine" );
	m_pChatInput->SetVisible( false );
}

void CHudChat::CreateChatLines( void )
{
	m_ChatLine = new CHudChatLine( this, "ChatLine1" );
	m_ChatLine->SetVisible( false );	
}

void CHudChat::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: Shows chat input box
//-----------------------------------------------------------------------------
static void __CmdFunc_MessageModeVgui2()
{
	if (gEngfuncs.GetMaxClients() == 1)
		return;
	gHUD.m_Chat->StartMessageMode(MM_SAY);
}

int CHudChat::Init( void )
{
	BaseClass::Init();

	HOOK_MESSAGE(SayText);
	//HOOK_HUD_MESSAGE( CHudChat, TextMsg );
	HOOK_COMMAND("messagemode_vgui2", MessageModeVgui2);

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Overrides base reset to not cancel chat at round restart
//-----------------------------------------------------------------------------
void CHudChat::Reset( void )
{
}

int CHudChat::GetChatInputOffset( void )
{
	if ( m_pChatInput->IsVisible() )
	{
		return m_iFontHeight;
	}
	else
		return 0;
}

SDK_Color CHudChat::GetClientColor(int clientIndex)
{
	if (clientIndex == 0) // console msg
	{
		return gHUD.GetTeamColor(0);
	}
	else
	{
		if (clientIndex >= 1 && clientIndex <= MAX_PLAYERS)
			return gHUD.GetTeamColor(g_PlayerExtraInfo[clientIndex].teamnumber);
	}

	return gHUD.GetTeamColor(0);
}

int CHudChat::MsgFunc_SayText(const char *pszName, int iSize, void * pbuf)
{
	return CBaseHudChat::MsgFunc_SayText(pszName, iSize, pbuf);
}
