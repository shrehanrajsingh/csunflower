#pragma once

#include "header.h"
#include "llist.h"
#include "sfmem.h"
#include "sfstr.h"
#include "tree.h"

struct _obj_s;
struct _expr_s;
struct _mod_s;

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

  int is_llnode;
  llnode_t *node;

  union
  {
    char *op;
    struct _expr_s *val;
  } v;
};

enum
{
  SF_ARITH_RES_DOUBLE = 0,
  SF_ARITH_RES_OBJ = 1,
};

struct _sfa_arithres_s
{
  int type;

  union
  {
    struct
    {
      double v;
    } dres;

    struct
    {
      struct _obj_s *v;
    } ores;
  } v;
};

typedef struct _sfa_postfix_tree __sfapostfix_tree;
typedef struct _sfa_arithres_s __sfaarith_res;

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * \brief Retrieves the precedence of an arithmetic operator.
   *
   * This function returns the precedence level of the given arithmetic operator.
   * The precedence is used to determine the order of operations in expressions.
   *
   * \param op The arithmetic operator as a string.
   * \return The precedence level of the operator.
   */
  SF_API int sf_arith_get_precedence (char *op);

  /**
   * \brief Converts an infix expression to postfix notation.
   *
   * This function transforms an arithmetic expression from infix notation
   * to postfix notation in place.
   *
   * \param tree A pointer to the root of the infix expression tree.
   * \param size The number of nodes in the infix expression tree.
   */
  SF_API void sf_arith_infix_to_postfix (__sfapostfix_tree **, size_t *);

  SF_API tree_t *sf_arith_pft_to_tree (__sfapostfix_tree *, int);

  SF_API __sfaarith_res sf_arith_eval_tree (struct _mod_s *, tree_t *);

  SF_API tree_t *sf_arith_tree_copyd (tree_t *);

#ifdef __cplusplus
}
#endif