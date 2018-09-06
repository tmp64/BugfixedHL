#include "CScorePanel.h"
#include "vgui2/CClientVGUI.h"
#include "vgui2/VGUI2Paths.h"
#include "vgui2/CBaseViewport.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>
#include <KeyValues.h>
#include <vgui_controls/BuildModeDialog.h>

CScorePanel::CScorePanel(IViewport *pParent) : BaseClass(nullptr, "ClientMOTD"),
												m_pViewport(pParent)
{
	SetTitle("", true);
	SetCloseButtonVisible(false);
	SetScheme("GameScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	// Header labels
	m_pServerNameLabel = new vgui2::Label(this, "ServerName", "A Half-Life Server");
	m_pMapNameLabel = new vgui2::Label(this, "MapName", "Map: crossfire");
	m_pPlayerCountLabel = new vgui2::Label(this, "PlayerCount", "2/32");

	// Player list
	m_pPlayerList = new vgui2::SectionedListPanel(this, "PlayerList");
	m_pPlayerList->SetVerticalScrollbar(false);

	LoadControlSettings(UI_RESOURCE_DIR "/ScorePanel.res");
	InvalidateLayout();
	SetVisible(false);

	/*int wide, tall;
	GetSize(wide, tall);
	m_pPlayerList->SetSize(wide, tall);*/

	// Header
	m_pPlayerList->AddSection(m_pHeader, "", StaticPlayerSortFunc);
	m_pPlayerList->SetSectionAlwaysVisible(m_pHeader);
	m_pPlayerList->AddColumnToSection(m_pHeader, "name", "#PlayerName", vgui2::SectionedListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "frags", "#PlayerScore", vgui2::SectionedListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "deaths", "#PlayerDeath", vgui2::SectionedListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "ping", "#PlayerPing", vgui2::SectionedListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), PING_WIDTH));

	for (int section = 1; section < 3; section++)
	{
		m_pPlayerList->AddSection(section, "", StaticPlayerSortFunc);
		m_pPlayerList->SetSectionAlwaysVisible(section);
		m_pPlayerList->AddColumnToSection(section, "name", "barney", vgui2::SectionedListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH));
		m_pPlayerList->AddColumnToSection(section, "frags", "", vgui2::SectionedListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
		m_pPlayerList->AddColumnToSection(section, "deaths", "", vgui2::SectionedListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
		m_pPlayerList->AddColumnToSection(section, "ping", "", vgui2::SectionedListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), PING_WIDTH));

		{
			KeyValues *playerData = new KeyValues("data");
			playerData->SetString("name", "Test 1");
			playerData->SetInt("frags", 10);
			playerData->SetInt("deaths", 3);
			playerData->SetInt("ping", 61);
			int item = m_pPlayerList->AddItem(section, playerData);
			m_pPlayerList->SetItemFgColor(item, SDK_Color(255, 0, 0, 255));
			playerData->deleteThis();
			m_pPlayerList->SetSelectedItem(item);
		}

		{
			KeyValues *playerData = new KeyValues("data");
			playerData->SetString("name", "Test 2");
			playerData->SetInt("frags", 11);
			playerData->SetInt("deaths", 4);
			playerData->SetInt("ping", 31);
			int item = m_pPlayerList->AddItem(section, playerData);
			m_pPlayerList->SetItemFgColor(item, SDK_Color(0, 255, 0, 255));
			playerData->deleteThis();
		}

		{
			KeyValues *playerData = new KeyValues("data");
			playerData->SetString("name", "Test 3");
			playerData->SetInt("frags", 37);
			playerData->SetInt("deaths", 1);
			playerData->SetInt("ping", 100);
			int item = m_pPlayerList->AddItem(section, playerData);
			m_pPlayerList->SetItemFgColor(item, SDK_Color(0, 0, 255, 255));
			playerData->deleteThis();

		}
	}

	ActivateBuildMode();
}

CScorePanel::~CScorePanel()
{
}

void CScorePanel::Reset()
{
	//m_pPlayerList->DeleteAllItems();
	//m_pPlayerList->RemoveAllSections();
}

void CScorePanel::ApplySchemeSettings(vgui2::IScheme * pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	//m_pServerNameLabel->SetFgColor(SDK_Color(255, 255, 255, 255));
	m_pPlayerList->SetBorder(pScheme->GetBorder("FrameBorder"));
}

void CScorePanel::OnKeyCodeTyped(vgui2::KeyCode code)
{
	BaseClass::OnKeyCodeTyped(code);
}

void CScorePanel::ShowPanel(bool state)
{
	if (BaseClass::IsVisible() == state)
		return;

	//m_pViewport->ShowBackGround(state);

	if (state)
	{
		Reset();
		Update();

		BaseClass::Activate();
	}
	else
	{
		BaseClass::SetVisible(false);
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
	}
}

bool CScorePanel::StaticPlayerSortFunc(vgui2::SectionedListPanel *list, int itemID1, int itemID2)
{
	KeyValues *it1 = list->GetItemData(itemID1);
	KeyValues *it2 = list->GetItemData(itemID2);
	Assert(it1 && it2);

	// first compare test value
	int v1 = it1->GetInt("val");
	int v2 = it2->GetInt("val");
	if (v1 > v2)
		return true;
	else if (v1 < v2)
		return false;

	// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
	return itemID1 < itemID2;
}
