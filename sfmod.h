#pragma once

#include "ast.h"
#include "header.h"
#include "sfmem.h"
#include "sfstr.h"
#include "trie.h"

enum
{
  MOD_TYPE_FILE,
  MOD_TYPE_INTERACTIVE,
  MOD_TYPE_FUNC,
  MOD_TYPE_CLASS,
};

struct _mod_s
{
  int type;
  trie_t *vtable;

  stmt_t *body;
  size_t body_len;

  obj_t *retv; // return value of a mod (return value of a function)
  struct _mod_s *parent;
};

typedef struct _mod_s mod_t;

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif
