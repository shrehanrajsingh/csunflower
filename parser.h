#pragma once

#include "ast.h"
#include "header.h"
#include "llist.h"
#include "objtable.h"
#include "sfmem.h"
#include "sfmod.h"
#include "sfstr.h"
#include "tokenizer.h"
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

  SF_API parser_rt sf_parser_exec (mod_t *_Mod);

#ifdef __cplusplus
}
#endif
