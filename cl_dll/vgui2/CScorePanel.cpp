#include <string>
#include <locale>
#include <codecvt>
#include <set>
#include "vgui2/CClientVGUI.h"
#include "vgui2/VGUI2Paths.h"
#include "vgui2/CBaseViewport.h"
#include "CScorePanel.h"
#include "CPlayerListPanel.h"
#include "CVoiceStatus2.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/BuildModeDialog.h>
#include <vgui_controls/ImageList.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include "hud.h"
#include "cl_util.h"
#include "cl_dll.h"
#include "demo_api.h"
#include "CHudTextMessage.h"
#include "clientsteamcontext.h"
#include "CAvatarImage.h"
#include "CHudScoreBoard.h"

#ifdef CSCOREBOARD_DEBUG
#define DebugPrintf ConPrintf
#else
#define DebugPrintf
#endif

#define STEAM_PROFILE_URL "http://steamcommunity.com/profiles/"
#define PING "#PlayerPing"
#define PING_LOSS "Ping/Loss"

extern int g_iVisibleMouse;
void IN_ResetMouse(void);
CScorePanel *CScorePanel::m_sSingleton = nullptr;

//--------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------
CScorePanel::CScorePanel(IViewport *pParent) : BaseClass(nullptr, VIEWPORT_PANEL_SCORE),
												m_pViewport(pParent)
{
	m_sSingleton = this;
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
	m_pTimerLabel = new vgui2::Label(this, "TimeLeft", "Time left: 22:48");

	// Player list
	m_pPlayerList = new CPlayerListPanel(this, "PlayerList");
	m_pPlayerList->SetVerticalScrollbar(false);

	CreatePlayerMenu();

	LoadControlSettings(UI_RESOURCE_DIR "/ScorePanel.res");
	InvalidateLayout();
	SetVisible(false);
	EnableMousePointer(false);
	//ActivateBuildMode();

	m_pImageList = NULL;

	m_mapAvatarsToImageList.SetLessFunc(DefLessFunc(CSteamID));
	m_mapAvatarsToImageList.RemoveAll();
}

CScorePanel::~CScorePanel()
{
	if (NULL != m_pImageList)
	{
		delete m_pImageList;
		m_pImageList = NULL;
	}
}

//--------------------------------------------------------------
// VGUI overrides and etc
//--------------------------------------------------------------
void CScorePanel::Reset()
{
}

void CScorePanel::ApplySchemeSettings(vgui2::IScheme * pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_pPlayerList->SetBorder(pScheme->GetBorder("FrameBorder"));
	m_pTimerLabel->SetVisible(false);

	if (m_pImageList)
		delete m_pImageList;
	m_pImageList = new vgui2::ImageList(false);

	m_mapAvatarsToImageList.RemoveAll();
	m_iAvatarPaddingLeft = m_iAvatarPaddingRight = vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), AVATAR_OFFSET);

	// resize the images to our resolution
	for (int i = 0; i < m_pImageList->GetImageCount(); i++)
	{
		int wide, tall;
		m_pImageList->GetImage(i)->GetSize(wide, tall);
		m_pImageList->GetImage(i)->SetSize(vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), wide), vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), tall));
	}

	m_pPlayerList->SetImageList(m_pImageList, false);
	m_pPlayerList->SetVisible(true);
}

void CScorePanel::OnKeyCodeTyped(vgui2::KeyCode code)
{
	BaseClass::OnKeyCodeTyped(code);
}

