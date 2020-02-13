#include <vgui_controls/RichText.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <CGameUpdater.h>
#include "CUpdaterDebugDialog.h"
#include "GameUIPanelNames.h"
#include "vgui2/VGUI2Paths.h"
#include "vgui2/IEngineVgui.h"
#include "vgui2/gameui/CGameUIViewport.h"
#include "cl_dll.h"
#include "cl_util.h"

static void __CmdFunc_OpenUpdaterDebugDialog()
{
	CUpdaterDebugDialog *panel = g_pGameUIViewport->FindPanel<CUpdaterDebugDialog>(GAMEUI_UPDATER_DEBUG);
	if (!panel)
	{
		ConPrintf("__CmdFunc_OpenGameUITestPanel: panel is NULL\n");
		return;
	}

	panel->Activate();
}

CUpdaterDebugDialog::CUpdaterDebugDialog(vgui2::VPANEL parent) : BaseClass(nullptr, GAMEUI_UPDATER_DEBUG)
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
	SetTitle("Game updater debug", false);

	m_pCheckBtn = new vgui2::Button(this, "CheckBtn", "Check", this, "check");

	m_pDownloadBtn = new vgui2::Button(this, "DownloadBtn", "Download", this, "download");
	m_pDownloadBtn->SetEnabled(false);

	m_pInstallBtn = new vgui2::Button(this, "InstallBtn", "Install", this, "install");
	m_pInstallBtn->SetEnabled(false);

	m_pChangeLogBox = new vgui2::RichText(this, "ChangeLogBox");
	m_pChangeLogBox->SetSize(460, 410);

	m_pErrorLabel = new vgui2::Label(this, "ErrorLabel", "Status: none");
	m_pGameVerLabel = new vgui2::Label(this, "GameVerLabel", "Game: ???");
	m_pLatestVerLabel = new vgui2::Label(this, "LatestVerLabel", "Update: ???");

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(UI_GAMEUISCHEME_FILENAME, "SourceScheme"));
	LoadControlSettings(UI_RESOURCE_DIR "/UpdaterDebugDialog.res");

	HOOK_COMMAND("gameui_open_updater_debug", OpenUpdaterDebugDialog);
}

CUpdaterDebugDialog::~CUpdaterDebugDialog() {}

void CUpdaterDebugDialog::OnCommand(const char* command)
{
	if (!strcmp(command, "check"))
	{
		m_pCheckBtn->SetEnabled(false);
		m_pDownloadBtn->SetEnabled(false);
		m_pInstallBtn->SetEnabled(false);
		gGameUpdater->CheckForUpdates();
	}
	else if (!strcmp(command, "download"))
	{
#ifdef UPDATER_WIP_DL
		gGameUpdater->Download();
#else
		m_pErrorLabel->SetText("Error: not implemented");
#endif
	}
	else if (!strcmp(command, "install"))
	{
#ifdef UPDATER_WIP_DL
		try
		{
			gGameUpdater->Install();
		}
		catch (const std::exception &e)
		{
			m_pErrorLabel->SetText((std::string("Error: (exception) ") + e.what()).c_str());
		}
#else
		m_pErrorLabel->SetText("Error: not implemented");
#endif
	}
	else BaseClass::OnCommand(command);
}

void CUpdaterDebugDialog::Activate()
{
	constexpr int WIDE = 640, TALL = 480;
	SetSize(WIDE, TALL);
	MoveToCenterOfScreen();
	AddCallbacks();
	BaseClass::Activate();
}

const char *CUpdaterDebugDialog::GetName()
{
	return GAMEUI_UPDATER_DEBUG;
}

void CUpdaterDebugDialog::OnGameUIActivated()
{
}

void CUpdaterDebugDialog::OnGameUIHidden()
{
}

vgui2::VPANEL CUpdaterDebugDialog::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CUpdaterDebugDialog::IsVisible()
{
	return BaseClass::IsVisible();
}

void CUpdaterDebugDialog::AddCallbacks()
{
	if (m_bCallbacksAdded) return;

	char buf[64];
	int major, minor, patch;

	gGameUpdater->GetGameVersion().GetVersion(major, minor, patch);
	snprintf(buf, sizeof(buf), "Game: %d.%d.%d", major, minor, patch);
	m_pGameVerLabel->SetText(buf);

	gGameUpdater->AddErrorOccuredCallback([&](std::string error)
	{
		ErrorOccuredCallback(error);
	});

	gGameUpdater->AddCheckFinishedCallback([&](bool isUpdateFound)
	{
		CheckFinishedCallback(isUpdateFound);
	});

#ifdef UPDATER_WIP_DL
	gGameUpdater->AddDownloadFinishedCallback([&]()
	{
		DownloadFinishedCallback();
	});
#endif

	m_bCallbacksAdded = true;
}

void CUpdaterDebugDialog::ErrorOccuredCallback(std::string error)
{
	m_pErrorLabel->SetText(("Error: (ErrorOccuredCallback) " + error).c_str());
}

void CUpdaterDebugDialog::CheckFinishedCallback(bool isUpdateFound)
{
	m_pCheckBtn->SetEnabled(true);

	char buf[64];
	int major, minor, patch;
	gGameUpdater->GetLatestVersion().GetVersion(major, minor, patch);
	snprintf(buf, sizeof(buf), "Latest: %d.%d.%d", major, minor, patch);
	m_pLatestVerLabel->SetText(buf);

	if (isUpdateFound)
	{
		m_pDownloadBtn->SetEnabled(true);
		m_pChangeLogBox->SetText(gGameUpdater->GetChangeLog().c_str());
	}
	else
	{
		m_pErrorLabel->SetText("Status: no updates");
	}
}

void CUpdaterDebugDialog::DownloadFinishedCallback()
{
	m_pDownloadBtn->SetEnabled(false);
	m_pInstallBtn->SetEnabled(true);
	m_pErrorLabel->SetText("Status: download finished");
}
