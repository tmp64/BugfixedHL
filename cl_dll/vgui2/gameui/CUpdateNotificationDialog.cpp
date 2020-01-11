#include <vgui_controls/RichText.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <vgui/ILocalize.h>
#include "CUpdateNotificationDialog.h"
#include "GameUIPanelNames.h"
#include "vgui2/VGUI2Paths.h"
#include "vgui2/IEngineVgui.h"
#include "vgui2/gameui/CGameUIViewport.h"
#include "cl_dll.h"
#include "cl_util.h"
#include "hud.h"
#include <CGameUpdater.h>

class CChangelogDialog : vgui2::Frame
{
public:
	DECLARE_CLASS_SIMPLE(CChangelogDialog, Frame);

public:
	CChangelogDialog(vgui2::Panel *parent) : BaseClass(parent, "ChangelogDialog")
	{
		SetKeyBoardInputEnabled(true);
		SetMouseInputEnabled(true);
		MakePopup();
		SetProportional(false);
		SetTitleBarVisible(true);
		SetMinimizeButtonVisible(false);
		SetMaximizeButtonVisible(false);
		SetCloseButtonVisible(true);
		SetSizeable(false);
		SetMoveable(true);
		SetVisible(false);
		SetTitle("Changelog", false);
		SetSize(480, 320);

		m_pChangelogText = new vgui2::RichText(this, "ChangelogText");
		LoadControlSettings(UI_RESOURCE_DIR "/ChangelogDialog.res");

		SetScheme(vgui2::scheme()->LoadSchemeFromFile(UI_GAMEUISCHEME_FILENAME, "SourceScheme"));
	}

	virtual void Activate()
	{
		// SetText(const char *) uses a tiny buffer (1 KB) to convert UTF-8 to UTF-16
		wchar_t *buf = new wchar_t[16 * 1024];	// 16 KB should probably be enough
		vgui2::localize()->ConvertANSIToUnicode(gGameUpdater->GetChangeLog().c_str(), buf, 16 * 1024);
		m_pChangelogText->SetText(buf);
		delete[] buf;
		BaseClass::Activate();
	}

private:
	vgui2::RichText *m_pChangelogText = nullptr;
};

CUpdateNotificationDialog::CUpdateNotificationDialog(vgui2::VPANEL parent) : BaseClass(nullptr, GAMEUI_UPDATE_NOTIF)
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
	SetTitle("New update available", false);

	m_pChangelogDialog = new CChangelogDialog(this);

	m_pMainTextLabel = new vgui2::Label(this, "MainTextLabel", "A new update was released.");
	m_pGameVersionLabel = new vgui2::Label(this, "GameVersionLabel", "");
	m_pNewVersionLabel = new vgui2::Label(this, "NewVersionLabel", "");
	m_pAutoUpdateBox = new vgui2::CheckButton(this, "AutoUpdateBox", "Automatically check for updates");
	m_pOpenGithubBtn = new vgui2::Button(this, "OpenGithubBtn", "Open GitHub", this, "url https://github.com/tmp64/BugfixedHL/releases");
	m_pOkBtn = new vgui2::Button(this, "OkBtn", "OK", this, "okay");
	m_pChangelogBtn = new vgui2::Button(this, "ChangelogBtn", "Changelog", this, "changelog");

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(UI_GAMEUISCHEME_FILENAME, "SourceScheme"));
	LoadControlSettings(UI_RESOURCE_DIR "/UpdateNotificationDialog.res");
}

CUpdateNotificationDialog::~CUpdateNotificationDialog() {}

void CUpdateNotificationDialog::OnCommand(const char* command)
{
	if (!strcmp(command, "okay"))
	{
		if (m_pAutoUpdateBox->IsSelected())
			ClientCmd("cl_check_for_updates 1");
		else
			ClientCmd("cl_check_for_updates 0");
		OnCommand("Close");
	}
	else if (!strcmp(command, "changelog"))
	{
		m_pChangelogDialog->Activate();
	}
	else BaseClass::OnCommand(command);
}

void CUpdateNotificationDialog::Activate()
{
	if (!IsVisible())
	{
		constexpr int WIDE = 400, TALL = 200, GAP = 6;
		SetSize(WIDE, TALL);
		MoveToCenterOfScreen();
		if (gHUD.m_pCvarCheckUpdates)
			m_pAutoUpdateBox->SetSelected(!!gHUD.m_pCvarCheckUpdates->value);

		int major, minor, patch;
		char buf[32];

		gGameUpdater->GetGameVersion().GetVersion(major, minor, patch);
		if (patch != 0)
			snprintf(buf, sizeof(buf), "Your version: %d.%d.%d", major, minor, patch);
		else
			snprintf(buf, sizeof(buf), "Your version: %d.%d", major, minor);
		m_pGameVersionLabel->SetText(buf);

		gGameUpdater->GetLatestVersion().GetVersion(major, minor, patch);
		snprintf(buf, sizeof(buf), "New version: %d.%d", major, minor);
		m_pNewVersionLabel->SetText(buf);
	}
	BaseClass::Activate();
}

const char *CUpdateNotificationDialog::GetName()
{
	return GAMEUI_UPDATE_NOTIF;
}

void CUpdateNotificationDialog::OnGameUIActivated()
{
}

void CUpdateNotificationDialog::OnGameUIHidden()
{
}

vgui2::VPANEL CUpdateNotificationDialog::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CUpdateNotificationDialog::IsVisible()
{
	return BaseClass::IsVisible();
}
