#ifndef CHUDSUBOPTIONS_H
#define CHUDSUBOPTIONS_H
#include <vgui_controls/PropertyPage.h>

namespace vgui2
{
	class Label;
	class Slider;
	class ComboBox;
}

class CCvarTextEntry;
class CCvarColor;
class CCvarCheckButton;

class CHudSubOptions : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHudSubOptions, vgui2::PropertyPage);

public:
	CHudSubOptions(vgui2::Panel *parent);

	virtual void OnResetData();
	virtual void OnApplyChanges();

private:
	vgui2::Label *m_pOpacityLabel = nullptr;
	vgui2::Slider *m_pOpacitySlider = nullptr;
	CCvarTextEntry *m_pOpacityValue = nullptr;

	vgui2::Label *m_pColorLabel[4];
	CCvarColor *m_pColorValue[4];

	CCvarCheckButton *m_pDimCheckbox = nullptr;
	CCvarCheckButton *m_pWeaponSpriteCheckbox = nullptr;
	CCvarCheckButton *m_pViewmodelCheckbox = nullptr;
	CCvarCheckButton *m_pSpeedCheckbox = nullptr;

	MESSAGE_FUNC_PARAMS(OnSliderMoved, "SliderMoved", kv);
	MESSAGE_FUNC_PARAMS(OnCvarTextChanged, "CvarTextChanged", kv);
};

#endif