void CScorePanel::ShowPanel(bool state)
{
	if (m_pImageList == NULL)
	{
		InvalidateLayout(true, true);
	}

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
		EnableMousePointer(false);
		SetKeyBoardInputEnabled(false);
		m_pMenu->SetVisible(false);
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
// Shows mouse cursor when mouse button is cliecked
//--------------------------------------------------------------
void CScorePanel::EnableMousePointer(bool enable)
{
	if (enable && !IsVisible()) return;
	dynamic_cast<vgui2::EditablePanel *>(m_pViewport)->SetMouseInputEnabled(enable);
	dynamic_cast<vgui2::EditablePanel *>(m_pViewport)->SetKeyBoardInputEnabled(enable);
	SetMouseInputEnabled(enable);
	SetKeyBoardInputEnabled(enable);
	if (!enable)
	{
		if (!gEngfuncs.pDemoAPI->IsPlayingback())
		{
			IN_ResetMouse();
		}
	}
	g_iVisibleMouse = enable;
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

	int spectatorCount = 0;
	int spectatorFrags = 0;
	int spectatorDeaths = 0;
	int totalPlayerCount = 0;
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
		if (g_IsSpectator[i])
		{
			spectatorCount++;
			spectatorFrags += g_PlayerExtraInfo[i].frags;
			spectatorDeaths += g_PlayerExtraInfo[i].deaths;
		}
		totalPlayerCount++;
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
		char buf[64];
		if (team == m_pHeader) continue;
		m_pPlayerList->AddSection(team, "", StaticPlayerSortFunc);
		if (gHUD.m_ScoreBoard->m_CvarAvatars->value)
			m_pPlayerList->AddColumnToSection(team, "avatar", "", CPlayerListPanel::COLUMN_IMAGE, m_iAvatarWidth + m_iAvatarPaddingLeft + m_iAvatarPaddingRight);
		snprintf(buf, sizeof(buf), "%s (%d/%d, %.0f%%)", m_pTeamInfo[team].name, m_pTeamInfo[team].players, totalPlayerCount, (double)m_pTeamInfo[team].players / totalPlayerCount * 100.0);
		m_pPlayerList->AddColumnToSection(team, "name", buf, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH));
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

	// Add spectator section
	if (spectatorCount != 0)
	{
		char buf[64];
		m_pPlayerList->AddSection(m_pSpectatorSection, "", StaticPlayerSortFunc);
		if (gHUD.m_ScoreBoard->m_CvarAvatars->value)
			m_pPlayerList->AddColumnToSection(m_pSpectatorSection, "avatar", "", CPlayerListPanel::COLUMN_IMAGE, m_iAvatarWidth + m_iAvatarPaddingLeft + m_iAvatarPaddingRight);
		snprintf(buf, sizeof(buf), "Spectators (%d/%d, %.0f%%)", spectatorCount, totalPlayerCount, (double)spectatorCount / totalPlayerCount * 100.0);
		m_pPlayerList->AddColumnToSection(m_pSpectatorSection, "name", buf, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH));
		m_pPlayerList->AddColumnToSection(m_pSpectatorSection, "steamid", "", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), STEAMID_WIDTH));
		snprintf(buf, sizeof(buf), "%.2f", (double)spectatorFrags / (double)(spectatorDeaths + 1));
		m_pPlayerList->AddColumnToSection(m_pSpectatorSection, "eff", buf, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
		snprintf(buf, sizeof(buf), "%d", spectatorFrags);
		m_pPlayerList->AddColumnToSection(m_pSpectatorSection, "frags", buf, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
		snprintf(buf, sizeof(buf), "%d", spectatorDeaths);
		m_pPlayerList->AddColumnToSection(m_pSpectatorSection, "deaths", buf, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
		m_pPlayerList->AddColumnToSection(m_pSpectatorSection, "ping", "", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), PING_WIDTH));
		m_pPlayerList->SetSectionFgColor(m_pSpectatorSection, gHUD.GetTeamColor(0));
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
	
	int team;
	if (g_IsSpectator[client])
		team = m_pSpectatorSection;
	else
		team = g_PlayerExtraInfo[client].teamnumber;

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
	if (gHUD.m_ScoreBoard->m_CvarAvatars->value)
		UpdatePlayerAvatar(client, playerData);
	playerData->SetInt("client", client);
	playerData->SetString("name", RemoveColorCodes(g_PlayerInfoList[client].name));
	playerData->SetString("steamid", g_PlayerSteamId[client]);
	snprintf(buf, sizeof(buf), "%.2f", (double)g_PlayerExtraInfo[client].frags / (double)(g_PlayerExtraInfo[client].deaths + 1));
	playerData->SetString("eff", buf);
	playerData->SetInt("frags", g_PlayerExtraInfo[client].frags);
	playerData->SetInt("deaths", g_PlayerExtraInfo[client].deaths);
	if (gHUD.m_ScoreBoard->m_Cvar_Loss->value)
	{
		snprintf(buf, sizeof(buf), "%d/%d", g_PlayerInfoList[client].ping, g_PlayerInfoList[client].packetloss);
		playerData->SetString("ping", buf);
	}
	else playerData->SetInt("ping", g_PlayerInfoList[client].ping);
	if (g_PlayerInfoList[client].thisplayer) playerData->SetInt("thisPlayer", 1);

	if (m_pClientItems[client] == -1)
	{
		// Create new item
		m_pClientItems[client] = m_pPlayerList->AddItem(team, playerData);
		m_pPlayerList->SetItemFgColor(m_pClientItems[client], gHUD.GetTeamColor( g_IsSpectator[client] ? 0 : team ));
		m_iPlayerCount++;
		DebugPrintf("CScorePanel::UpdateClientInfo: client %d added\n", client);
	}
	else
	{
		// Modify existing item
		m_pPlayerList->ModifyItem(m_pClientItems[client], team, playerData);
		DebugPrintf("CScorePanel::UpdateClientInfo: client %d modified\n", client);
	}
	
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
	if (gHUD.GetServerName())
	{
		wchar_t str[64];
		vgui2::localize()->ConvertANSIToUnicode(gHUD.GetServerName(), str, sizeof(str));
		m_pServerNameLabel->SetText(str);
	}
}

void CScorePanel::UpdateMapName()
{
	try
	{
		std::wstring_convert<std::codecvt_utf8 <wchar_t>, wchar_t> convert;
		std::wstring mapfile = convert.from_bytes(gEngfuncs.pfnGetLevelName());
		std::wstring str = L"Map: " + mapfile.substr(5, mapfile.size() - 5 - 4);	// substr is to remove "maps/" before mapname and ".bsp" after
		m_pMapNameLabel->SetText(str.c_str());
	}
	catch (const std::exception &)
	{
		m_pMapNameLabel->SetText(L"Error! Invalid map name");
	}
}

//--------------------------------------------------------------
// Updates player count label using calculated in RecalcTeams() amount of players online and info from the engine
//--------------------------------------------------------------
void CScorePanel::UpdatePlayerCount()
{
	wchar_t buf[32];
#ifdef _WIN32
	swprintf(buf, // May be unsafe, but 32 chars should be enough
#else
	swprintf(buf, sizeof(buf),
#endif
		L"%d/%d", m_iPlayerCount, gEngfuncs.GetMaxClients()
	);	
	m_pPlayerCountLabel->SetText(buf);
}

//--------------------------------------------------------------
// Creates the header section
//--------------------------------------------------------------
void CScorePanel::AddHeader()
{
	m_pPlayerList->AddSection(m_pHeader, "", StaticPlayerSortFunc);
	m_pPlayerList->SetSectionAlwaysVisible(m_pHeader);
	if (gHUD.m_ScoreBoard->m_CvarAvatars->value)
		m_pPlayerList->AddColumnToSection(m_pHeader, "avatar", "", CPlayerListPanel::COLUMN_IMAGE, m_iAvatarWidth + m_iAvatarPaddingLeft + m_iAvatarPaddingRight);
	m_pPlayerList->AddColumnToSection(m_pHeader, "name", "#PlayerName", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), NAME_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "steamid", "Steam ID", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), STEAMID_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "eff", "Eff", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "frags", "#PlayerScore", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), SCORE_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "deaths", "#PlayerDeath", CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), DEATH_WIDTH));
	m_pPlayerList->AddColumnToSection(m_pHeader, "ping", gHUD.m_ScoreBoard->m_Cvar_Loss->value ? PING_LOSS : PING, CPlayerListPanel::COLUMN_BRIGHT, vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), PING_WIDTH));
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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CScorePanel::UpdatePlayerAvatar(int playerIndex, KeyValues *kv)
{
	// Update their avatar
	if (kv && ClientSteamContext().SteamFriends() && ClientSteamContext().SteamUtils())
	{
		uint64 steamID64 = GetSteamID64(g_PlayerSteamId[playerIndex]);
		if (steamID64)
		{
			CSteamID steamIDForPlayer(steamID64);

			// See if we already have that avatar in our list
			int iMapIndex = m_mapAvatarsToImageList.Find(steamIDForPlayer);
			int iImageIndex;
			if (iMapIndex == m_mapAvatarsToImageList.InvalidIndex())
			{
				CAvatarImage *pImage = new CAvatarImage();
				pImage->SetOffset(m_iAvatarPaddingLeft, 0);
				pImage->SetDrawFriend(false);
				pImage->SetAvatarSteamID(steamIDForPlayer);
				pImage->SetAvatarSize(32, 32);	// Deliberately non scaling
				iImageIndex = m_pImageList->AddImage(pImage);

				m_mapAvatarsToImageList.Insert(steamIDForPlayer, iImageIndex);
			}
			else
			{
				iImageIndex = m_mapAvatarsToImageList[iMapIndex];
			}

			kv->SetInt("avatar", iImageIndex);

			CAvatarImage *pAvIm = (CAvatarImage *)m_pImageList->GetImage(iImageIndex);
			pAvIm->UpdateFriendStatus();
		}
	}
}

