/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Results.h
//
// Functions for storing game results files.
//

#ifdef _WIN32
#include <windows.h>
#elif !defined(MAX_PATH)
#define MAX_PATH 1024
#endif

#ifdef _WIN32
bool GetResultsFilename(const char *extension, char fileName[MAX_PATH], char fullPath[MAX_PATH]);
void ResultsAddLog(const char *line, bool chat);
void ResultsStop(void);
void ResultsFrame(double time);
void ResultsThink(void);
void ResultsInit(void);
#endif