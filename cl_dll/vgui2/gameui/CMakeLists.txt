add_sources(
	CMakeLists.txt
	GameUIPanelNames.h
	IGameUIPanel.h
	
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

add_subdirectory( options )
