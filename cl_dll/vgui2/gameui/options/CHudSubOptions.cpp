#include <vgui_controls/Label.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/ComboBox.h>
#include <KeyValues.h>
#include "vgui2/VGUI2Paths.h"
#include "CHudSubOptions.h"
#include "CCvarTextEntry.h"
#include "CCvarColor.h"
#include "CCvarCheckButton.h"
#include "hud.h"

CHudSubOptions::CHudSubOptions(vgui2::Panel *parent) : BaseClass(parent, nullptr)
{
	m_pOpacityLabel = new vgui2::Label(this, "OpacityLabel", "#BHL_AdvOptions_HUD_Opacity");
	m_pOpacityValue = new CCvarTextEntry(this, "OpacityValue", "hud_draw", CCvarTextEntry::CvarType::Float);
	m_pOpacitySlider = new vgui2::Slider(this, "OpacitySlider");
	m_pOpacitySlider->SetRange(0, 100);
	m_pOpacitySlider->SetValue(m_pOpacityValue->GetFloat() * 100.f);
	m_pOpacitySlider->AddActionSignalTarget(this);

	m_pColorLabel[0] = new vgui2::Label(this, "ColorLabel", "#BHL_AdvOptions_HUD_Color");
	m_pColorValue[0] = new CCvarColor(this, "ColorValue", "hud_color", "#BHL_AdvOptions_HUD_Color_Title");

	m_pColorLabel[1] = new vgui2::Label(this, "Color1Label", "#BHL_AdvOptions_HUD_Color1");
	m_pColorValue[1] = new CCvarColor(this, "Color1Value", "hud_color1", "#BHL_AdvOptions_HUD_Color1_Title");

	m_pColorLabel[2] = new vgui2::Label(this, "Color2Label", "#BHL_AdvOptions_HUD_Color2");
	m_pColorValue[2] = new CCvarColor(this, "Color2Value", "hud_color2", "#BHL_AdvOptions_HUD_Color2_Title");

	m_pColorLabel[3] = new vgui2::Label(this, "Color3Label", "#BHL_AdvOptions_HUD_Color3");
	m_pColorValue[3] = new CCvarColor(this, "Color3Value", "hud_color3", "#BHL_AdvOptions_HUD_Color3_Title");

	m_pDimCheckbox = new CCvarCheckButton(this, "DimCheckbox", "#BHL_AdvOptions_HUD_Dim", "hud_dim");
	m_pViewmodelCheckbox = new CCvarCheckButton(this, "ViewmodelCheckbox", "#BHL_AdvOptions_HUD_Viewmodel", "r_drawviewmodel", true);
	m_pWeaponSpriteCheckbox = new CCvarCheckButton(this, "WeaponSpriteCheckbox", "#BHL_AdvOptions_HUD_WeapSprite", "hud_weapon");
	m_pSpeedCheckbox = new CCvarCheckButton(this, "SpeedCheckbox", "#BHL_AdvOptions_HUD_Speed", "hud_speedometer");

	LoadControlSettings(UI_RESOURCE_DIR "/options/HudSubOptions.res");
}

void CHudSubOptions::OnResetData()
{
	m_pOpacityValue->ResetData();
	m_pColorValue[0]->ResetData();
	m_pColorValue[1]->ResetData();
	m_pColorValue[2]->ResetData();
	m_pColorValue[3]->ResetData();
	m_pDimCheckbox->ResetData();
	m_pViewmodelCheckbox->ResetData();
	m_pWeaponSpriteCheckbox->ResetData();
	m_pSpeedCheckbox->ResetData();
}

void CHudSubOptions::OnApplyChanges()
{
	m_pOpacityValue->ApplyChanges();
	m_pColorValue[0]->ApplyChanges();
	m_pColorValue[1]->ApplyChanges();
	m_pColorValue[2]->ApplyChanges();
	m_pColorValue[3]->ApplyChanges();
	m_pDimCheckbox->ApplyChanges();
	m_pViewmodelCheckbox->ApplyChanges();
	m_pWeaponSpriteCheckbox->ApplyChanges();
	m_pSpeedCheckbox->ApplyChanges();
}

void CHudSubOptions::OnSliderMoved(KeyValues *kv)
{
	void *pPanel = kv->GetPtr("panel");
	if (pPanel == m_pOpacitySlider)
	{
		float val = kv->GetFloat("position") / 100.f;
		m_pOpacityValue->SetValue(val);
	}
}

void CHudSubOptions::OnCvarTextChanged(KeyValues *kv)
{
	void *pPanel = kv->GetPtr("panel");
	if (pPanel == m_pOpacityValue)
	{
		float val = kv->GetFloat("value") * 100.f;
		m_pOpacitySlider->SetValue(val, false);
	}
}
