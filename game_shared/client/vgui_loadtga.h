//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI1_LOADTGA_H
#define VGUI1_LOADTGA_H
#ifdef _WIN32
#pragma once
#endif


#include "VGUI_BitmapTGA.h"


vgui::BitmapTGA* vgui_LoadTGA(char const *pFilename);
vgui::BitmapTGA* vgui_LoadTGANoInvertAlpha(char const *pFilename);


#endif // VGUI1_LOADTGA_H
