#include "CAdvOptionsDialog.h"
#include "vgui2/gameui/GameUIPanelNames.h"
#include "vgui2/VGUI2Paths.h"
#include "vgui2/IEngineVgui.h"
#include "vgui2/CClientVGUI.h"
#include "vgui2/gameui/CGameUIViewport.h"
#include "IBaseUI.h"
#include "hud.h"

#include "vgui_controls/PropertySheet.h"

#include "CHudSubOptions.h"
#include "CChatSubOptions.h"
#include "CCrosshairSubOptions.h"
#include "CScoreboardSubOptions.h"
#include "CGeneralSubOptions.h"
#include "CAboutSubOptions.h"

static void OpenAdvOptionsCommand()
{
	CAdvOptionsDialog *panel = g_pGameUIViewport->FindPanel<CAdvOptionsDialog>(GAMEUI_ADV_OPTIONS);

	auto fnActivateNextFrame = []()
	{
		CAdvOptionsDialog *panel = g_pGameUIViewport->FindPanel<CAdvOptionsDialog>(GAMEUI_ADV_OPTIONS); 
		if (panel)
			panel->Activate();
		else
			assert(!("fnActivateNextFrame: GAMEUI_ADV_OPTIONS panel not found"));
	};

	if (panel)
	{
		if (panel->IsVisible())
		{
			// See below for more details about that hack
			gHUD.CallOnNextFrame(fnActivateNextFrame);
			baseUI()->ActivateGameUI();
			return;
		}
		else
		{
			// Panel will be recreated
			g_pGameUIViewport->DeletePanel(panel);
		}
	}

	// Create new panel
	panel = g_pGameUIViewport->CreatePanel<CAdvOptionsDialog>(GAMEUI_ADV_OPTIONS);
	g_pGameUIViewport->AddPanel(panel);

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

	AddPage(new CGeneralSubOptions(this), "#BHL_AdvOptions_General");
	AddPage(new CHudSubOptions(this), "#BHL_AdvOptions_HUD");
	AddPage(new CChatSubOptions(this), "#BHL_AdvOptions_Chat");
	AddPage(new CScoreboardSubOptions(this), "#BHL_AdvOptions_Scores");
	AddPage(new CCrosshairSubOptions(this), "#BHL_AdvOptions_Cross");
	AddPage(new CAboutSubOptions(this), "#BHL_AdvOptions_About");

	SetApplyButtonVisible(true);
	EnableApplyButton(true);
	GetPropertySheet()->SetTabWidth(84);
}

CAdvOptionsDialog::~CAdvOptionsDialog()
{

}

void CAdvOptionsDialog::Activate()
{
	if (!IsVisible())
	{
		MoveToCenterOfScreen();
	}
	BaseClass::Activate();
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

void CAdvOptionsDialog::OnGameUIActivated()
{
}

void CAdvOptionsDialog::OnGameUIHidden()
{
}

vgui2::VPANEL CAdvOptionsDialog::GetVPanel()
{
	return BaseClass::GetVPanel();
}
