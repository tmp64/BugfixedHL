add_sources(
    voice_common.h
    bitvec.h
	CGameVersion.h
	CGameVersion.cpp
	ClientSupportsFlags.h
)

if( USE_UPDATER )
	add_sources(
		CGameUpdater.h
		CGameUpdater.cpp
	)
endif()
