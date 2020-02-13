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

#if !defined(_WIN32) && !defined(MAX_PATH)
#define MAX_PATH 1024
#endif

#ifdef _WIN32
bool GetResultsFilename(const char *extension, char *fileName, char *fullPath);
void ResultsAddLog(const char *line, bool chat);
void ResultsStop(void);
void ResultsFrame(double time);
void ResultsThink(void);
void ResultsInit(void);
#endif