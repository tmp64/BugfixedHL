//
// CHudScoreBoard.cpp
//
// implementation of CHudScoreBoard class, a VGUI2-based scorebard
//

#include "CHudScoreBoard.h"
#include "hud.h"
#include "cl_util.h"
#include "vgui2/CBaseViewport.h"
#include "CScorePanel.h"

void CHudScoreBoard::Init()
{
	m_CvarMouseBtn = CVAR_CREATE("hud_scoreboard_mousebtn", "1", FCVAR_ARCHIVE);
	m_CvarAvatars = CVAR_CREATE("hud_scoreboard_showavatars", "1", FCVAR_ARCHIVE);
	m_CvarLoss = CVAR_CREATE("hud_scoreboard_showloss", "1", FCVAR_ARCHIVE);
	m_CvarEffSort = CVAR_CREATE("hud_scoreboard_effsort", "0", FCVAR_ARCHIVE);
	m_CvarEffType = CVAR_CREATE("hud_scoreboard_efftype", "1", FCVAR_ARCHIVE);
	m_CvarEffPercent = CVAR_CREATE("hud_scoreboard_effpercent", "0", FCVAR_ARCHIVE);
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);
	m_pScorePanel = CScorePanel::m_sSingleton;
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
