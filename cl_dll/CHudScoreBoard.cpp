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

int CHudScoreBoard::Init(void)
{
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);
	return 1;
}

int CHudScoreBoard::VidInit(void)
{
	if (m_pScorePanel == nullptr)
	{
		m_pScorePanel = dynamic_cast<CScorePanel *>(g_pViewport->CreatePanelByName(VIEWPORT_PANEL_SCORE));
		g_pViewport->AddNewPanel(m_pScorePanel);
	}
	m_pScorePanel->ShowPanel(false);
	return 1;
}

int CHudScoreBoard::Draw(float flTime)
{
	return 0;
}

void CHudScoreBoard::Think()
{
	if (m_pScorePanel && m_pScorePanel->IsVisible() && m_flScoreBoardLastUpdated < gHUD.m_flTime)
	{
		m_pScorePanel->UpdateAllClients();
		m_flScoreBoardLastUpdated = gHUD.m_flTime + 0.5;
	}
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
