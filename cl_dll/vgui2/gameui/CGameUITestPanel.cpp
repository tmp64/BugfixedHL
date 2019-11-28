#include <vgui_controls/RichText.h>
#include "CGameUITestPanel.h"
#include "GameUIPanelNames.h"
#include "vgui2/VGUI2Paths.h"
#include "vgui2/IEngineVgui.h"
#include "vgui2/gameui/CGameUIViewport.h"
#include "cl_dll.h"
#include "cl_util.h"

static void __CmdFunc_OpenGameUITestPanel()
{
	CGameUITestPanel *panel = g_pGameUIViewport->FindPanel<CGameUITestPanel>(GAMEUI_PANEL_TEST);
	if (!panel)
	{
		ConPrintf("__CmdFunc_OpenGameUITestPanel: panel is NULL\n");
		return;
	}

	panel->Activate();
}

CGameUITestPanel::CGameUITestPanel(vgui2::VPANEL parent) : BaseClass(nullptr, GAMEUI_PANEL_TEST)
{
	BaseClass::SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(true);
	SetSizeable(false);
	SetMoveable(true);
	SetVisible(false);
	SetTitle(L"Quote of the day", false);

	m_pRichText = new vgui2::RichText(this, "QuoteBox");
	m_pRichText->SetVerticalScrollbar(false);

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(UI_GAMEUISCHEME_FILENAME, "SourceScheme"));
	Reset();
	HOOK_COMMAND("gameui_open_test_panel", OpenGameUITestPanel);
}

CGameUITestPanel::~CGameUITestPanel() {}

void CGameUITestPanel::Reset()
{
	constexpr int WIDE = 400, TALL = 200, GAP = 6;
	SetSize(WIDE, TALL);
	MoveToCenterOfScreen();
	m_pRichText->SetSize(WIDE - GAP - GAP, TALL - GAP - GAP - GetCaptionHeight());
	m_pRichText->SetPos(GAP, GetCaptionHeight() + GAP);

	const wchar_t *quotes[] = {
		L"\"DO YOU **** *****?\" - Gunnery Sergeant Hartman, your senior drill instructor",
		L"\"Freeman, you fool!\" - that scientist from the teleport chamber",
		L"\"Did you submit your status report to the administrator, today?\" - Did you submit your status report to the administrator, today?"
	};

	int idx = rand() % ARRAYSIZE(quotes);
	m_pRichText->SetText(quotes[idx]);
}

void CGameUITestPanel::Activate()
{
	if (!IsVisible())
	{
		Reset();
	}
	BaseClass::Activate();
}

void CGameUITestPanel::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	SetPaintBackgroundType(2);
	BaseClass::ApplySchemeSettings(pScheme);
}

const char *CGameUITestPanel::GetName()
{
	return GAMEUI_PANEL_TEST;
}

void CGameUITestPanel::OnGameUIActivated()
{
}

void CGameUITestPanel::OnGameUIHidden()
{
}

vgui2::VPANEL CGameUITestPanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}
