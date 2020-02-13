#ifndef CGAMEUITESTPANEL_H
#define CGAMEUITESTPANEL_H

#include <vgui/VGUI2.h>
#include <vgui_controls/Frame.h>
#include "IGameUIPanel.h"

namespace vgui2
{
	class RichText;
}

class CGameUITestPanel : public vgui2::Frame, public IGameUIPanel
{
public:
	DECLARE_CLASS_SIMPLE(CGameUITestPanel, Frame);

public:
	CGameUITestPanel(vgui2::VPANEL parent);
	virtual ~CGameUITestPanel();
	void Activate() override;
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	void Reset();

	// IGameUIPanel overrides
	const char *GetName() override;
	virtual void OnGameUIActivated() override;
	virtual void OnGameUIHidden() override;

	// VGUI functions:
	vgui2::VPANEL GetVPanel() override final;

private:
	vgui2::RichText *m_pRichText = nullptr;
};

#endif