//--------------------------------------------------------------
// Player context menu
//--------------------------------------------------------------
void CScorePanel::CreatePlayerMenu()
{
	m_pMenu = new vgui2::Menu(this, nullptr);
	m_pMenu->SetVisible(false);
	m_pMenu->AddActionSignalTarget(this);
	m_pMenuInfo.muteItemID = m_pMenu->AddMenuItem("Mute", "Mute", "MenuMute", this);
	m_pMenu->AddSeparator();
	m_pMenuInfo.profilePageItemID = m_pMenu->AddMenuItem("SteamProfile", "Open Steam profile", "MenuSteamProfile", this);
	m_pMenuInfo.profileUrlItemID = m_pMenu->AddMenuItem("SteamURL", "Copy profile URL", "MenuSteamURL", this);
	m_pMenu->AddSeparator();
	m_pMenu->AddMenuItem("CopyName", "Copy nickname", "MenuCopyName", this);
	m_pMenu->AddMenuItem("CopyNameRaw", "Copy raw nickname", "MenuCopyNameRaw", this);
	m_pMenu->AddMenuItem("CopySteamID", "Copy Steam ID", "MenuCopySteamID", this);
	m_pMenu->AddMenuItem("CopySteamID64", "Copy Steam ID 64", "MenuCopySteamID64", this);
}

