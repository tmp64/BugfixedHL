#ifndef _DLLEXPORT

#ifdef _WIN32
#define _DLLEXPORT __declspec(dllexport)
#else
#define _DLLEXPORT __attribute__ ((visibility("default")))
#endif

#define DLLEXPORT _DLLEXPORT

#endif