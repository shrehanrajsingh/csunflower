#pragma once

#include "arithmetic.h"
#include "function.h"
#include "header.h"
#include "llist.h"
#include "module.h"
#include "sfclass.h"
#include "sfmem.h"
#include "tokenizer.h"
#include "tree.h"

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
  STMT_CLASS_DECL = 9,
  STMT_WHILE_BLOCK = 10,
  STMT_IMPORT = 11,
  STMT_RETURN = 12,
  STMT_WITH_BLOCK = 13,
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
  EXPR_TOSTEPTYPE = 10,
  EXPR_INCLAUSE = 11,
  EXPR_ARRAY = 12,
  EXPR_IDX_ACCESS = 13,
  EXPR_ARITHMETIC = 14,
  EXPR_MEM_ACCESS = 15,
  EXPR_THIS = 16,
  EXPR_MAP = 17,
  EXPR_AND = 18,
  EXPR_OR = 19,
  EXPR_NOT = 20,
  EXPR_WHERE = 21,
};

enum // Used for both constant types for expr and obj_t
{
  CONST_INT = 0,
  CONST_FLOAT = 1,
  CONST_STRING = 2,
  CONST_BOOL = 3,
  CONST_NONE = 4, // No struct for this type
};

enum
{
  OBJ_CONST = 0,
  OBJ_FUN = 1,
  OBJ_ARRAY = 2,
  OBJ_CLASS = 3,
  OBJ_CLASSOBJ = 4,
  OBJ_MODULE = 5,
  OBJ_MAP = 6,
};

struct _expr_s;
struct _sf_array_s;
struct _sf_map_s;

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

    } blk_if, blk_elseif, blk_for, blk_while;

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

    /*
        Classes in sunflower:

        class <Identifier_Named>(<Inheritance list>)
              |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
              (Caught as a function call expression)

            <Body...>
            var1 = none
            var2 = none
            <...>

            <Magic methods>
            fun _init()
              this.var1 = "var1"
              this.var2 = [1, 4, 3]

            fun _kill()
              put("Killing\n")

        obj = <Class_name>()
    */
    struct
    {

      struct _expr_s *name;
      struct _stmt_s *body;
      size_t body_count;

    } class_decl;

    struct
    {
      sf_charptr path;
      sf_charptr alias;

    } stmt_import;

    struct
    {
      struct _expr_s *val;

    } stmt_return;

    struct
    {
      struct _expr_s *val, *alias;
      struct _stmt_s *body;
      size_t body_count;

    } blk_with;

  } v;
};

struct _obj_s;

struct __sf_const_s
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
};

struct _expr_s
{
  int type;

  union
  {
    struct __sf_const_s e_const;

    struct
    {

      struct _expr_s *name;
      struct _expr_s *args;
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
      struct _expr_s *e_type;

    } to_step;

    struct
    {

      struct _expr_s *lval;
      struct _expr_s *rval;

    } in_clause;

    struct
    {
      struct _expr_s *vals;
      size_t val_count;

    } e_array;

    struct
    {
      struct _expr_s *name;
      struct _expr_s *val;

    } e_idx_access;

    struct
    {
      tree_t *tree;

    } e_arith;

    /*
      class Test
        a = 10
        b = 20


      x = Test.(a) ----> val
          ^^^^
          parent
    */
    struct
    {
      struct _expr_s *parent;
      struct _expr_s *val;

    } mem_access;

    struct
    {
      struct _expr_s *keys, *vals;
      size_t count;

    } e_map;

    struct
    {
      struct _expr_s *lhs, *rhs;
    } e_and, e_or;

    struct
    {
      struct _expr_s *v;
    } e_not;

    /*
      `where` clause uses strict syntax structures
      Usage:
      <preceding_expr> where <vname_1> = <vval>, <vname_2> = <vval>
      */
    struct
    {
      struct _expr_s *prev_expr;
      sf_charptr *vnames;
      struct _expr_s *vvals;
      size_t vsize;
    } e_where;

  } v;
};

struct _obj_s
{
  int type;

  union
  {
    struct __sf_const_s o_const;

    struct
    {
      fun_t *f;
      int uses_self;
      llnode_t *selfarg;

    } o_fun;

    struct
    {
      struct _sf_array_s *v;

    } o_array;

    struct
    {
      struct _sf_map_s *v;

    } o_map;

    struct
    {
      class_t *val;

    } o_class, o_cobj;

    struct
    {
      sfmodule_t *val;

    } o_mod;

  } v;

  struct
  {
    struct _obj_s **passargs;
    size_t pa_size;

    llnode_t *mem_ref;

  } meta;
};

typedef struct _stmt_s stmt_t;
typedef struct _expr_s expr_t;
typedef struct _obj_s obj_t;

#define OBJ_IS_NUMBER(X)                                                      \
  ((X)->type == OBJ_CONST                                                     \
   && ((X)->v.o_const.type == CONST_INT                                       \
       || (X)->v.o_const.type == CONST_FLOAT))

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API obj_t *sf_ast_objnew (int _Type);

  SF_API stmt_t *sf_ast_stmtgen (tok_t *_Arr, size_t *_SizePtr);
  SF_API expr_t sf_ast_exprgen (tok_t *_Arr, size_t _ArrLen);

  SF_API void sf_ast_exprprint (expr_t _Expr);
  SF_API void sf_ast_stmtprint (stmt_t _Stmt);

  SF_API void sf_ast_freeObj (obj_t **_Obj);

#ifdef __cplusplus
}
#endif
