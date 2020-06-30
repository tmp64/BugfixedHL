//
// CHudScoreBoard.cpp
//
// implementation of CHudScoreBoard class, a VGUI2-based scorebard
//

#include <keydefs.h>
#include "CHudScoreBoard.h"
#include "hud.h"
#include "cl_util.h"
#include "vgui2/CBaseViewport.h"
#include "CScorePanel.h"

void CHudScoreBoard::Init()
{
	m_CvarMouseBtn = CVAR_CREATE("hud_scoreboard_mousebtn", "1", FCVAR_BHL_ARCHIVE);
	m_CvarAvatars = CVAR_CREATE("hud_scoreboard_showavatars", "1", FCVAR_BHL_ARCHIVE);
	m_CvarLoss = CVAR_CREATE("hud_scoreboard_showloss", "1", FCVAR_BHL_ARCHIVE);
	m_CvarEffSort = CVAR_CREATE("hud_scoreboard_effsort", "0", FCVAR_BHL_ARCHIVE);
	m_CvarEffType = CVAR_CREATE("hud_scoreboard_efftype", "1", FCVAR_BHL_ARCHIVE);
	m_CvarEffPercent = CVAR_CREATE("hud_scoreboard_effpercent", "0", FCVAR_BHL_ARCHIVE);
	m_CvarShowSteamId = CVAR_CREATE("hud_scoreboard_showsteamid", "1", FCVAR_BHL_ARCHIVE);
	m_CvarShowEff = CVAR_CREATE("hud_scoreboard_showeff", "1", FCVAR_BHL_ARCHIVE);
	m_CvarSize = CVAR_CREATE("hud_scoreboard_size", "0", FCVAR_BHL_ARCHIVE);
	m_CvarSpacingNormal = CVAR_CREATE("hud_scoreboard_spacing_normal", "0", FCVAR_BHL_ARCHIVE);
	m_CvarSpacingCompact = CVAR_CREATE("hud_scoreboard_spacing_compact", "0", FCVAR_BHL_ARCHIVE);
	m_iFlags |= HUD_ACTIVE;
	m_pScorePanel = dynamic_cast<CScorePanel *>(g_pViewport->FindPanelByName(VIEWPORT_PANEL_SCORE));
	Assert(m_pScorePanel);
}

void CHudScoreBoard::VidInit()
{
}

void CHudScoreBoard::Draw(float flTime)
{
}

void CHudScoreBoard::Think()
{
	if (m_pScorePanel && m_pScorePanel->IsVisible() && m_flScoreBoardLastUpdated < gHUD.m_flTime)
	{
		m_pScorePanel->UpdateAllClients();
		m_flScoreBoardLastUpdated = gHUD.m_flTime + 0.5;
	}
}

void CHudScoreBoard::Reset(void)
{
	if (m_pScorePanel)
		m_pScorePanel->Reset();
}

void CHudScoreBoard::ShowScoreBoard()
{
	if (!m_pScorePanel) return;

	// No Scoreboard in single-player
	if (gEngfuncs.GetMaxClients() <= 1) return;

	//m_pScorePanel->ShowPanel(true);
	g_pViewport->ShowPanel(m_pScorePanel, true);
	m_pScorePanel->FullUpdate();
}

void CHudScoreBoard::HideScoreBoard(bool force)
{
	if (!m_pScorePanel) return;

	// Prevent removal of scoreboard during intermission
	if (!force && gHUD.m_iIntermission) return;

	//m_pScorePanel->ShowPanel(false);
	g_pViewport->ShowPanel(m_pScorePanel, false);
}

void CHudScoreBoard::UpdateClientInfo(int client)
{
	if (!m_pScorePanel || !m_pScorePanel->IsVisible()) return;
	m_pScorePanel->UpdateClientInfo(client);
}

void CHudScoreBoard::EnableMousePointer(bool enable)
{
	if (m_pScorePanel) m_pScorePanel->EnableMousePointer(enable);
}

bool CHudScoreBoard::IsVisible()
{
	return m_pScorePanel->IsVisible();
}

void CHudScoreBoard::UpdateServerName()
{
	m_pScorePanel->UpdateServerName();
}

void CHudScoreBoard::InitHUDData()
{
	m_pScorePanel->InitHudData();
}

bool CHudScoreBoard::HandleKeyEvent(int down, int keynum, const char * pszCurrentBinding)
{
	int constexpr WHEEL_DELTA = 20;

	if (!down)
		return false;

	if (!gHUD.m_ScoreBoard->IsVisible())
		return false;

	if ((keynum == K_MOUSE1 && m_CvarMouseBtn->value == 1) || (keynum == K_MOUSE2 && m_CvarMouseBtn->value == 2))
	{
		EnableMousePointer(true);
	}
	else if (keynum == K_MWHEELDOWN)
	{
		m_pScorePanel->OnMouseWheeled(WHEEL_DELTA);
	}
	else if (keynum == K_MWHEELUP)
	{
		m_pScorePanel->OnMouseWheeled(-WHEEL_DELTA);
	}
	else
		return false;

	return true;
}
