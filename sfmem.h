#pragma once

#include "header.h"

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API void *sfmalloc (size_t _Size);
  SF_API void *sfrealloc (void *_Old_ptr, size_t _New_size);
  SF_API void sffree (void *_Ptr);

  SF_API char *sfstrdup (char *_Str);

#ifdef __cplusplus
}
#endif
