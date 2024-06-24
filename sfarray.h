#pragma once

#include "ast.h"
#include "header.h"
#include "sfmem.h"
#include "sfstr.h"

struct _sf_array_s
{
  size_t len;
  llnode_t **vals;

  struct
  {
    size_t id;
    int has_id;

  } meta;
};

typedef struct _sf_array_s array_t;

#define SF_ARR_CACHE_SIZE 32

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API void sf_array_init (void);

  SF_API array_t *sf_array_new (void);

  SF_API void sf_array_pushVal (array_t *_Arr, llnode_t *_Val);

  SF_API array_t *sf_array_add (array_t *_Arr);

  SF_API void sf_array_free (array_t *_Arr);

#ifdef __cplusplus
}
#endif
