#pragma once

#include "ast.h"
#include "header.h"
#include "sfmem.h"
#include "sfstr.h"
#include "trie.h"

/* maps in sunflower are tries for the moment */
struct _sf_map_s
{
  trie_t *t;

  struct
  {
    size_t id;
    int has_id;

  } meta;
};

typedef struct _sf_map_s map_t;

#define SF_MAP_CACHE_SIZE 32

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API void sf_map_init (void);

  SF_API map_t *sf_map_new (void);

  SF_API void sf_map_addKeyVal (map_t *_Map, sf_charptr _Name, llnode_t *_Val);

  SF_API map_t *sf_map_add (map_t *_Map);

  SF_API void sf_map_free (map_t *_Map);

#ifdef __cplusplus
}
#endif
