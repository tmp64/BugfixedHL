#ifndef CSCOREPANEL_H
#define CSCOREPANEL_H

#include <vgui_controls/Frame.h>
#include "vgui2/IViewportPanel.h"
#include "vgui2/ViewportPanelNames.h"

class IViewport;

namespace vgui2
{
	class Label;
	class TextEntry;
	class SectionedListPanel;
}

class CScorePanel : public vgui2::Frame, public IViewportPanel
{
public:
	DECLARE_CLASS_SIMPLE(CScorePanel, vgui2::Frame);

	// Screw that, we have SectionedListPanel
	/*struct Column
	{
		enum class Type
		{
			PlayerName, SteamId, Kills, Deaths, Ping
		};

		Type type;			// Defines data that will be displayed in the column
		double width;		// Width in 'em's (width times m_pFontSize)
		const char *header;	// Will be displayed in the header
	};*/

	// column widths at 640
	enum { NAME_WIDTH = 160, SCORE_WIDTH = 60, DEATH_WIDTH = 60, PING_WIDTH = 80, VOICE_WIDTH = 0, FRIENDS_WIDTH = 0 };
	// total = 340

public:
	CScorePanel(IViewport *pParent);
	virtual ~CScorePanel();

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

private:
	IViewport *m_pViewport;
	vgui2::SectionedListPanel *m_pPlayerList = nullptr;
	vgui2::Label *m_pServerNameLabel = nullptr;
	vgui2::Label *m_pMapNameLabel = nullptr;
	vgui2::Label *m_pPlayerCountLabel = nullptr;

	int m_pHeader = 0;

	static bool StaticPlayerSortFunc(vgui2::SectionedListPanel *list, int itemID1, int itemID2);
};

#endif