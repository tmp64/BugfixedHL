#include "CUpdateNotification.h"
#include "cl_dll.h"
#include "cl_util.h"
#include <CGameUpdater.h>
#include "CHudBase.h"

#ifdef USE_VGUI2
#include "vgui2/gameui/GameUIPanelNames.h"
#include "vgui2/gameui/CGameUIViewport.h"
#include "vgui2/gameui/CUpdateNotificationDialog.h"
#endif

CUpdateNotification *gUpdateNotif = nullptr;

CUpdateNotification::CUpdateNotification()
{
	m_iCallbackId = gGameUpdater->AddCheckFinishedCallback([&](bool b)
	{
		CheckFinishedCallback(b);
	});
}

CUpdateNotification::~CUpdateNotification()
{
	if (m_iCallbackId != -1)
	{
		gGameUpdater->RemoveCheckFinishedCallback(m_iCallbackId);
	}
}

void CUpdateNotification::SetActive(bool state)
{
	m_bIsActive = state;
}

void CUpdateNotification::CheckFinishedCallback(bool isUpdateFound)
{
	if (!m_bIsActive || m_bIsNotified)
		return;
	m_bIsNotified = true;
	if (!isUpdateFound)
		return;
	
	char gameVer[32], newVer[32];
	int major, minor, patch;

	gGameUpdater->GetGameVersion().GetVersion(major, minor, patch);
	snprintf(gameVer, sizeof(gameVer), "%d.%d.%d", major, minor, patch);

	gGameUpdater->GetLatestVersion().GetVersion(major, minor, patch);
	snprintf(newVer, sizeof(newVer), "%d.%d.%d", major, minor, patch);

	// Print to console
	RGBA oldColor = SetConsoleColor(RGBA(0, 255, 0));
	ConPrintf("************************************************\n");
	ConPrintf("* A new update was found!\n");
	ConPrintf("* Your version: %s\n", gameVer);
	ConPrintf("* New version:  %s\n", newVer);
	ConPrintf("************************************************\n");
	ConPrintf("* Type 'update_changelog' to see what's new.\n");
	ConPrintf("* https://github.com/tmp64/BugfixedHL/releases\n");
	ConPrintf("************************************************\n");
	SetConsoleColor(oldColor);

#ifdef USE_VGUI2
	auto upd = g_pGameUIViewport->FindPanel<CUpdateNotificationDialog>(GAMEUI_UPDATE_NOTIF);
	upd->Activate();
#endif
}
