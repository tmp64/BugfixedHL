#include <vgui/IInputInternal.h>
#include "vgui2/CClientMOTD.h"

#include "CHudViewport.h"

#include <vgui/ISurface.h>

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
}

IViewportPanel* CHudViewport::CreatePanelByName( const char* pszName )
{
	IViewportPanel* pPanel = nullptr;

	if( Q_strcmp( VIEWPORT_PANEL_MOTD, pszName ) == 0 )
	{
		pPanel = new CClientMOTD( this );
	}

	return pPanel;
}
