#include "CCrosshairSubOptions.h"
#include "../VGUI2Paths.h"
#include "CCvarCheckButton.h"
#include "CCvarTextEntry.h"
#include "CCvarColor.h"
#include "hud.h"

CCrosshairSubOptions::CCrosshairSubOptions(vgui2::Panel *parent) :
	BaseClass(parent, nullptr)
{
	m_pEnableCvar = new CCvarCheckButton(this, "EnableCvar", "#BHL_AdvOptions_Cross_Enable", "cl_crosshair_custom");
	
	m_pColorLabel = new vgui2::Label(this, "ColorLabel", "#BHL_AdvOptions_Cross_Color");
	m_pColorCvar = new CCvarColor(this, "ColorCvar", nullptr, "#BHL_AdvOptions_Cross_Color_Title");
	
	m_pGapLabel = new vgui2::Label(this, "GapLabel", "#BHL_AdvOptions_Cross_Gap");
	m_pGapCvar = new CCvarTextEntry(this, "GapCvar", "cl_crosshair_gap", CCvarTextEntry::CvarType::Int);
	
	m_pSizeLabel = new vgui2::Label(this, "SizeLabel", "#BHL_AdvOptions_Cross_Size");
	m_pSizeCvar = new CCvarTextEntry(this, "SizeCvar", "cl_crosshair_size", CCvarTextEntry::CvarType::Int);
	
	m_pThicknessLabel = new vgui2::Label(this, "ThicknessLabel", "#BHL_AdvOptions_Cross_Thickness");
	m_pThicknessCvar = new CCvarTextEntry(this, "ThicknessCvar", "cl_crosshair_thickness", CCvarTextEntry::CvarType::Int);
	
	m_pOutlineThicknessLabel = new vgui2::Label(this, "OutlineThickness", "#BHL_AdvOptions_Cross_OutThickness");
	m_pOutlineThicknessCvar = new CCvarTextEntry(this, "OutlineThicknessCvar", "cl_crosshair_outline_thickness", CCvarTextEntry::CvarType::Int);
	
	m_pDotCvar = new CCvarCheckButton(this, "DotCvar", "#BHL_AdvOptions_Cross_Dot", "cl_crosshair_dot");
	
	m_pTCvar = new CCvarCheckButton(this, "TCvar", "#BHL_AdvOptions_Cross_T", "cl_crosshair_t");

	m_pColors[0] = gEngfuncs.pfnGetCvarPointer("cl_crosshair_red");
	m_pColors[1] = gEngfuncs.pfnGetCvarPointer("cl_crosshair_green");
	m_pColors[2] = gEngfuncs.pfnGetCvarPointer("cl_crosshair_blue");

	LoadControlSettings(UI_RESOURCE_DIR "/options/CrosshairSubOptions.res");
}

void CCrosshairSubOptions::OnResetData()
{
	m_pEnableCvar->ResetData();
	m_pGapCvar->ResetData();
	m_pSizeCvar->ResetData();
	m_pThicknessCvar->ResetData();
	m_pOutlineThicknessCvar->ResetData();
	m_pDotCvar->ResetData();
	m_pTCvar->ResetData();

	SDK_Color color;
	color.SetColor(m_pColors[0]->value, m_pColors[1]->value, m_pColors[2]->value, 255);
	m_pColorCvar->SetInitialColor(color);
}

void CCrosshairSubOptions::OnApplyChanges()
{
	m_pEnableCvar->ApplyChanges();
	m_pGapCvar->ApplyChanges();
	m_pSizeCvar->ApplyChanges();
	m_pThicknessCvar->ApplyChanges();
	m_pOutlineThicknessCvar->ApplyChanges();
	m_pDotCvar->ApplyChanges();
	m_pTCvar->ApplyChanges();

	char buf[128];
	SDK_Color color = m_pColorCvar->GetSelectedColor();
	m_pColorCvar->SetInitialColor(color);
	
	for (int i = 0; i < 3; i++)
	{
		snprintf(buf, sizeof(buf), "%s \"%d\"", m_pColors[i]->name, color[i]);
		gEngfuncs.pfnClientCmd(buf);
	}
}
