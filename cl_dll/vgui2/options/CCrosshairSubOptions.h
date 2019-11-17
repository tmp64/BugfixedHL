#ifndef CCROSSHAIRSUBOPTIONS
#define CCROSSHAIRSUBOPTIONS
#include <vgui_controls/PropertyPage.h>

namespace vgui2
{
class Label;
class ComboBox;
}

class CCvarCheckButton;
class CCvarTextEntry;
class CCvarColor;

typedef struct cvar_s cvar_t;

class CCrosshairSubOptions : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CCrosshairSubOptions, vgui2::PropertyPage);
public:
	CCrosshairSubOptions(vgui2::Panel *parent);

	virtual void OnResetData();
	virtual void OnApplyChanges();

private:
	CCvarCheckButton *m_pEnableCvar = nullptr;
	vgui2::Label *m_pColorLabel = nullptr;
	CCvarColor *m_pColorCvar = nullptr;
	vgui2::Label *m_pGapLabel = nullptr;
	CCvarTextEntry *m_pGapCvar = nullptr;
	vgui2::Label *m_pSizeLabel = nullptr;
	CCvarTextEntry *m_pSizeCvar = nullptr;
	vgui2::Label *m_pThicknessLabel = nullptr;
	CCvarTextEntry *m_pThicknessCvar = nullptr;
	vgui2::Label *m_pOutlineThicknessLabel = nullptr;
	CCvarTextEntry *m_pOutlineThicknessCvar = nullptr;
	CCvarCheckButton *m_pDotCvar = nullptr;
	CCvarCheckButton *m_pTCvar = nullptr;

	cvar_t *m_pColors[3];
};

#endif
