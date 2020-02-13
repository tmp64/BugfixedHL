#include "hud.h"
#include "CHudBase.h"

CHudBase::CHudBase()
{
	gHUD.m_HudList.push_back(this);
	m_ThisIterator = --gHUD.m_HudList.end();
	m_bIsIteratorValid = true;
}

CHudBase::~CHudBase()
{
	if (m_bIsIteratorValid)
		EraseFromHudList();
}

void CHudBase::EraseFromHudList()
{
	assert(m_bIsIteratorValid);
	gHUD.m_HudList.erase(m_ThisIterator);
	m_bIsIteratorValid = false;
}

void CHudBase::Init()
{
	return;
}

void CHudBase::VidInit()
{
	return;
}

void CHudBase::Draw(float flTime)
{
	return;
}

void CHudBase::Think(void)
{
}

void CHudBase::Reset(void)
{
}

void CHudBase::InitHUDData(void)
{
}
