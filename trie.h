#pragma once

#include "header.h"
#include "sfmem.h"

#define TRIE_KEYMAXSIZE 64 // [a-zA-Z0-9]\b(_)

struct _trienode_s
{
  char key[TRIE_KEYMAXSIZE];
  size_t kl;

  struct _trienode_s **nodes;

  int isval;
  void *val;

  struct // for val
  {
    int i;
  } meta;
};

typedef struct _trienode_s trie_t;

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API trie_t *sf_trie_new (void);

  SF_API void sf_trie_makeVal (trie_t *_Trie, void *_Val);

  SF_API void sf_trie_add (trie_t *_Trie, char *_Key, void *_Val);

  SF_API char **sf_trie_getKeys (trie_t *_Trie);

  SF_API void *sf_trie_getVal (trie_t *_Trie, char *_Key);

  SF_API void sf_trie_free (trie_t *_Trie);

#ifdef __cplusplus
}
#endif