void CScorePanel::OpenPlayerMenu(int itemID)
{
	// Set menu info
	m_pMenuInfo.itemID = itemID;
	m_pMenuInfo.client = 0;
	KeyValues *kv = m_pPlayerList->GetItemData(itemID);
	if (!kv) return;
	m_pMenuInfo.client = kv->GetInt("client", 0);
	if (m_pMenuInfo.client == 0) return;

	// SteamID64
	m_pMenuInfo.steamID64 = GetSteamID64(g_PlayerSteamId[m_pMenuInfo.client]);
	if (m_pMenuInfo.steamID64 != 0)
	{
		m_pMenu->SetItemEnabled(m_pMenuInfo.profilePageItemID, true);
		m_pMenu->SetItemEnabled(m_pMenuInfo.profileUrlItemID, true);
	}
	else
	{
		m_pMenu->SetItemEnabled(m_pMenuInfo.profilePageItemID, false);
		m_pMenu->SetItemEnabled(m_pMenuInfo.profileUrlItemID, false);
	}

	// Player muting
	bool thisPlayer = kv->GetInt("thisplayer", 0);
	if (thisPlayer)
	{
		// Can't mute yourself
		m_pMenu->UpdateMenuItem(m_pMenuInfo.muteItemID, "Mute", new KeyValues("Command", "command", "MenuMute"));
		m_pMenu->SetItemEnabled(m_pMenuInfo.muteItemID, false);
	}
	else
	{
		m_pMenu->SetItemEnabled(m_pMenuInfo.muteItemID, true);
		if (vgui2::voicemgr()->IsPlayerBlocked(m_pMenuInfo.client))
		{
			m_pMenu->UpdateMenuItem(m_pMenuInfo.muteItemID, "Unmute", new KeyValues("Command", "command", "MenuMute"));
		}
		else
		{
			m_pMenu->UpdateMenuItem(m_pMenuInfo.muteItemID, "Mute", new KeyValues("Command", "command", "MenuMute"));
		}
	}

	// Open menu
	// Code copied from vgui2::TextEntry
	int cursorX, cursorY;
	vgui2::input()->GetCursorPos(cursorX, cursorY);
	m_pMenu->SetVisible(true);
	m_pMenu->RequestFocus();

	// relayout the menu immediately so that we know it's size
	m_pMenu->InvalidateLayout(true);
	int menuWide, menuTall;
	m_pMenu->GetSize(menuWide, menuTall);

	// work out where the cursor is and therefore the best place to put the menu
	int wide, tall;
	vgui2::surface()->GetScreenSize(wide, tall);

	if (wide - menuWide > cursorX)
	{
		// menu hanging right
		if (tall - menuTall > cursorY)
		{
			// menu hanging down
			m_pMenu->SetPos(cursorX, cursorY);
		}
		else
		{
			// menu hanging up
			m_pMenu->SetPos(cursorX, cursorY - menuTall);
		}
	}
	else
	{
		// menu hanging left
		if (tall - menuTall > cursorY)
		{
			// menu hanging down
			m_pMenu->SetPos(cursorX - menuWide, cursorY);
		}
		else
		{
			// menu hanging up
			m_pMenu->SetPos(cursorX - menuWide, cursorY - menuTall);
		}
	}

	m_pMenu->RequestFocus();
}

void CScorePanel::OnItemContextMenu(int itemID)
{
	DebugPrintf("CScorePanel::OnRowContextMenu(itemID = %d) Player right-clicked\n", itemID);
	OpenPlayerMenu(itemID);
}

