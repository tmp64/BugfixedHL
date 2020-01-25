#ifndef CCHANGELOGDIALOG_H
#define CCHANGELOGDIALOG_H
#include <vgui_controls/Frame.h>

class CChangeLogDialog : vgui2::Frame
{
public:
	DECLARE_CLASS_SIMPLE(CChangeLogDialog, Frame);

public:
	CChangeLogDialog(vgui2::Panel *parent);
	virtual void PerformLayout();
	virtual void Activate();

private:
	vgui2::RichText *m_pChangelogText = nullptr;
};

#endif
