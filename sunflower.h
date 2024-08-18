#pragma once

#include "ast.h"
#include "function.h"
#include "header.h"
#include "llist.h"
#include "objtable.h"
#include "parser.h"
#include "sfarray.h"
#include "sfclass.h"
#include "sfmem.h"
#include "sfmod.h"
#include "sfstr.h"
#include "tokenizer.h"
#include "tree.h"
#include "trie.h"
#include "nativemethods.h"

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API void sf_dbg_fledump_init ();
  SF_API FILE *sf_dbg_get_filedump ();
  SF_API void sf_dbg_dumpclose ();

#ifdef __cplusplus
}
#endif
