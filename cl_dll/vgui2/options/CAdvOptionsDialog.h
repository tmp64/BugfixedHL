#ifndef CADVOPTIONSDIALOG_H
#define CADVOPTIONSDIALOG_H

#include <vgui/VGUI2.h>
#include <vgui_controls/PropertyDialog.h>
#include "../IGameUIPanel.h"

class CAdvOptionsDialog : public vgui2::PropertyDialog, public IGameUIPanel
{
public:
	DECLARE_CLASS_SIMPLE(CAdvOptionsDialog, vgui2::PropertyDialog);

public:
	CAdvOptionsDialog(vgui2::VPANEL parent);
	virtual ~CAdvOptionsDialog();

	void Activate() override;
	void OnClose() override;
	bool IsOpen();
	virtual void OnCommand(const char *command);

	// IGameUIPanel overrides
	const char *GetName() override;
	void Reset() override;
	void ShowPanel(bool state) override;
	virtual void OnGameUIActivated() override;
	virtual void OnGameUIDeactivated() override;

	// VGUI functions:
	vgui2::VPANEL GetVPanel() override final;
	bool IsVisible() override final;

	static void RegisterConsoleCommands();

private:
	bool m_bIsOpen = false;
};

#endif