//--------------------------------------------------------------
// Commands
//--------------------------------------------------------------
void CScorePanel::OnCommandOverride(const char *command)
{
	DebugPrintf("CScorePanel::OnCommand(command = '%s')\n", command);
	//-----------------------------------------------------------------------
	if (!stricmp(command, "MenuMute"))
	{
		int client = m_pMenuInfo.client;
		GetPlayerInfo(client, &g_PlayerInfoList[client]);
		if (!g_PlayerInfoList[client].name || !g_PlayerInfoList[client].name[0])
			return;	// Client disconnected
		if (g_PlayerInfoList[client].thisplayer)
			return;	// Can't mute yourself

		char string[256];

		if (vgui2::voicemgr()->IsPlayerBlocked(client))
		{
			char string1[1024];

			// remove mute
			vgui2::voicemgr()->SetPlayerBlockedState(client, false);
			sprintf(string1, CHudTextMessage::BufferedLocaliseTextString("#Unmuted"), g_PlayerInfoList[client].name);
			sprintf(string, "%c** %s\n", HUD_PRINTTALK, string1);
		}
		else
		{
			char string1[1024];
			char string2[1024];

			// mute the player
			vgui2::voicemgr()->SetPlayerBlockedState(client, true);

			sprintf(string1, CHudTextMessage::BufferedLocaliseTextString("#Muted"), g_PlayerInfoList[client].name);
			sprintf(string2, CHudTextMessage::BufferedLocaliseTextString("#No_longer_hear_that_player"));
			sprintf(string, "%c** %s %s\n", HUD_PRINTTALK, string1, string2);
		}
		gHUD.m_TextMessage->MsgFunc_TextMsg(NULL, strlen(string) + 1, string);
	}
	//-----------------------------------------------------------------------
	else if (!stricmp(command, "MenuSteamProfile"))
	{
		if (m_pMenuInfo.steamID64 > 0)
		{
			CSteamID steamId = CSteamID((uint64)m_pMenuInfo.steamID64);
			ClientSteamContext().SteamFriends()->ActivateGameOverlayToUser("steamid", steamId);
		}
	}
	else if (!stricmp(command, "MenuSteamURL"))
	{
		GetPlayerInfo(m_pMenuInfo.client, &g_PlayerInfoList[m_pMenuInfo.client]);
		if (!g_PlayerInfoList[m_pMenuInfo.client].name || !g_PlayerInfoList[m_pMenuInfo.client].name[0])
			return;	// Client disconnected
		std::string url = STEAM_PROFILE_URL + std::to_string(m_pMenuInfo.steamID64);
		vgui2::system()->SetClipboardText(url.c_str(), url.size());
	}
	//-----------------------------------------------------------------------
	else if (!stricmp(command, "MenuCopyName"))
	{
		GetPlayerInfo(m_pMenuInfo.client, &g_PlayerInfoList[m_pMenuInfo.client]);
		if (!g_PlayerInfoList[m_pMenuInfo.client].name || !g_PlayerInfoList[m_pMenuInfo.client].name[0])
			return;	// Client disconnected
		wchar_t name[MAX_PLAYER_NAME + 1];
		vgui2::localize()->ConvertANSIToUnicode(RemoveColorCodes(g_PlayerInfoList[m_pMenuInfo.client].name), name, sizeof(name));
		vgui2::system()->SetClipboardText(name, wcslen(name));
	}
	else if (!stricmp(command, "MenuCopyNameRaw"))
	{
		GetPlayerInfo(m_pMenuInfo.client, &g_PlayerInfoList[m_pMenuInfo.client]);
		if (!g_PlayerInfoList[m_pMenuInfo.client].name || !g_PlayerInfoList[m_pMenuInfo.client].name[0])
			return;	// Client disconnected
		wchar_t name[MAX_PLAYER_NAME + 1];
		vgui2::localize()->ConvertANSIToUnicode(g_PlayerInfoList[m_pMenuInfo.client].name, name, sizeof(name));
		vgui2::system()->SetClipboardText(name, wcslen(name));
	}
	else if (!stricmp(command, "MenuCopySteamID"))
	{
		GetPlayerInfo(m_pMenuInfo.client, &g_PlayerInfoList[m_pMenuInfo.client]);
		if (!g_PlayerInfoList[m_pMenuInfo.client].name || !g_PlayerInfoList[m_pMenuInfo.client].name[0])
			return;	// Client disconnected
		std::string steamid = "STEAM_" + std::string(g_PlayerSteamId[m_pMenuInfo.client]);
		vgui2::system()->SetClipboardText(steamid.c_str(), steamid.size());
	}
	else if (!stricmp(command, "MenuCopySteamID64"))
	{
		GetPlayerInfo(m_pMenuInfo.client, &g_PlayerInfoList[m_pMenuInfo.client]);
		if (!g_PlayerInfoList[m_pMenuInfo.client].name || !g_PlayerInfoList[m_pMenuInfo.client].name[0])
			return;	// Client disconnected
		std::string steamid = std::to_string(m_pMenuInfo.steamID64);
		vgui2::system()->SetClipboardText(steamid.c_str(), steamid.size());
	}
	//-----------------------------------------------------------------------
	else
	{
		BaseClass::OnCommand(command);
	}
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
