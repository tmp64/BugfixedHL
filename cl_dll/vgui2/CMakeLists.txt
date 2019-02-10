add_sources(
	CBackGroundPanel.h
	CBackGroundPanel.cpp
	CBaseViewport.h
	CBaseViewport.cpp
	CHudViewport.h
	CHudViewport.cpp
	CClientVGUI.h
	CClientVGUI.cpp
	IViewport.h
	IViewportPanel.h
	VGUI2Paths.h
	ViewportPanelNames.h
	CAvatarImage.h
	CAvatarImage.cpp
	CHudScoreBoard.h
	CHudScoreBoard.cpp
	CHudTextVgui.h
	CHudTextVgui.cpp
	CPlayerListPanel.h
	CPlayerListPanel.cpp
	CVoiceStatus2.h
	CVoiceStatus2.cpp
	KeyValuesCompat.h
	KeyValuesCompat.cpp
	CPngImage.h
	CPngImage.cpp
	IGameUIPanel.h
	GameUIPanelNames.h
	IEngineVgui.h
	
	# HUD panels
	CScorePanel.h
	CScorePanel.cpp
	CClientMOTD.h
	CClientMOTD.cpp
	
	# GameUI panels
	CGameUITestPanel.h
	CGameUITestPanel.cpp
)

if( USE_UPDATER )
	add_sources(
		CUpdaterDebugDialog.h
		CUpdaterDebugDialog.cpp
		CUpdateNotificationDialog.h
		CUpdateNotificationDialog.cpp
	)
endif()
