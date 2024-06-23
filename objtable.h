#pragma once

#include "ast.h"
#include "header.h"
#include "llist.h"
#include "sfmem.h"
#include "sfstr.h"

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API void sf_ot_init (void);

  SF_API llnode_t *sf_ot_addobj (obj_t *_Obj);

  /* unlink node and return object contained so it can be free'd accordingly */
  SF_API void sf_ot_removeobj (llnode_t *_Node);

#ifdef __cplusplus
}
#endif
