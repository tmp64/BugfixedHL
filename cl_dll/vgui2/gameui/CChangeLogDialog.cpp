#include <vgui/ILocalize.h>
#include <vgui_controls/RichText.h>
#include <CGameUpdater.h>
#include "CChangelogDialog.h"
#include "vgui2/VGUI2Paths.h"

CChangeLogDialog::CChangeLogDialog(vgui2::Panel *parent) : BaseClass(parent, "ChangelogDialog")
{
	MakePopup();
	SetProportional(false);
	SetVisible(false);
	SetTitle("#BHL_ChangeLog_Title", false);
	SetSize(480, 320);

	m_pChangelogText = new vgui2::RichText(this, "ChangelogText");
	LoadControlSettings(UI_RESOURCE_DIR "/ChangelogDialog.res");

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(UI_GAMEUISCHEME_FILENAME, "SourceScheme"));
}

void CChangeLogDialog::PerformLayout()
{
	BaseClass::PerformLayout();
	constexpr int PADDING = 8;
	constexpr int HEAD = 24;
	m_pChangelogText->SetBounds(PADDING, PADDING + HEAD,
		GetWide() - PADDING * 2, GetTall() - PADDING * 2 - HEAD);
}

void CChangeLogDialog::Activate()
{
	// SetText(const char *) uses a tiny buffer (1 KB) to convert UTF-8 to UTF-16
	wchar_t *buf = new wchar_t[16 * 1024];	// 16 KB should probably be enough
	vgui2::localize()->ConvertANSIToUnicode(gGameUpdater->GetChangeLog().c_str(), buf, 16 * 1024);
	m_pChangelogText->SetText(buf);
	delete[] buf;
	BaseClass::Activate();
}
