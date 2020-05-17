//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "hud.h"
#include "cl_util.h"
#include "CHudChat.h"
#include "CHudTextMessage.h"
#include "CHudSayText.h"
#include "vgui/ILocalize.h"

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
static void (*gfnEngineMsgMode)(void) = nullptr;
static void (*gfnEngineMsgMode2)(void) = nullptr;
static void MessageModeVgui2()
{
	if (gHUD.m_SayText->m_pCvarOldChat->value || gEngfuncs.Cmd_Argc() != 1)
	{
		gfnEngineMsgMode();
	}
	else
	{
		if (gEngfuncs.GetMaxClients() == 1)
			return;
		gHUD.m_Chat->StartMessageMode(MM_SAY);
	}
}

static void MessageMode2Vgui2()
{
	if (gHUD.m_SayText->m_pCvarOldChat->value || gEngfuncs.Cmd_Argc() != 1)
	{
		gfnEngineMsgMode2();
	}
	else
	{
		if (gEngfuncs.GetMaxClients() == 1)
			return;
		gHUD.m_Chat->StartMessageMode(MM_SAY_TEAM);
	}
}

void CHudChat::Init( void )
{
	BaseClass::Init();

	// Hook messagemode and messagemode2
	if (!gfnEngineMsgMode && !gfnEngineMsgMode2)
	{
		cmd_function_t *item = gEngfuncs.GetFirstCmdFunctionHandle();
		cmd_function_t *msgMode = nullptr, *msgMode2 = nullptr;

		while (item)
		{
			if (!strcmp(item->name, "messagemode"))
				msgMode = item;
			else if (!strcmp(item->name, "messagemode2"))
				msgMode2 = item;
			item = item->next;
		}

		if (!msgMode || !msgMode2)
			ConPrintf("Failed to hook messagemode and messagemode2\n");
		else
		{
			gfnEngineMsgMode = msgMode->handler;
			msgMode->handler = MessageModeVgui2;
			gfnEngineMsgMode2 = msgMode2->handler;
			msgMode2->handler = MessageMode2Vgui2;
		}
	}
	else
		ConPrintf("messagemode and messagemode2 already hooked\n");
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
