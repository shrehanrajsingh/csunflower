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

// FILE *SF_DEBUG_DUMP;

#define SF_DISABLE_THIS

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined (SF_NODEBUG)
  SF_API FILE *sf_dbg_get_filedump ();
#endif

#ifdef __cplusplus
}
#endif

#if !defined(SF_NODEBUG)
#define e_printf(...) fprintf (sf_dbg_get_filedump (), __VA_ARGS__)
#else
#define e_printf(...)
#endif
#define here printf ("%s (%s): %d\n", __FILE__, __FUNCTION__, __LINE__);

// This is how they are to be written in code
#define SF_BOOL_TRUE_REPR "true"
#define SF_BOOL_FALSE_REPR "false"
#define SF_DTYPE_NONE_REPR "none"

#define SF_OP_OVRLD_PLUS ("operator+")

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
