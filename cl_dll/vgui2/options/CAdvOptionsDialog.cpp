#include "CAdvOptionsDialog.h"
#include "../GameUIPanelNames.h"
#include "../VGUI2Paths.h"
#include "../IEngineVgui.h"
#include "../CBaseViewport.h"
#include "../CClientVGUI.h"
#include "IBaseUI.h"
#include "hud.h"

#include "vgui_controls/PropertySheet.h"

#include "CHudSubOptions.h"

static void OpenAdvOptionsCommand()
{
	CAdvOptionsDialog *panel = dynamic_cast<CAdvOptionsDialog *>(g_pViewport->FindGameUIPanelByName(GAMEUI_ADV_OPTIONS));
	
	auto fnActivateNextFrame = []()
	{
		CAdvOptionsDialog *panel = dynamic_cast<CAdvOptionsDialog *>(g_pViewport->FindGameUIPanelByName(GAMEUI_ADV_OPTIONS));
		if (panel)
			panel->Activate();
		else
			assert(!("fnActivateNextFrame: GAMEUI_ADV_OPTIONS panel not found"));
	};

	if (panel)
	{
		if (panel->IsOpen())
		{
			// See below for more details about that hack
			gHUD.CallOnNextFrame(fnActivateNextFrame);
			baseUI()->ActivateGameUI();
			return;
		}
		else
		{
			// Panel will be recreated
			g_pViewport->DeleteGameUIPanel(panel);
		}
	}

	// Create new panel
	panel = dynamic_cast<CAdvOptionsDialog *>(g_pViewport->CreateGameUIPanelByName(GAMEUI_ADV_OPTIONS));
	Assert(panel);
	g_pViewport->AddNewGameUIPanel(panel);

	// Since this command is called from game menu using "engine gameui_open_adv_options"
	// GameUI will hide itself and show the game.
	// We need to show it again and after that activate CAdvOptionsDialog
	// Otherwise it may be hidden by the dev console
	gHUD.CallOnNextFrame(fnActivateNextFrame);
	baseUI()->ActivateGameUI();
}

void CAdvOptionsDialog::RegisterConsoleCommands()
{
	gEngfuncs.pfnAddCommand("gameui_open_adv_options", OpenAdvOptionsCommand);
}

CAdvOptionsDialog::CAdvOptionsDialog(vgui2::VPANEL parent) : BaseClass(nullptr, GAMEUI_ADV_OPTIONS)
{
	BaseClass::SetParent(parent);

	SetBounds(0, 0, 512, 406);
	SetSizeable(false);

	SetTitle("#BHL_AdvOptions", true);

	AddPage(new CHudSubOptions(this), "#BHL_AdvOptions_HUD");

	SetApplyButtonVisible(true);
	EnableApplyButton(true);
	GetPropertySheet()->SetTabWidth(84);
}

CAdvOptionsDialog::~CAdvOptionsDialog()
{

}

void CAdvOptionsDialog::Activate()
{
	if (!m_bIsOpen)
	{
		m_bIsOpen = true;
		Reset();
	}
	BaseClass::Activate();
}

void CAdvOptionsDialog::OnClose()
{
	BaseClass::OnClose();
	m_bIsOpen = false;	// Panel will be recreated next time it is openned
}

bool CAdvOptionsDialog::IsOpen()
{
	return m_bIsOpen;
}

void CAdvOptionsDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "Apply"))
	{
		BaseClass::OnCommand("Apply");
		EnableApplyButton(true);	// Apply should always be enabled
	}
	else
		BaseClass::OnCommand(command);
}

const char *CAdvOptionsDialog::GetName()
{
	return GAMEUI_ADV_OPTIONS;
}

void CAdvOptionsDialog::Reset()
{
	MoveToCenterOfScreen();
}

void CAdvOptionsDialog::ShowPanel(bool state)
{
	if (BaseClass::IsVisible() == state)
		return;

	if (state)
	{
		BaseClass::Activate();
	}
	else
	{
		BaseClass::SetVisible(false);
	}
}

void CAdvOptionsDialog::OnGameUIActivated()
{
	if (m_bIsOpen)
		ShowPanel(true);
}

void CAdvOptionsDialog::OnGameUIDeactivated()
{
	if (m_bIsOpen)
		ShowPanel(false);
}

vgui2::VPANEL CAdvOptionsDialog::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CAdvOptionsDialog::IsVisible()
{
	return BaseClass::IsVisible();
}
