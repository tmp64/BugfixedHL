#include "CScorePanel.h"
#include "vgui2/CClientVGUI.h"
#include "vgui2/VGUI2Paths.h"
#include "vgui2/CBaseViewport.h"
#include <vgui_controls/Label.h>
#include <CPlayerListPanel.h>
#include <KeyValues.h>
#include <vgui_controls/BuildModeDialog.h>
#include <string>
#include <locale>
#include <codecvt>
#include <set>
#include "hud.h"
#include "cl_util.h"

#ifdef CSCOREBOARD_DEBUG
#define DebugPrintf ConPrintf
#else
#define DebugPrintf
#endif

//--------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------
CScorePanel::CScorePanel(IViewport *pParent) : BaseClass(nullptr, "ScorePanel"),
												m_pViewport(pParent)
{
	SetTitle("", true);
	SetCloseButtonVisible(false);
	SetScheme("GameScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);

	// Header labels
	m_pServerNameLabel = new vgui2::Label(this, "ServerName", "A Half-Life Server");
	m_pMapNameLabel = new vgui2::Label(this, "MapName", "Map: crossfire");
	m_pPlayerCountLabel = new vgui2::Label(this, "PlayerCount", "2/32");

	// Player list
	m_pPlayerList = new CPlayerListPanel(this, "PlayerList");
	m_pPlayerList->SetVerticalScrollbar(false);

	LoadControlSettings(UI_RESOURCE_DIR "/ScorePanel.res");
	InvalidateLayout();
	SetVisible(false);

	//ActivateBuildMode();
}

CScorePanel::~CScorePanel()
{
}

//--------------------------------------------------------------
// VGUI overrides and etc
//--------------------------------------------------------------
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

//--------------------------------------------------------------
// Completeley resets the scoreboard and recalculates everything
//--------------------------------------------------------------
void CScorePanel::FullUpdate()
{
	DebugPrintf("CScoreBoard::FullUpdate: Full update called\n");
	UpdateServerName();
	UpdateMapName();
	RecalcItems();
	Resize();
}

//--------------------------------------------------------------
// Creates sections for teams in right order and fills them with players
//--------------------------------------------------------------
void CScorePanel::RecalcItems()
{
	m_pPlayerList->DeleteAllItems();
	m_pPlayerList->RemoveAllSections();
	m_iPlayerCount = 0;
	memset(m_pTeamInfo, 0, sizeof(m_pTeamInfo));
	memset(m_pClientItems, -1, sizeof(m_pClientItems));
	memset(m_pClientTeams, 0, sizeof(m_pClientTeams));
	AddHeader();
	DebugPrintf("CScorePanel::RecalItems: Full resorting\n");
	// Fill team info from player info
	// FIXME: Use overriden values in g_TeamInfo if need to
	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		GetPlayerInfo(i, &g_PlayerInfoList[i]);
		if (!g_PlayerInfoList[i].name) continue; // Player is not connected
		int team = g_PlayerExtraInfo[i].teamnumber;
		m_pClientTeams[i] = team;
		if (!m_pTeamInfo[team].name[0]) strncpy(m_pTeamInfo[team].name, g_PlayerExtraInfo[i].teamname, MAX_TEAM_NAME);
		m_pTeamInfo[team].name[MAX_TEAM_NAME - 1] = '\0';
		m_pTeamInfo[team].players++;
		m_pTeamInfo[team].kills += g_PlayerExtraInfo[i].frags;
		m_pTeamInfo[team].deaths += g_PlayerExtraInfo[i].deaths;
	}

	// Sort teams
	auto cmp = [this](int lhs, int rhs)
	{
		// Comapre kills
		if (m_pTeamInfo[lhs].kills > m_pTeamInfo[rhs].kills) return true;
		else if (m_pTeamInfo[lhs].kills < m_pTeamInfo[rhs].kills) return false;

		// Comapre deaths if kills are equal
		if (m_pTeamInfo[lhs].deaths < m_pTeamInfo[rhs].deaths) return true;
		else if (m_pTeamInfo[lhs].deaths > m_pTeamInfo[rhs].deaths) return false;

		// Comapre idx if everything is equal
		return lhs > rhs;
	};
	std::set<int, decltype(cmp)> set(cmp);

	for (int i = 1; i <= MAX_TEAMS; i++) if (m_pTeamInfo[i].players > 0) set.insert(i);
	
	// Create sections in right order
	for (int team : set)
	{
		char buf[16];
		if (team == m_pHeader) continue;
		m_pPlayerList->AddSection(team, "", StaticPlayerSortFunc);
		m_pPlayerList->AddColumnToSection(team, "name", m_pTeamInfo[team].name, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH));
		m_pPlayerList->AddColumnToSection(team, "steamid", "", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), STEAMID_WIDTH));
		snprintf(buf, sizeof(buf), "%.2f", (double)m_pTeamInfo[team].kills / (double)(m_pTeamInfo[team].deaths + 1));
		m_pPlayerList->AddColumnToSection(team, "eff", buf, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
		snprintf(buf, sizeof(buf), "%d", m_pTeamInfo[team].kills);
		m_pPlayerList->AddColumnToSection(team, "frags", buf, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
		snprintf(buf, sizeof(buf), "%d", m_pTeamInfo[team].deaths);
		m_pPlayerList->AddColumnToSection(team, "deaths", buf, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
		m_pPlayerList->AddColumnToSection(team, "ping", "", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), PING_WIDTH));
		m_pPlayerList->SetSectionFgColor(team, gHUD.GetTeamColor(team));
		DebugPrintf("CScorePanel::RecalItems Team '%s' is %d\n", m_pTeamInfo[team].name, team);
	}

	// Add players to sections
	UpdateAllClients();
}

