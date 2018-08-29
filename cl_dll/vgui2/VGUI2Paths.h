#ifndef GAME_CLIENT_UI_VGUI2_VGUI2PATHS_H
#define GAME_CLIENT_UI_VGUI2_VGUI2PATHS_H

/**
*	@defgroup VGUI2Paths Paths used by the VGUI2 UI
*
*	@{
*/

/**
*	Base directory for UI files.
*/
#define UI_BASE_DIR ""

/**
*	Resource files.
*/
#define UI_RESOURCE_DIR UI_BASE_DIR "/resource"

/**
*	Scripts.
*/
#define UI_SCRIPTS_DIR UI_BASE_DIR "/scripts"

/**
*	Scheme used by default.
*/
#define UI_CLIENTSCHEME_FILENAME ( UI_RESOURCE_DIR "/GameScheme.res" )

/**
*	Scheme used for CZero tutor elements.
*/
#define UI_TUTORSCHEME_FILENAME ( UI_RESOURCE_DIR "/TutorScheme.res" )

/**
*	File containing HUD animations.
*/
#define UI_HUDANIMS_FILENAME ( UI_SCRIPTS_DIR "/HudAnimations.txt" )

/**
*	File that defines the HUD layout.
*/
#define UI_HUDLAYOUT_FILENAME ( UI_SCRIPTS_DIR "/HudLayout.res" )

/** @} */

#endif //GAME_CLIENT_UI_VGUI2_VGUI2PATHS_H
