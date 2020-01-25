add_sources(
	CMakeLists.txt
	GameUIPanelNames.h
	IGameUIPanel.h
	CGameUIViewport.h
	CGameUIViewport.cpp
	
	CGameUITestPanel.h
	CGameUITestPanel.cpp
)

if( USE_UPDATER )
	add_sources(
		CUpdaterDebugDialog.h
		CUpdaterDebugDialog.cpp
		CUpdateNotificationDialog.h
		CUpdateNotificationDialog.cpp
		CChangeLogDialog.h
		CChangeLogDialog.cpp
	)
endif()

add_subdirectory( options )
