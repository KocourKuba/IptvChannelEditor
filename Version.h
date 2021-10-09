// * Copyright(c) 2006-2020 MediaTwins s.r.o.
// *********************************************************

#pragma once

#define STR2(x) #x
#define STR(x) STR2(x)

#include "VerGIT.h"

#define MAJOR	2
#define MINOR	1
#define SPECIAL 0

#define FILEVER        MAJOR,MINOR,BUILD,SPECIAL
#define PRODUCTVER     FILEVER

#define STRPRODUCTVER	STR(MAJOR) "." STR(MINOR) "." STR(BUILD)
#define STRPRIVATEBUILD STR(SPECIAL)
#define STRFILEVERSION	STRPRODUCTVER ", " RELEASEDATE
