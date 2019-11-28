#ifndef CGAMEUIVIEWPORT_H
#define CGAMEUIVIEWPORT_H
#include <vector>
#include "IGameUIPanel.h"
#include <vgui_controls/EditablePanel.h>

class CGameUIViewport : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CGameUIViewport, vgui2::EditablePanel);
public:
	CGameUIViewport();
	void Initialize(CreateInterfaceFn *pFactories, int iNumFactories);
	void Start();
	void ActivateClientUI();
	void HideClientUI();
	void Shutdown();

	IGameUIPanel *CreatePanelByName(const char *pszName);
	template <typename T>
	inline T *CreatePanel(const char *pszName) { return dynamic_cast<T *>(CreatePanelByName(pszName)); }

	IGameUIPanel *FindPanelByName(const char *pszName);
	template <typename T>
	inline T *FindPanel(const char *pszName) { return dynamic_cast<T *>(FindPanelByName(pszName)); }

	void AddPanel(IGameUIPanel *pPanel);
	bool DeletePanel(const char *pszName);
	bool DeletePanel(IGameUIPanel *pPanel);

private:
	std::vector<IGameUIPanel *> m_Panels;

	void CreateDefaultPanels();
};

extern CGameUIViewport *g_pGameUIViewport;

#endif
