add_sources(
	basetypes.h
	commonmacros.h
	coordsize.h
	datamap.h
	FileSystem_Helpers.cpp
	FileSystem_Helpers.h
	ICVar.h
	mathlib.h
	OfflineMode.h
	SDK_Color.h
	Vector.h
	Vector2D.h
	winlite.h
	worldsize.h
)

add_subdirectory( appframework )
add_subdirectory( bitmap )
add_subdirectory( html )
add_subdirectory( mathlib )
add_subdirectory( tier0 )
add_subdirectory( tier1 )
add_subdirectory( vgui )
add_subdirectory( vgui_controls )
add_subdirectory( vstdlib )
