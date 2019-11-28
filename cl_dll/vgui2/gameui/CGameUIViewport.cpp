#include "CGameUIViewport.h"
#include "GameUIPanelNames.h"
#include "vgui2/IEngineVgui.h"
#include "vgui2/VGUI2Paths.h"

#include "CGameUITestPanel.h"
#include "options/CAdvOptionsDialog.h"

#ifdef USE_UPDATER
#include "CUpdaterDebugDialog.h"
#include "CUpdateNotificationDialog.h"
#endif

CGameUIViewport *g_pGameUIViewport = nullptr;
bool g_bIsCreatingGameUIPanel = false;	// Used in vgui2::Panel::Panel()

CGameUIViewport::CGameUIViewport() : BaseClass(nullptr, "ClientGameUIViewport")
{
	Assert(!g_pGameUIViewport);
	g_pGameUIViewport = this;
	SetParent(engineVgui()->GetPanel(PANEL_ROOT));
	SetScheme(vgui2::scheme()->LoadSchemeFromFile(UI_CLIENTSCHEME_FILENAME, "SourceScheme"));
	SetProportional(false);
	SetSize(0, 0);
}

void CGameUIViewport::Initialize(CreateInterfaceFn *pFactories, int iNumFactories)
{
}

void CGameUIViewport::Start()
{
	CreateDefaultPanels();
}

void CGameUIViewport::ActivateClientUI()
{
	// HUD activated, hide GameUI
	SetVisible(false);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	// Notify panels
	for (IGameUIPanel *i : m_Panels)
		i->OnGameUIHidden();
}

void CGameUIViewport::HideClientUI()
{
	// HUD is hidden, show GameUI
	SetVisible(true);
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	// Notify panels
	for (IGameUIPanel *i : m_Panels)
		i->OnGameUIActivated();
}

void CGameUIViewport::Shutdown()
{
}

void CGameUIViewport::CreateDefaultPanels()
{
	AddPanel(CreatePanelByName(GAMEUI_PANEL_TEST));
#ifdef USE_UPDATER
	AddPanel(CreatePanelByName(GAMEUI_UPDATER_DEBUG));
	AddPanel(CreatePanelByName(GAMEUI_UPDATE_NOTIF));
#endif

	CAdvOptionsDialog::RegisterConsoleCommands();
}

IGameUIPanel *CGameUIViewport::CreatePanelByName(const char *pszName)
{
	IGameUIPanel *pPanel = nullptr;
	g_bIsCreatingGameUIPanel = true;

	if (Q_strcmp(GAMEUI_PANEL_TEST, pszName) == 0)
	{
		pPanel = new CGameUITestPanel(GetVPanel());
	}
	else if (Q_strcmp(GAMEUI_ADV_OPTIONS, pszName) == 0)
	{
		pPanel = new CAdvOptionsDialog(GetVPanel());
	}
#ifdef USE_UPDATER
	else if (Q_strcmp(GAMEUI_UPDATER_DEBUG, pszName) == 0)
	{
		pPanel = new CUpdaterDebugDialog(GetVPanel());
	}
	else if (Q_strcmp(GAMEUI_UPDATE_NOTIF, pszName) == 0)
	{
		pPanel = new CUpdateNotificationDialog(GetVPanel());
	}
#endif

	if (pPanel)
	{
		vgui2::Panel *p = dynamic_cast<vgui2::Panel *>(pPanel);
		Assert(p);	// pPanel must inherit from vgui2::Panel
		p->SetProportional(false);
		p->MakePopup();
	}

	g_bIsCreatingGameUIPanel = false;
	return pPanel;
}

IGameUIPanel *CGameUIViewport::FindPanelByName(const char *pszName)
{
	for (IGameUIPanel *i : m_Panels)
	{
		if (!strcmp(i->GetName(), pszName))
		{
			return i;
		}
	}

	return nullptr;
}

void CGameUIViewport::AddPanel(IGameUIPanel *pPanel)
{
	Assert(pPanel);

	for (IGameUIPanel *i : m_Panels)
	{
		if (i == pPanel)
		{
			Assert(!("pPanel already exists"));
			Error("CGameUIViewport::AddPanel: panel %s already exists", pPanel->GetName());
			return;
		}
	}

	m_Panels.push_back(pPanel);
}

bool CGameUIViewport::DeletePanel(const char *pszName)
{
	for (int i = 0; i < m_Panels.size(); i++)
	{
		if (!strcmp(m_Panels[i]->GetName(), pszName))
		{
			delete m_Panels[i];
			m_Panels.erase(m_Panels.begin() + i);
			return true;
		}
	}

	return false;
}

bool CGameUIViewport::DeletePanel(IGameUIPanel *pPanel)
{
	for (int i = 0; i < m_Panels.size(); i++)
	{
		if (m_Panels[i] == pPanel)
		{
			delete m_Panels[i];
			m_Panels.erase(m_Panels.begin() + i);
			return true;
		}
	}

	return false;
}
