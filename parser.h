#pragma once

#include "arithmetic.h"
#include "ast.h"
#include "header.h"
#include "llist.h"
#include "objtable.h"
#include "sfarray.h"
#include "sfmem.h"
#include "sfmod.h"
#include "sfstr.h"
#include "tokenizer.h"
#include "tree.h"
#include "trie.h"

struct _parser_ret_s
{
  int i;
};

typedef struct _parser_ret_s parser_rt;

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API void sf_parser_init (void);
  SF_API parser_rt sf_parser_exec (mod_t *_Mod);

  SF_API char *sf_parser_objRepr (mod_t *_Mod, obj_t *_Obj);
  SF_API llnode_t *eval_expr (mod_t *, expr_t *);

#ifdef __cplusplus
}
#endif
