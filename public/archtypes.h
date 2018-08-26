//
// Word size dependent definitions
// DAL 1/03
//
#ifndef ARCHTYPES_H
#define ARCHTYPES_H

#include "steam/steamtypes.h"

#ifndef _WIN32
//TODO: resolve the multiple definitions of this constant. - Solokiller
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <stddef.h>
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
typedef long unsigned int ulong;
#endif

#endif // ARCHTYPES_H
