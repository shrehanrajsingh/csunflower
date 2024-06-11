#include "ast.h"

stmt_t _sf_stmt_vdgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_fcgen (tok_t *_arr, size_t _idx, size_t *_jumper);

SF_API stmt_t *
sf_ast_stmtgen (tok_t *arr, size_t *sptr)
{
  stmt_t *res = NULL;
  size_t rc = 0;
  size_t i = 0;

  while (arr[i].type != TOK_EOF)
    {
      tok_t c = arr[i];

      switch (c.type)
        {
        case TOK_OPERATOR:
          {
            if (sf_str_eq (c.v.t_op.v, "="))
              {
                stmt_t t = _sf_stmt_vdgen (arr, i, &i);

                res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t;
              }

            else if (sf_str_eq (c.v.t_op.v, "("))
              {
                stmt_t t = _sf_stmt_fcgen (arr, i, &i);

                res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t;
              }
          }
          break;

        default:
          break;
        }

    l_end:
      i++;
    }

end:
  if (sptr != NULL)
    *sptr = rc;

  return res;
}

SF_API expr_t
sf_ast_exprgen (tok_t *arr, size_t len)
{
  expr_t res;
  size_t i = 0;

  while (i < len)
    {
      tok_t c = arr[i];

      switch (c.type)
        {
        case TOK_FLOAT:
          {
            res.type = EXPR_CONSTANT;
            res.v.e_const.type = CONST_FLOAT;
            res.v.e_const.v.c_float.v = c.v.t_float.v;
          }
          break;
        case TOK_INT:
          {
            // here;
            res.type = EXPR_CONSTANT;
            res.v.e_const.type = CONST_INT;
            res.v.e_const.v.c_int.v = c.v.t_int.v;
          }
          break;
        case TOK_STRING:
          {
            res.type = EXPR_CONSTANT;
            res.v.e_const.type = CONST_STRING;
            res.v.e_const.v.c_string.v
                = sf_str_new_fromStr (SFCPTR_TOSTR (c.v.t_str.v));
          }
          break;
        case TOK_IDENTIFIER:
          {
            if (c.v.t_ident.is_reserved)
              {
                ;
              }
            else
              {
                res.type = EXPR_VAR;
                res.v.var.name
                    = sf_str_new_fromStr (SFCPTR_TOSTR (c.v.t_ident.v));
              }
          }
          break;

        default:
          break;
        }

    l_end:
      i++;
    }

end:
  return res;
}

stmt_t
_sf_stmt_vdgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;
  res.type = STMT_VAR_DECL;

  size_t lsi = 0; // lval_starting_index
  size_t rsi = 0; // rval_ending_index
  int gb = 0;

  for (int i = idx - 1; i >= 0; i--)
    {
      tok_t c = arr[i];

      if ((c.type == TOK_NEWLINE || c.type == TOK_SPACE) && !gb)
        {
          lsi = (size_t)i;
          break;
        }

      if (c.type == TOK_OPERATOR)
        {
          if (sf_str_inStr ("({[", c.v.t_op.v))
            gb++;
          else if (sf_str_inStr (")}]", c.v.t_op.v))
            gb--;
        }
    }

  assert (gb == 0);

  size_t i;
  for (i = idx + 1; arr[i].type != TOK_EOF; i++)
    {
      tok_t c = arr[i];

      if ((c.type == TOK_NEWLINE || c.type == TOK_SPACE) && !gb)
        {
          rsi = (size_t)i;
          break;
        }

      if (c.type == TOK_OPERATOR)
        {
          if (sf_str_inStr ("({[", c.v.t_op.v))
            gb++;
          else if (sf_str_inStr (")}]", c.v.t_op.v))
            gb--;
        }
    }

  if (!rsi)
    rsi = i;

  res.v.var_decl.name = sfmalloc (sizeof (expr_t));
  *res.v.var_decl.name = sf_ast_exprgen (arr + lsi, idx - lsi);

  res.v.var_decl.val = sfmalloc (sizeof (expr_t));
  // printf ("%d %d\n", rsi, idx);
  *res.v.var_decl.val = sf_ast_exprgen (arr + idx + 1, rsi - idx - 1);

  // here;

  if (jptr != NULL)
    {
      *jptr = rsi - 1;
    }

  return res;
}

