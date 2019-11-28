#ifndef CUPDATERDEBUGDIALOG_H
#define CUPDATERDEBUGDIALOG_H

#include <string>
#include <vgui/VGUI2.h>
#include <vgui_controls/Frame.h>
#include "IGameUIPanel.h"

namespace vgui2
{
	class RichText;
	class Button;
	class Label;
}

class CUpdaterDebugDialog : public vgui2::Frame, public IGameUIPanel
{
public:
	DECLARE_CLASS_SIMPLE(CUpdaterDebugDialog, Frame);

public:
	CUpdaterDebugDialog(vgui2::VPANEL parent);
	virtual ~CUpdaterDebugDialog();
	void OnCommand(const char* command) override;
	void Activate() override;

	// IGameUIPanel overrides
	const char *GetName() override;
	void Reset() override;
	void ShowPanel(bool state) override;
	virtual void OnGameUIActivated() override;
	virtual void OnGameUIDeactivated() override;

	// VGUI functions:
	vgui2::VPANEL GetVPanel() override final;
	bool IsVisible() override final;

private:
	bool m_bIsOpen = false;
	bool m_bCallbacksAdded = false;

	vgui2::Button *m_pCheckBtn, *m_pDownloadBtn, *m_pInstallBtn;
	vgui2::RichText *m_pChangeLogBox;
	vgui2::Label *m_pErrorLabel, *m_pGameVerLabel, *m_pLatestVerLabel;

	void AddCallbacks();
	void ErrorOccuredCallback(std::string error);
	void CheckFinishedCallback(bool isUpdateFound);
	void DownloadFinishedCallback();
};

#endif