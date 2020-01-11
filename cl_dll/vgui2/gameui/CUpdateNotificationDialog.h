#ifndef CUPDATENOTIFICATIONDIALOG_H
#define CUPDATENOTIFICATIONDIALOG_H

#include <vgui/VGUI2.h>
#include <vgui_controls/Frame.h>
#include "IGameUIPanel.h"

namespace vgui2
{
	class Button;
	class Label;
	class CheckButton;
	class Button;
}

class CChangelogDialog;

class CUpdateNotificationDialog : public vgui2::Frame, public IGameUIPanel
{
public:
	DECLARE_CLASS_SIMPLE(CUpdateNotificationDialog, Frame);

public:
	CUpdateNotificationDialog(vgui2::VPANEL parent);
	virtual ~CUpdateNotificationDialog();
	void OnCommand(const char* command) override;
	void Activate() override;

	// IGameUIPanel overrides
	const char *GetName() override;
	virtual void OnGameUIActivated() override;
	virtual void OnGameUIHidden() override;

	// VGUI functions:
	vgui2::VPANEL GetVPanel() override final;
	bool IsVisible() override final;

private:
	CChangelogDialog *m_pChangelogDialog;
	vgui2::Label *m_pMainTextLabel;
	vgui2::Label *m_pGameVersionLabel, *m_pNewVersionLabel;
	vgui2::CheckButton *m_pAutoUpdateBox;
	vgui2::Button *m_pOpenGithubBtn, *m_pOkBtn, *m_pChangelogBtn;
};

#endif