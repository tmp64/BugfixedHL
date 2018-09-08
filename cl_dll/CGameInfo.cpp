#include "CGameInfo.h"
#include "hud.h"
#include "CHudScores.h"
#include "CScorePanel.h"

CGameInfo gGameInfo;

void CGameInfo::UpdateScoreboard()
{
	if (gHUD.m_Scores->m_pScorePanel) gHUD.m_Scores->m_pScorePanel->FullUpdate();
}