stmt_t
_sf_stmt_fcgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;
  res.type = STMT_FUN_CALL;
  res.v.fun_call.arg_count = 0;
  res.v.fun_call.args = NULL;
  res.v.fun_call.name = NULL;

  size_t lsi = 0; // lval_starting_index
  size_t rsi = 0; // rval_ending_index
  int gb = 0;

  for (int i = idx - 1; i >= 0; i--)
    {
      tok_t c = arr[i];

      if ((c.type == TOK_NEWLINE || c.type == TOK_SPACE) && !gb)
        {
          lsi = (size_t)i;
          break;
        }

      if (c.type == TOK_OPERATOR)
        {
          if (sf_str_inStr ("({[", c.v.t_op.v))
            gb++;
          else if (sf_str_inStr (")}]", c.v.t_op.v))
            gb--;
        }
    }

  assert (gb == 0);

  size_t i;
  for (i = idx + 1; arr[i].type != TOK_EOF; i++)
    {
      tok_t c = arr[i];

      if (c.type == TOK_OPERATOR)
        {
          if (sf_str_eq_rCp (c.v.t_op.v, ")") && !gb)
            {
              rsi = i;
              break;
            }
          if (sf_str_inStr ("({[", c.v.t_op.v))
            gb++;
          else if (sf_str_inStr (")}]", c.v.t_op.v))
            gb--;
        }
    }

  assert (rsi != 0); // Missing ')'
  assert (gb == 0);

  res.v.fun_call.name = sfmalloc (sizeof (expr_t));
  *res.v.fun_call.name = sf_ast_exprgen (arr + lsi, idx - lsi);

  tok_t *arg_arr = sfmalloc ((rsi - idx) * sizeof (*arg_arr));
  size_t aai = 0;

  for (size_t j = idx + 1; j < rsi; j++)
    {
      tok_t c = arr[j];

      if (c.type == TOK_OPERATOR)
        {
          if (sf_str_eq_rCp (c.v.t_op.v, ",") && !gb)
            {
              res.v.fun_call.args = sfrealloc (res.v.fun_call.args,
                                               (res.v.fun_call.arg_count + 1)
                                                   * sizeof (expr_t));

              res.v.fun_call.args[res.v.fun_call.arg_count++]
                  = sf_ast_exprgen (arg_arr, aai);

              aai = 0;
              continue;
            }
          if (sf_str_inStr ("({[", c.v.t_op.v))
            gb++;
          else if (sf_str_inStr (")}]", c.v.t_op.v))
            gb--;
        }

      arg_arr[aai++] = c;
    }

  if (aai)
    {
      res.v.fun_call.args
          = sfrealloc (res.v.fun_call.args,
                       (res.v.fun_call.arg_count + 1) * sizeof (expr_t));

      res.v.fun_call.args[res.v.fun_call.arg_count++]
          = sf_ast_exprgen (arg_arr, aai);
      aai = 0;
    }

  if (jptr != NULL)
    *jptr = rsi;

  return res;
}

SF_API void
sf_ast_exprprint (expr_t e)
{
  switch (e.type)
    {
    case EXPR_CONSTANT:
      {
        switch (e.v.e_const.type)
          {
          case CONST_BOOL:
            printf ("const_bool %d\n", e.v.e_const.v.c_bool.v);
            break;
          case CONST_FLOAT:
            printf ("const_float %d\n", e.v.e_const.v.c_float.v);
            break;
          case CONST_INT:
            printf ("const_int %d\n", e.v.e_const.v.c_int.v);
            break;
          case CONST_STRING:
            printf ("const_string %s\n", e.v.e_const.v.c_string.v);
            break;

          default:
            printf ("unknown constant %d\n", e.v.e_const.type);
            break;
          }
      }
      break;
    case EXPR_VAR:
      {
        printf ("var %s\n", e.v.var.name);
      }
      break;
    case EXPR_VAR_DECL:
      {
        printf ("var_decl\n");

        printf ("name\n");
        sf_ast_exprprint (*e.v.var_decl.name);

        printf ("value\n");
        sf_ast_exprprint (*e.v.var_decl.val);
      }
      break;
    case EXPR_FUN_CALL:
      {
        printf ("fun_call\n");
        sf_ast_exprprint (*e.v.fun_call.name);

        printf ("args\n");
        for (size_t i = 0; i < e.v.fun_call.arg_count; i++)
          {
            sf_ast_exprprint (*e.v.fun_call.args[i]);
          }
      }
      break;

    default:
      printf ("unknown expression %d\n", e.type);
      break;
    }
}

SF_API void
sf_ast_stmtprint (stmt_t s)
{
  switch (s.type)
    {
    case STMT_VAR_DECL:
      {
        printf ("var_decl\n");

        printf ("name\n");
        sf_ast_exprprint (*s.v.var_decl.name);

        printf ("value\n");
        sf_ast_exprprint (*s.v.var_decl.val);
      }
      break;
    case STMT_FUN_CALL:
      {
        printf ("fun_call\n");
        sf_ast_exprprint (*s.v.fun_call.name);

        printf ("args\n");
        for (size_t i = 0; i < s.v.fun_call.arg_count; i++)
          {
            sf_ast_exprprint (s.v.fun_call.args[i]);
          }
      }
      break;
    case STMT_FUN_DECL:
      {
        printf ("fun_decl\n");
        sf_ast_exprprint (*s.v.fun_decl.name);

        printf ("args\n");
        for (size_t i = 0; i < s.v.fun_decl.arg_count; i++)
          {
            sf_ast_exprprint (s.v.fun_decl.args[i]);
          }

        printf ("body\n");
        for (size_t i = 0; i < s.v.fun_decl.body_count; i++)
          {
            sf_ast_stmtprint (s.v.fun_decl.body[i]);
          }
      }
      break;

    default:
      printf ("unknown statement %d\n", s.type);
      break;
    }
}