#pragma once

#include "header.h"
#include "sfmem.h"

#ifndef SFCPTR_TOSTR
#define SFCPTR_TOSTR(X) ((char *)X)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API sf_charptr sf_str_new_empty (void);
  SF_API sf_charptr sf_str_new_fromStr (const char *_Source);

  SF_API void sf_str_push (sf_charptr *_Target, const char *_Source);
  SF_API void sf_str_pushchr (sf_charptr *_Target, const char _Source);

  SF_API int sf_str_eq (sf_charptr _L1, sf_charptr _L2);
  SF_API int sf_str_eq_rCp (sf_charptr _L1, const char *_L2);
  SF_API int sf_str_inStr (const char *_L1,
                           sf_charptr _L2); // is "str" _L1 in "str" _L2?

  SF_API sf_charptr sf_str_copy (sf_charptr _Source);

  SF_API void sf_str_unescape (sf_charptr *_Target);

  SF_API int sf_str_startswith (sf_charptr _Str, const char *_SubStr);
  SF_API int sf_str_endswith (sf_charptr _Str, const char *_SubStr);

#ifdef __cplusplus
}
#endif
