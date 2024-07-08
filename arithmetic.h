#pragma once

#include "header.h"
#include "sfmem.h"
#include "sfstr.h"
#include "tree.h"

struct _expr_s;

struct _sfa_postfix_tree
{
  int is_op;

  union
  {
    char *op;
    struct _expr_s *val;
  } v;
};

struct _sfa_treetok_s
{
  int is_op;

  union
  {
    char *op;
    struct _expr_s *val;
  } v;
};

typedef struct _sfa_postfix_tree __sfapostfix_tree;

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API int sf_arith_get_precedence (char *);

  SF_API void sf_arith_infix_to_postfix (__sfapostfix_tree **, size_t *);

  SF_API tree_t *sf_arith_pft_to_tree (__sfapostfix_tree *, int);

  SF_API double sf_arith_eval_tree (tree_t *);

#ifdef __cplusplus
}
#endif