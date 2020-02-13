#ifndef IGAMEUIPANEL_H
#define IGAMEUIPANEL_H

#include <vgui/VGUI2.h>

/**
*	A panel that is part of the client's GameUI (pause menu).
*/
class IGameUIPanel
{
public:
	virtual	~IGameUIPanel() {}

	/**
	*	@return identifer name
	*/
	virtual const char *GetName() = 0;

	/**
	*	Called when GameUI is shown
	*/
	virtual void OnGameUIActivated() = 0;

	/**
	*	Called when GameUI is hidden
	*/
	virtual void OnGameUIHidden() = 0;

	// VGUI functions:

	/**
	*	@return VGUI panel handle
	*/
	virtual vgui2::VPANEL GetVPanel() = 0;
};

#endif //IGAMEUIPANEL_H
