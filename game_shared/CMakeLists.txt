add_sources(
    voice_common.h
    bitvec.h
	CGameVersion.h
	CGameVersion.cpp
)

if( USE_UPDATER )
	add_sources(
		CGameUpdater.h
		CGameUpdater.cpp
	)
endif()
