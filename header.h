#pragma once

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifndef SF_API
#define SF_API
#endif

typedef char *sf_charptr;
typedef int sf_int;
typedef double sf_float;

#define e_printf printf
#define here printf ("%s (%s): %d\n", __FILE__, __FUNCTION__, __LINE__);
