#ifndef CSCOREPANEL_H
#define CSCOREPANEL_H

#include <string>
#include <unordered_map>
#include <functional>
#include <cdll_dll.h>
#include <vgui_controls/Frame.h>
#include "vgui2/IViewportPanel.h"
#include "vgui2/ViewportPanelNames.h"
#include <tier1/UtlMap.h>
#include <steam/steam_api.h>
#include "CPlayerListPanel.h"

class IViewport;
class CHudScoreBoard;

namespace vgui2
{
	class Label;
	class Menu;
	class CheckButton;
}

class CPlayerListPanel;
class CPngImage;
class CAvatarImage;

class CScorePanel : public vgui2::Frame, public IViewportPanel
{
public:
	DECLARE_CLASS_SIMPLE(CScorePanel, Frame);

	// column widths at 640
	enum
	{
		AVATAR_OFFSET = 4,
		AVATAR_OFF_WIDTH = 22,
		NAME_WIDTH = 184,
		STEAMID_WIDTH = 100,
		EFF_WIDTH = 60,
		SCORE_WIDTH = 60,
		DEATH_WIDTH = 60,
		PING_WIDTH = 80
	};
	// total = 544

public:
	CScorePanel(IViewport *pParent);
	virtual ~CScorePanel();

	void InitHudData();
	void FullUpdate();
	void UpdateClientInfo(int client, bool autoUpdate = true);	// autoUpdate - whether to update player count and resize at the end of client update
	void UpdateAllClients();
	void EnableMousePointer(bool enable);
	void UpdateServerName();
	void DeathMsg(int killer, int victim);

	//IViewportPanel overrides
	const char *GetName() override
	{
		return VIEWPORT_PANEL_SCORE;
	}

	void SetData(KeyValues *data) override {}
	void Reset() override;
	void Update() override {}
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);
	virtual void OnKeyCodeTyped(vgui2::KeyCode code);
	virtual void OnThink();

	bool NeedsUpdate() override
	{
		return false;
	}

	bool HasInputElements() override
	{
		return true;
	}

	void ShowPanel(bool state) override;

	// VGUI functions:
	vgui2::VPANEL GetVPanel() override final
	{
		return BaseClass::GetVPanel();
	}

	bool IsVisible() override final
	{
		return BaseClass::IsVisible();
	}

	void SetParent(vgui2::VPANEL parent) override final
	{
		BaseClass::SetParent(parent);
	}

	// Messages
	virtual void OnCommand(const char *command) override;
	MESSAGE_FUNC_INT(OnItemContextMenu, "ItemContextMenu", itemID);
	MESSAGE_FUNC_INT(OnCheckButtonChecked, "CheckButtonChecked", state);

	void MsgFunc_TeamScore(const char *teamName, int frags, int deaths);

	friend class CHudScoreBoard;

private:
	struct team_info_t
	{
		int kills = 0;	// Calculated kills
		int deaths = 0;	// Calculated deaths
		int players = 0;// Number of players
	};

	struct menu_info_t
	{
		// Global data
		int muteItemID;
		int profilePageItemID;
		int profileUrlItemID;

		// Selected player info
		int itemID;
		int client;
		long long steamID64;
	};

	struct team_score_t
	{
		int frags = 0;
		int deaths = 0;
	};
	IViewport *m_pViewport;
	CPlayerListPanel *m_pPlayerList = nullptr;
	vgui2::Label *m_pServerNameLabel = nullptr;
	vgui2::Label *m_pMapNameLabel = nullptr;
	vgui2::Label *m_pPlayerCountLabel = nullptr;
	vgui2::Label *m_pTimerLabel = nullptr;
	vgui2::Menu *m_pMenu = nullptr;
	vgui2::CheckButton *m_pEffSortSwitch = nullptr;
	CPngImage *m_pMutedIcon = nullptr;

#ifdef CSCOREBOARD_DEBUG
	vgui2::Label *m_pLastUpdateLabel = nullptr;
#endif

	int m_pHeader = 0;
	int m_iPlayerCount = 0;
	int m_iMargin = 100;
	int m_iMinHeight = 320;
	int m_iMutedIconIndex = 0;
	int m_iSpectatorSection = -1;

	vgui2::ImageList *m_pImageList;
	CUtlMap<CSteamID, int> m_mapAvatarsToImageList;
	int m_iAvatarPaddingLeft = 0, m_iAvatarPaddingRight = 0, m_iAvatarWidth = 0;

	team_info_t m_pTeamInfo[MAX_TEAMS + 1];
	int m_pClientItems[MAX_PLAYERS + 1];
	int m_pClientTeams[MAX_PLAYERS + 1];
	std::unordered_map<std::string, team_score_t> m_mTeamNameToScore;

	SDK_Color m_ThisPlayerBgColor = SDK_Color(0, 0, 0, 0);
	SDK_Color m_KillerBgColor = SDK_Color(0, 0, 0, 0);
	int m_iKillerIndex = -1;
	float m_flKillerHighlightStart = 0;

	menu_info_t m_pMenuInfo;

	void RecalcItems();
	void UpdateMapName();
	void UpdatePlayerCount();
	void AddHeader();
	void Resize();
	void UpdatePlayerAvatar(int playerIndex, KeyValues *kv);
	void UpdateTeamScores();
	void UpdateTeamScore(int i);
	int GetLineSpacingForHeight(int h);
	int GetAvatarSize();

	int GetLineSpacingForNormal();
	int GetLineSpacingForCompact();

	// Mode:
	// - 0 -- compact if overflows
	// - 1 -- always big
	// - 2 -- always compact
	int GetSizeMode();

	// Menu
	void CreatePlayerMenu();
	void OpenPlayerMenu(int itemID);

	// Extra controls
	void ShowExtraControls();
	void HideExtraControls();

	// Sorting functions
	CPlayerListPanel::SectionSortFunc_t m_pPlayerSortFunction = nullptr;
	std::function<bool(int lhs, int rhs)> m_pTeamSortFunction;

	void SetSortByFrags();
	void SetSortByEff();

	SDK_Color GetPlayerBgColor(int idx);

	static bool StaticPlayerSortFuncByFrags(CPlayerListPanel *list, int itemID1, int itemID2);
	static bool StaticPlayerSortFuncByEff(CPlayerListPanel *list, int itemID1, int itemID2);
};

#endif