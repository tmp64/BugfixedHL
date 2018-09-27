#include <vgui/IInputInternal.h>
#include "vgui2/CClientMOTD.h"
#include "CScorePanel.h"
#include "CHudViewport.h"
#include <vgui/ISurface.h>
#include "hud.h"
#include "CHudScoreBoard.h"

void CHudViewport::Start()
{
	BaseClass::Start();
}

void CHudViewport::ActivateClientUI()
{
	BaseClass::ActivateClientUI();
}

void CHudViewport::CreateDefaultPanels()
{
	AddNewPanel( CreatePanelByName( VIEWPORT_PANEL_MOTD ) );
	AddNewPanel( CreatePanelByName( VIEWPORT_PANEL_SCORE ) );
}

IViewportPanel* CHudViewport::CreatePanelByName( const char* pszName )
{
	IViewportPanel* pPanel = nullptr;

	if (Q_strcmp( VIEWPORT_PANEL_MOTD, pszName ) == 0)
	{
		pPanel = new CClientMOTD( this );
	}
	else if (Q_strcmp(VIEWPORT_PANEL_SCORE, pszName) == 0)
	{
		pPanel = new CScorePanel(this);
	}

	return pPanel;
}
