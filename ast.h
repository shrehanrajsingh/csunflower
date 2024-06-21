#pragma once

#include "header.h"
#include "sfmem.h"
#include "tokenizer.h"

/**
 * Three divisions of ASTs
 *
 * stmt_t: Statements
 *
 * expr_t: Expressions (statements in places like
 *          function calls, variable declarations etc)
 *
 * obj_t: Evaluated expressions stored as variable
 *          values
 */

enum
{
  STMT_VAR_DECL = 0,
  STMT_FUN_CALL = 1,
  STMT_FUN_DECL = 2,
  STMT_COMMENT = 3,
  STMT_IF_BLOCK = 4,
  STMT_ELSEIF_BLOCK = 5,
  STMT_ELSE_BLOCK = 6,
  STMT_FOR_BLOCK = 7,
  STMT_OPEQ = 8, /* +=, -=, *=, /= */
};

enum
{
  EXPR_CONSTANT = 0,
  EXPR_FUN_CALL = 1,
  EXPR_VAR_DECL = 2, /* a = (b = 10) --> expression */
  EXPR_VAR = 3,
  EXPR_CONDITIONAL_EQEQ = 4,
  EXPR_CONDITIONAL_NEQ = 5,
  EXPR_CONDITIONAL_GT = 6,
  EXPR_CONDITIONAL_LT = 7,
  EXPR_CONDITIONAL_GTEQ = 8,
  EXPR_CONDITIONAL_LTEQ = 9,
  EXPR_TOSTEP = 10,
  EXPR_INCLAUSE = 11,
};

enum // Used for both constant types for expr and obj_t
{
  CONST_INT = 0,
  CONST_FLOAT = 1,
  CONST_STRING = 2,
  CONST_BOOL = 3,
};

enum
{
  OBJ_CONST = 0,
};

struct _expr_s;

struct _stmt_s
{
  int type;

  union
  {
    struct
    {

      struct _expr_s *name;
      struct _expr_s *val;

    } var_decl;

    struct
    {

      struct _expr_s *name;
      struct _expr_s *args;
      size_t arg_count;

    } fun_call;

    struct
    {

      struct _expr_s *name;
      struct _expr_s *args;
      size_t arg_count;

      struct _stmt_s *body;
      size_t body_count;

    } fun_decl;

    struct
    {
      sf_charptr v;

    } stmt_cmt;

    struct
    {

      struct _expr_s *cond;

      struct _stmt_s *body;
      size_t body_count;

    } blk_if, blk_elseif, blk_for;

    struct
    {

      struct _stmt_s *body;
      size_t body_count;

    } blk_else;

    struct
    {

      struct _expr_s *name;
      struct _expr_s *val;
      sf_charptr op;

    } opeq_decl;

  } v;
};

struct _expr_s
{
  int type;

  union
  {
    struct
    {
      int type;

      union
      {
        struct
        {
          sf_charptr v;

        } c_string;

        struct
        {
          sf_int v;

        } c_int;

        struct
        {
          sf_float v;

        } c_float;

        struct
        {
          int v;
        } c_bool;

      } v;

    } e_const;

    struct
    {

      struct _expr_s *name;
      struct _expr_s **args;
      size_t arg_count;

    } fun_call;

    struct
    {

      struct _expr_s *name;
      struct _expr_s *val;

    } var_decl;

    struct
    {
      sf_charptr name;
    } var;

    struct
    {

      struct _expr_s *lval;
      struct _expr_s *rval;

    } expr_conditional;

    struct
    {

      struct _expr_s *lval;
      struct _expr_s *rval;
      struct _expr_s *e_step;

    } to_step;

    struct
    {

      struct _expr_s *lval;
      struct _expr_s *rval;

    } in_clause;

  } v;
};

struct _obj_s
{
  int type;

  union
  {
    struct
    {
      int type;

      union
      {
        struct
        {
          sf_charptr v;

        } c_string;

        struct
        {
          sf_int v;

        } c_int;

        struct
        {
          sf_float v;

        } c_float;

        struct
        {
          int v;
        } c_bool;
      } v;

    } o_const;
  } v;
};

typedef struct _stmt_s stmt_t;
typedef struct _expr_s expr_t;
typedef struct _obj_s obj_t;

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API stmt_t *sf_ast_stmtgen (tok_t *_Arr, size_t *_SizePtr);
  SF_API expr_t sf_ast_exprgen (tok_t *_Arr, size_t _ArrLen);

  SF_API void sf_ast_exprprint (expr_t _Expr);
  SF_API void sf_ast_stmtprint (stmt_t _Stmt);

#ifdef __cplusplus
}
#endif