//--------------------------------------------------------------
// Creates or updates client's row in the list
//--------------------------------------------------------------
void CScorePanel::UpdateClientInfo(int client, bool autoUpdate)
{
	GetPlayerInfo(client, &g_PlayerInfoList[client]);
	if (!g_PlayerInfoList[client].name)
	{
		// Player is not connected
		if (m_pClientItems[client] != -1)
		{
			m_pPlayerList->RemoveItem(m_pClientItems[client]);
			m_iPlayerCount--;
			DebugPrintf("CScorePanel::UpdateClientInfo: client %d removed\n", client);
		}
		return;
	}
	
	int team = g_PlayerExtraInfo[client].teamnumber;

	if (m_pClientTeams[client] != team)
	{
		// Player changed teams
		// Remove existing item, code below will recreate it in the right team
		m_pPlayerList->RemoveItem(m_pClientItems[client]);
		m_pClientItems[client] = -1;
		m_pClientTeams[client] = team;
		DebugPrintf("CScorePanel::UpdateClientInfo: client %d changed teams\n", client);
	}

	char buf[16];
	KeyValues *playerData = new KeyValues("data");
	playerData->SetString("name", RemoveColorCodes(g_PlayerInfoList[client].name));
	playerData->SetString("steamid", g_PlayerSteamId[client]);
	snprintf(buf, sizeof(buf), "%.2f", (double)g_PlayerExtraInfo[client].frags / (double)(g_PlayerExtraInfo[client].deaths + 1));
	playerData->SetString("eff", buf);
	playerData->SetInt("frags", g_PlayerExtraInfo[client].frags);
	playerData->SetInt("deaths", g_PlayerExtraInfo[client].deaths);
	playerData->SetInt("ping", g_PlayerInfoList[client].ping);

	if (m_pClientItems[client] == -1)
	{
		// Create new item
		m_pClientItems[client] = m_pPlayerList->AddItem(team, playerData);
		m_pPlayerList->SetItemFgColor(m_pClientItems[client], gHUD.GetTeamColor(team));
		m_iPlayerCount++;
		DebugPrintf("CScorePanel::UpdateClientInfo: client %d added\n", client);
	}
	else
	{
		// Modify existing item
		m_pPlayerList->ModifyItem(m_pClientItems[client], team, playerData);
		DebugPrintf("CScorePanel::UpdateClientInfo: client %d modified\n", client);
	}
	
	if (g_PlayerInfoList[client].thisplayer) m_pPlayerList->SetSelectedItem(m_pClientItems[client]);
	playerData->deleteThis();

	if (autoUpdate)
	{
		UpdatePlayerCount();
		Resize();
	}
}

//--------------------------------------------------------------
// Update all players (also removes disconnected players)
//--------------------------------------------------------------
void CScorePanel::UpdateAllClients()
{
	DebugPrintf("CScorePanel::UpdateAllClients() called\n");
	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		UpdateClientInfo(i, false);
	}
	UpdatePlayerCount();
	Resize();
}

//--------------------------------------------------------------
// Copies server name from TeamFortressViewport to the label
//--------------------------------------------------------------
void CScorePanel::UpdateServerName()
{
	// TODO: Find a better way to convert UTF-8 -> wchar_t *
	if (gHUD.GetServerName())
	{
		std::wstring_convert<std::codecvt_utf8 <wchar_t>, wchar_t> convert;
		std::wstring dest = convert.from_bytes(gHUD.GetServerName());
		m_pServerNameLabel->SetText(dest.c_str());
	}
}

void CScorePanel::UpdateMapName()
{
	std::wstring_convert<std::codecvt_utf8 <wchar_t>, wchar_t> convert;
	std::wstring str = L"Map: " + convert.from_bytes(gEngfuncs.pfnGetLevelName()).substr(5);	// substr is to remove "maps/" before mapname.bsp
	m_pMapNameLabel->SetText(str.c_str());
}

//--------------------------------------------------------------
// Updates player count label using calculated in RecalcTeams() amount of players online and info from the engine
//--------------------------------------------------------------
void CScorePanel::UpdatePlayerCount()
{
	wchar_t buf[32];
	swprintf(buf, L"%d/%d", m_iPlayerCount, gEngfuncs.GetMaxClients());	// May be unsafe, but 32 chars should be enough
	m_pPlayerCountLabel->SetText(buf);
}

//--------------------------------------------------------------
// Creates the header section
//--------------------------------------------------------------
void CScorePanel::AddHeader()
{
	m_pPlayerList->AddSection(m_pHeader, "", StaticPlayerSortFunc);
	m_pPlayerList->SetSectionAlwaysVisible(m_pHeader);
	m_pPlayerList->AddColumnToSection(m_pHeader, "name", "#PlayerName", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "steamid", "Steam ID", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), STEAMID_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "eff", "Eff", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "frags", "#PlayerScore", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "deaths", "#PlayerDeath", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "ping", "#PlayerPing", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), PING_WIDTH));
}

//--------------------------------------------------------------
// Resizes the scoreboard to fit all players
//--------------------------------------------------------------
void CScorePanel::Resize()
{
	int wide, tall, x, y;
	int height = 0, listHeight = 0, addHeight = 0;
	m_pPlayerList->GetPos(x, y);
	addHeight = y + vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), 4);	// Header + bottom padding // TODO: Get padding in runtime
	height += addHeight;
	m_pPlayerList->GetContentSize(wide, tall);
	listHeight = max(m_iMinHeight, tall);
	height += listHeight;

	if (ScreenHeight - height < m_iMargin * 2)
	{
		// It didn't fit
		height = ScreenHeight - m_iMargin * 2;
		listHeight = height - addHeight;
		m_pPlayerList->SetVerticalScrollbar(true);
	}
	else
	{
		m_pPlayerList->SetVerticalScrollbar(false);
	}
	m_pPlayerList->GetSize(wide, tall);
	m_pPlayerList->SetSize(wide, listHeight);
	GetSize(wide, tall);
	SetSize(wide, height);

	// Move to center
	GetPos(x, y);
	y = (ScreenHeight - height) / 2;
	SetPos(x, y);
}

//--------------------------------------------------------------
// Sorting functions
//--------------------------------------------------------------
bool CScorePanel::StaticPlayerSortFunc(CPlayerListPanel *list, int itemID1, int itemID2)
{
	KeyValues *it1 = list->GetItemData(itemID1);
	KeyValues *it2 = list->GetItemData(itemID2);
	Assert(it1 && it2);

	// first compare frags
	int v1 = it1->GetInt("frags");
	int v2 = it2->GetInt("frags");
	if (v1 > v2)
		return true;
	else if (v1 < v2)
		return false;

	// next compare deaths
	v1 = it1->GetInt("deaths");
	v2 = it2->GetInt("deaths");
	if (v1 > v2)
		return false;
	else if (v1 < v2)
		return true;

	// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
	return itemID1 < itemID2;
}
