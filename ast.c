#include "ast.h"
#include "parser.h"
#include "sfarray.h"

stmt_t _sf_stmt_vdgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_opeqgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_fcgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_ifblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_elseblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_elseifblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_forblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_fundeclgen (tok_t *_arr, size_t _idx, size_t *_jumper);

/**
 * Function returns _arr + _C
 * _C is an integer which refers to the index where the block
 * has ended.
 * Returns the index where block has ended for the first time.
 */
tok_t *_sf_stmt_getblkend (tok_t *_arr, size_t _tabspace);

/**
 * Returns the indentation of the token _arr[_idx]
 */
size_t _sf_stmt_gettbsp (tok_t *_arr, size_t _idx);

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
            if (sf_str_eq_rCp (c.v.t_op.v, "="))
              {
                stmt_t t = _sf_stmt_vdgen (arr, i, &i);

                res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t;
              }

            else if (sf_str_endswith (c.v.t_op.v, "="))
              {
                // if (sf_str_eq_rCp (c.v.t_op.v, "+=")
                //     || sf_str_eq_rCp (c.v.t_op.v, "-=")
                //     || sf_str_eq_rCp (c.v.t_op.v, "*=")
                //     || sf_str_eq_rCp (c.v.t_op.v, "/=")
                //     || sf_str_eq_rCp (c.v.t_op.v, "%=")
                //     || sf_str_eq_rCp (c.v.t_op.v, "|=")
                //     || sf_str_eq_rCp (c.v.t_op.v, "&=")
                //     || sf_str_eq_rCp (c.v.t_op.v, "**=")
                //     || sf_str_eq_rCp (c.v.t_op.v, "<<=")
                //     || sf_str_eq_rCp (c.v.t_op.v, ">>="))
                //   {
                stmt_t t = _sf_stmt_opeqgen (arr, i, &i);

                res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t;
                // }
              }

            else if (sf_str_eq_rCp (c.v.t_op.v, "("))
              {
                stmt_t t = _sf_stmt_fcgen (arr, i, &i);

                res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t;
              }
          }
          break;
        case TOK_IDENTIFIER:
          {
            if (c.v.t_ident.is_reserved)
              {
                sf_charptr cv = c.v.t_ident.v;

                if (sf_str_eq_rCp (cv, "if"))
                  {
                    stmt_t t = _sf_stmt_ifblockgen (arr, i, &i);

                    res = sfrealloc (res, (rc + 1) * sizeof (*res));
                    res[rc++] = t;
                  }

                else if (sf_str_eq_rCp (cv, "else"))
                  {
                    stmt_t t;

                    if (arr[i + 1].type == TOK_IDENTIFIER
                        && sf_str_eq_rCp (
                            arr[i + 1].v.t_ident.v,
                            "if")) // .is_reserved is always 1 here
                      {
                        t = _sf_stmt_elseifblockgen (arr, i, &i);
                      }
                    else
                      {
                        t = _sf_stmt_elseblockgen (arr, i, &i);
                      }

                    res = sfrealloc (res, (rc + 1) * sizeof (*res));
                    res[rc++] = t;
                  }

                else if (sf_str_eq_rCp (cv, "for"))
                  {
                    stmt_t t = _sf_stmt_forblockgen (arr, i, &i);

                    res = sfrealloc (res, (rc + 1) * sizeof (*res));
                    res[rc++] = t;
                  }

                else if (sf_str_eq_rCp (cv, "fun"))
                  {
                    stmt_t t = _sf_stmt_fundeclgen (arr, i, &i);

                    res = sfrealloc (res, (rc + 1) * sizeof (*res));
                    res[rc++] = t;
                  }
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
  res.type = -1;

  size_t i = 0;
  int gb = 0;

  while (i < len)
    {
      tok_t c = arr[i];

      if (c.type == TOK_OPERATOR)
        {
          sf_charptr p = c.v.t_op.v;

          if (sf_str_inStr (")]}", p))
            gb--;
        }

      if (gb)
        goto l_end;

      if (c.type == TOK_OPERATOR)
        {
          sf_charptr p = c.v.t_op.v;

          if (sf_str_inStr ("([{", p))
            gb++;
        }

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
                sf_charptr p = c.v.t_ident.v;

                if (sf_str_eq_rCp (p, "to"))
                  {
                    expr_t pres_res = res;

                    res.type = EXPR_TOSTEPTYPE;
                    res.v.to_step.lval = sfmalloc (sizeof (expr_t));
                    *res.v.to_step.lval = pres_res;

                    res.v.to_step.rval = NULL;
                    res.v.to_step.e_step = NULL;
                    res.v.to_step.e_type = NULL;

                    int gb = 0;
                    int step_idx = -1;
                    int type_idx = -1;

                    for (size_t j = i + 1; j < len; j++)
                      {
                        tok_t d = arr[j];

                        if (d.type == TOK_OPERATOR)
                          {
                            sf_charptr q = d.v.t_op.v;

                            if (sf_str_inStr ("([{", q))
                              gb++;

                            else if (sf_str_inStr (")]}", q))
                              gb--;
                          }

                        if (d.type == TOK_IDENTIFIER
                            && sf_str_eq_rCp (d.v.t_ident.v, "step")
                            && !gb) // is_reserved is always 1 here
                          {
                            step_idx = j;
                          }

                        if (d.type == TOK_IDENTIFIER
                            && sf_str_eq_rCp (d.v.t_ident.v, "type")
                            && !gb) // is_reserved is always 1 here
                          {
                            type_idx = j;
                          }

                        if (step_idx != -1 && type_idx != -1)
                          break;
                      }

                    // if (step_idx != -1)
                    //   {
                    //     if (type_idx == -1)
                    //       {
                    //         res.v.to_step.e_step = sfmalloc (sizeof
                    //         (expr_t)); *res.v.to_step.e_step =
                    //         sf_ast_exprgen (
                    //             arr + step_idx + 1, len - step_idx - 1);
                    //       }
                    //     else
                    //       {
                    //         res.v.to_step.e_step = sfmalloc (sizeof
                    //         (expr_t)); *res.v.to_step.e_step =
                    //         sf_ast_exprgen (
                    //             arr + step_idx + 1, type_idx - step_idx -
                    //             1);

                    //         goto l2;
                    //       }
                    //   }

                    // if (type_idx != -1)
                    //   {
                    //   l2:;
                    //     res.v.to_step.e_type = sfmalloc (sizeof (expr_t));
                    //     *res.v.to_step.e_type = sf_ast_exprgen (
                    //         arr + type_idx + 1, len - type_idx - 1);
                    //   }

                    if (step_idx != -1 && type_idx != -1)
                      {
                        if (step_idx < type_idx)
                          {
                            res.v.to_step.e_step = sfmalloc (sizeof (expr_t));

                            *res.v.to_step.e_step = sf_ast_exprgen (
                                arr + step_idx + 1, type_idx - step_idx - 1);

                            res.v.to_step.e_type = sfmalloc (sizeof (expr_t));

                            *res.v.to_step.e_type = sf_ast_exprgen (
                                arr + type_idx + 1, len - type_idx - 1);

                            res.v.to_step.rval = sfmalloc (sizeof (expr_t));
                            *res.v.to_step.rval = sf_ast_exprgen (
                                arr + i + 1, step_idx - i - 1);
                          }
                        else
                          {
                            res.v.to_step.e_step = sfmalloc (sizeof (expr_t));

                            *res.v.to_step.e_step = sf_ast_exprgen (
                                arr + step_idx + 1, len - step_idx - 1);

                            res.v.to_step.e_type = sfmalloc (sizeof (expr_t));

                            *res.v.to_step.e_type = sf_ast_exprgen (
                                arr + type_idx + 1, step_idx - type_idx - 1);

                            res.v.to_step.rval = sfmalloc (sizeof (expr_t));
                            *res.v.to_step.rval = sf_ast_exprgen (
                                arr + i + 1, type_idx - i - 1);
                          }
                      }

                    else if (step_idx != -1 && type_idx == -1)
                      {
                        res.v.to_step.e_step = sfmalloc (sizeof (expr_t));

                        *res.v.to_step.e_step = sf_ast_exprgen (
                            arr + step_idx + 1, len - step_idx - 1);

                        res.v.to_step.rval = sfmalloc (sizeof (expr_t));
                        *res.v.to_step.rval
                            = sf_ast_exprgen (arr + i + 1, step_idx - i - 1);
                      }

                    else if (step_idx == -1 && type_idx != -1)
                      {
                        res.v.to_step.e_type = sfmalloc (sizeof (expr_t));

                        *res.v.to_step.e_type = sf_ast_exprgen (
                            arr + type_idx + 1, len - type_idx - 1);

                        res.v.to_step.rval = sfmalloc (sizeof (expr_t));
                        *res.v.to_step.rval
                            = sf_ast_exprgen (arr + i + 1, type_idx - i - 1);
                      }

                    else
                      {
                        res.v.to_step.rval = sfmalloc (sizeof (expr_t));
                        *res.v.to_step.rval
                            = sf_ast_exprgen (arr + i + 1, len - i - 1);
                      }

                    goto end;
                  }
                else if (sf_str_eq_rCp (p, "in"))
                  {
                    expr_t pres_res = res;

                    res.type = EXPR_INCLAUSE;
                    res.v.in_clause.lval = sfmalloc (sizeof (expr_t));
                    *res.v.in_clause.lval = pres_res;

                    res.v.in_clause.rval = sfmalloc (sizeof (expr_t));
                    *res.v.in_clause.rval
                        = sf_ast_exprgen (arr + i + 1, len - i - 1);

                    goto end;
                  }
              }

            else if (c.v.t_ident.is_bool)
              {
                res.type = EXPR_CONSTANT;
                res.v.e_const.type = CONST_BOOL;
                res.v.e_const.v.c_bool.v
                    = sf_str_eq_rCp (c.v.t_ident.v, SF_BOOL_TRUE_REPR);
              }

            else if (sf_str_eq_rCp (c.v.t_ident.v, SF_DTYPE_NONE_REPR))
              {
                res.type = EXPR_CONSTANT;
                res.v.e_const.type = CONST_NONE;
              }

            else
              {
                res.type = EXPR_VAR;
                res.v.var.name
                    = sf_str_new_fromStr (SFCPTR_TOSTR (c.v.t_ident.v));
              }
          }
          break;
        case TOK_OPERATOR:
          {
            sf_charptr p = c.v.t_op.v;

            if (sf_str_eq_rCp (p, "=="))
              {
                expr_t pres_res = res;

                res.type = EXPR_CONDITIONAL_EQEQ;
                res.v.expr_conditional.lval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.lval = pres_res;
                res.v.expr_conditional.rval = NULL;

                expr_t rv = sf_ast_exprgen (arr + i + 1, len - i - 1);
                res.v.expr_conditional.rval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.rval = rv;

                goto end;
              }

            else if (sf_str_eq_rCp (p, "!="))
              {
                expr_t pres_res = res;

                res.type = EXPR_CONDITIONAL_NEQ;
                res.v.expr_conditional.lval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.lval = pres_res;
                res.v.expr_conditional.rval = NULL;

                expr_t rv = sf_ast_exprgen (arr + i + 1, len - i - 1);
                res.v.expr_conditional.rval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.rval = rv;

                goto end;
              }

            else if (sf_str_eq_rCp (p, ">"))
              {
                expr_t pres_res = res;

                res.type = EXPR_CONDITIONAL_GT;
                res.v.expr_conditional.lval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.lval = pres_res;
                res.v.expr_conditional.rval = NULL;

                expr_t rv = sf_ast_exprgen (arr + i + 1, len - i - 1);
                res.v.expr_conditional.rval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.rval = rv;

                goto end;
              }

            else if (sf_str_eq_rCp (p, "<"))
              {
                expr_t pres_res = res;

                res.type = EXPR_CONDITIONAL_LT;
                res.v.expr_conditional.lval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.lval = pres_res;
                res.v.expr_conditional.rval = NULL;

                expr_t rv = sf_ast_exprgen (arr + i + 1, len - i - 1);
                res.v.expr_conditional.rval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.rval = rv;

                goto end;
              }

            else if (sf_str_eq_rCp (p, ">="))
              {
                expr_t pres_res = res;

                res.type = EXPR_CONDITIONAL_GTEQ;
                res.v.expr_conditional.lval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.lval = pres_res;
                res.v.expr_conditional.rval = NULL;

                expr_t rv = sf_ast_exprgen (arr + i + 1, len - i - 1);
                res.v.expr_conditional.rval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.rval = rv;

                goto end;
              }

            else if (sf_str_eq_rCp (p, "<="))
              {
                expr_t pres_res = res;

                res.type = EXPR_CONDITIONAL_LTEQ;
                res.v.expr_conditional.lval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.lval = pres_res;
                res.v.expr_conditional.rval = NULL;

                expr_t rv = sf_ast_exprgen (arr + i + 1, len - i - 1);
                res.v.expr_conditional.rval = sfmalloc (sizeof (expr_t));
                *res.v.expr_conditional.rval = rv;

                goto end;
              }

            else if (sf_str_eq_rCp (p, "="))
              {
                /* vardecl */
                expr_t pres_res = res;

                res.type = EXPR_VAR_DECL;
                res.v.var_decl.name = sfmalloc (sizeof (expr_t));
                *res.v.var_decl.name = pres_res;

                expr_t rv = sf_ast_exprgen (arr + i + 1, len - i - 1);
                res.v.var_decl.val = sfmalloc (sizeof (expr_t));
                *res.v.var_decl.val = rv;

                goto end;
              }

            else if (sf_str_eq_rCp (p, "["))
              {
                if (res.type == -1)
                  {
                    int comma_count = 0;
                    int gb = 0;
                    size_t end_idx = i;

                    int saw_some_token = 0;
                    int comma_idxs[SF_FUN_ARG_LIMIT - 1];

                    for (size_t j = i + 1; j < len; j++)
                      {
                        tok_t d = arr[j];

                        if (d.type == TOK_OPERATOR)
                          {
                            sf_charptr p = d.v.t_op.v;

                            if (sf_str_eq_rCp (p, "]") && !gb)
                              {
                                end_idx = j;
                                break;
                              }

                            if (sf_str_eq_rCp (p, ",") && !gb)
                              {
                                if (comma_count == SF_FUN_ARG_LIMIT - 1)
                                  {
                                    e_printf ("Argument limit reached in "
                                              "sf_ast_exprgen()\n");

                                    goto end;
                                  }

                                comma_idxs[comma_count++] = j;
                              }

                            if (sf_str_inStr ("([{", p))
                              gb++;

                            else if (sf_str_inStr (")]}", p))
                              gb--;
                          }

                        if (d.type != TOK_NEWLINE && d.type != TOK_SPACE)
                          saw_some_token = 1;
                      }

                    assert (end_idx != i);

                    res.type = EXPR_ARRAY;
                    size_t vlc = 0;

                    if (saw_some_token)
                      vlc = comma_count + 1;

                    // else
                    //   vlc = 0;

                    res.v.e_array.val_count = vlc;
                    comma_idxs[comma_count++]
                        = end_idx; // to get last argument

                    if (vlc)
                      {
                        expr_t *vls = sfmalloc (vlc * sizeof (*vls));

                        for (size_t j = 0; j < comma_count; j++)
                          {
                            if (j)
                              {
                                vls[j] = sf_ast_exprgen (
                                    arr + comma_idxs[j - 1] + 1,
                                    comma_idxs[j] - comma_idxs[j - 1] - 1);
                              }
                            else
                              {
                                vls[j] = sf_ast_exprgen (
                                    arr + i + 1, comma_idxs[j] - i - 1);
                              }
                          }

                        res.v.e_array.vals = vls;
                      }
                    else
                      res.v.e_array.vals = NULL;

                    i = end_idx - 1;
                    goto l_end;
                  }
                else
                  {
                    expr_t pres_res = res;

                    res.type = EXPR_IDX_ACCESS;
                    res.v.e_idx_access.name = NULL;
                    res.v.e_idx_access.val = NULL;

                    int comma_count = 0;
                    int gb = 0;
                    size_t end_idx = i;

                    for (size_t j = i + 1; j < len; j++)
                      {
                        tok_t d = arr[j];

                        if (d.type == TOK_OPERATOR)
                          {
                            sf_charptr p = d.v.t_op.v;

                            if (sf_str_eq_rCp (p, "]") && !gb)
                              {
                                end_idx = j;
                                break;
                              }

                            if (sf_str_inStr ("([{", p))
                              gb++;

                            else if (sf_str_inStr (")]}", p))
                              gb--;
                          }
                      }

                    assert (end_idx != i && end_idx != i + 1);

                    res.v.e_idx_access.name = sfmalloc (sizeof (expr_t));
                    *res.v.e_idx_access.name = pres_res;

                    res.v.e_idx_access.val = sfmalloc (sizeof (expr_t));
                    *res.v.e_idx_access.val
                        = sf_ast_exprgen (arr + i + 1, end_idx - i - 1);

                    // printf ("{%d}\n", res.v.e_idx_access.val->type);

                    i = end_idx - 1;
                    goto l_end;
                  }
              }

            else if (sf_str_eq_rCp (p, "("))
              {
                expr_t pres_res = res;

                res.type = EXPR_FUN_CALL;
                size_t last_idx = i + 1;
                size_t end_idx = i;
                int gb = 0;

                expr_t *args = NULL;
                size_t argc = 0;

                for (size_t j = last_idx; j < len; j++)
                  {
                    tok_t d = arr[j];

                    if (d.type == TOK_OPERATOR)
                      {
                        sf_charptr p = d.v.t_op.v;

                        if (sf_str_eq_rCp (p, ")") && !gb)
                          {
                            if (j != last_idx)
                              {
                                args = sfrealloc (args,
                                                  (argc + 1) * sizeof (*args));

                                args[argc++] = sf_ast_exprgen (arr + last_idx,
                                                               j - last_idx);
                              }

                            end_idx = j;
                            break;
                          }

                        if (sf_str_eq_rCp (p, ",") && !gb)
                          {
                            args = sfrealloc (args,
                                              (argc + 1) * sizeof (*args));

                            args[argc++] = sf_ast_exprgen (arr + last_idx,
                                                           j - last_idx);

                            last_idx = j + 1;
                            continue;
                          }

                        if (sf_str_inStr ("([{", p))
                          gb++;

                        else if (sf_str_inStr (")]}", p))
                          gb--;
                      }
                  }

                assert (end_idx != i);

                res.v.fun_call.arg_count = argc;
                res.v.fun_call.args = args;
                res.v.fun_call.name = sfmalloc (sizeof (expr_t));
                *res.v.fun_call.name = pres_res;

                i = end_idx - 1;
                goto l_end;
              }

            else if (sf_str_inStr ("+-*/%", p))
              {
                __sfapostfix_tree *atree = sfmalloc (
                    (len
                     + 1 /* +1 for next-breakpoint condition (see first if statement below) */)
                    * sizeof (*atree));
                size_t ac = 0;

                atree[ac] = (__sfapostfix_tree){
                  .is_op = 0,
                  .v.val = sfmalloc (sizeof (expr_t)),
                };

                if (res.type == -1
                    && (sf_str_eq_rCp (p, "-") || sf_str_eq_rCp (p, "+")))
                  {
                    res.type = EXPR_CONSTANT;
                    res.v.e_const.type = CONST_INT;
                    res.v.e_const.v.c_int.v = 0;
                  }

                *atree[ac].v.val = res;
                ac++;

                atree[ac++] = (__sfapostfix_tree){
                  .is_op = 1,
                  .v.op = sfstrdup (SFCPTR_TOSTR (p)),
                };

                int all_constants = (res.type == EXPR_CONSTANT);

                int gb = 0;
                size_t last_op_idx = i;
                for (size_t j = i + 1; j < len; j++)
                  {
                    tok_t *c = &arr[j];

                    if (c->type == TOK_OPERATOR && !gb)
                      {
                        sf_charptr p = c->v.t_op.v;

                        if (sf_str_inStr ("([{", p))
                          gb++;

                        if (sf_str_inStr (")]}", p))
                          gb--;

                        if (sf_str_inStr ("+-*/", p))
                          {
                            expr_t ev = sf_ast_exprgen (arr + last_op_idx + 1,
                                                        j - last_op_idx - 1);

                            all_constants
                                = all_constants && (ev.type == EXPR_CONSTANT);

                            expr_t *evp = sfmalloc (sizeof (*evp));
                            *evp = ev;

                            atree[ac++] = (__sfapostfix_tree){
                              .is_op = 0,
                              .v.val = evp,
                            };

                            atree[ac++] = (__sfapostfix_tree){
                              .is_op = 1,
                              .v.op = sfstrdup (SFCPTR_TOSTR (p)),
                            };

                            last_op_idx = j;
                          }
                      }
                  }

                expr_t ev = sf_ast_exprgen (arr + last_op_idx + 1,
                                            len - last_op_idx - 1);

                all_constants = all_constants && (ev.type == EXPR_CONSTANT);

                expr_t *evp = sfmalloc (sizeof (*evp));
                *evp = ev;

                atree[ac++] = (__sfapostfix_tree){
                  .is_op = 0,
                  .v.val = evp,
                };

                /* INFIX TREE */
                /* for (size_t j = 0; j < ac; j++)
                  {
                    if (atree[j].is_op)
                      {
                        printf ("op: %s\n", atree[j].v.op);
                      }
                    else
                      {
                        sf_ast_exprprint (*atree[j].v.val);
                      }
                  } */

                sf_arith_infix_to_postfix (&atree, &ac);

                /* POSTFIX TREE */
                /* for (size_t j = 0; j < ac; j++)
                  {
                    if (atree[j].is_op)
                      {
                        printf ("op: %s\n", atree[j].v.op);
                      }
                    else
                      {
                        sf_ast_exprprint (*atree[j].v.val);
                      }
                  } */

                res.type = EXPR_ARITHMETIC;
                res.v.e_arith.tree = sf_arith_pft_to_tree (atree, ac);

                if (all_constants)
                  {
                    double arith_res = sf_arith_eval_tree (res.v.e_arith.tree);

                    res.type = EXPR_CONSTANT;
                    res.v.e_const.type = CONST_FLOAT;
                    res.v.e_const.v.c_float.v = arith_res;
                  }

                sffree (atree);
                goto end;
              }
          }

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
  assert (res.v.var_decl.name->type != -1);
  assert (res.v.var_decl.val->type != -1);

  if (jptr != NULL)
    {
      *jptr = rsi - 1;
    }

  return res;
}

stmt_t
_sf_stmt_opeqgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t t = _sf_stmt_vdgen (arr, idx, jptr);
  stmt_t res;

  res.type = STMT_OPEQ;
  res.v.opeq_decl.name = t.v.var_decl.name;
  res.v.opeq_decl.val = t.v.var_decl.val;
  res.v.opeq_decl.op = sf_str_new_fromStr (SFCPTR_TOSTR (arr[idx].v.t_op.v));

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

  // assert (gb == 0);
  gb = 0;

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

  // for (size_t i = idx + 1; i < rsi; i++)
  //   {
  //     sf_tokenizer_print (arr[i]);
  //   }

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
      // for (size_t i = 0; i < aai; i++)
      //   {
      //     sf_tokenizer_print (arg_arr[i]);
      //   }

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

stmt_t
_sf_stmt_ifblockgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;
  res.type = STMT_IF_BLOCK;

  size_t cei = 0; // condition end index
  int gb = 0;

  for (size_t i = idx + 1; arr[i].type != TOK_EOF; i++)
    {
      tok_t c = arr[i];

      if (c.type == TOK_NEWLINE && !gb)
        {
          cei = i;
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

  assert (cei); // Syntax error

  res.v.blk_if.cond = sfmalloc (sizeof (expr_t));
  *res.v.blk_if.cond = sf_ast_exprgen (arr + idx + 1, cei - idx - 1);

  tok_t *body_end
      = _sf_stmt_getblkend (arr + cei, _sf_stmt_gettbsp (arr, idx));

  tok_t pres_end = *body_end;
  *body_end = (tok_t){
    .type = TOK_EOF,
  }; // end file at end of block (temporarily)

  size_t sts = 0; // statement tree size
  stmt_t *stree = sf_ast_stmtgen (arr + cei, &sts);

  *body_end = pres_end;
  res.v.blk_if.body = stree;
  res.v.blk_if.body_count = sts;

end:
  if (jptr != NULL)
    {
      *jptr = (body_end - arr) - 1;
    }

  return res;
}

stmt_t
_sf_stmt_elseifblockgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t p = _sf_stmt_ifblockgen (arr, idx + 1, jptr);
  stmt_t res;

  res.type = STMT_ELSEIF_BLOCK;
  res.v.blk_elseif.body = p.v.blk_if.body;
  res.v.blk_elseif.body_count = p.v.blk_if.body_count;
  res.v.blk_elseif.cond = p.v.blk_if.cond;

  return res;
}

stmt_t
_sf_stmt_elseblockgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;
  res.type = STMT_ELSE_BLOCK;

  tok_t *body_end
      = _sf_stmt_getblkend (arr + idx + 1, _sf_stmt_gettbsp (arr, idx));

  tok_t pres_end = *body_end;
  *body_end = (tok_t){
    .type = TOK_EOF,
  };

  size_t sts = 0;
  stmt_t *stree = sf_ast_stmtgen (arr + idx + 1, &sts);

  *body_end = pres_end;
  res.v.blk_else.body = stree;
  res.v.blk_else.body_count = sts;

end:
  if (jptr != NULL)
    {
      *jptr = (body_end - arr) - 1;
    }

  return res;
}

stmt_t
_sf_stmt_forblockgen (tok_t *arr, size_t idx, size_t *jptr)
{
  // similar to ifblockgen

  stmt_t res;
  res.type = STMT_FOR_BLOCK;

  size_t cei = 0; // condition end index
  int gb = 0;

  for (size_t i = idx + 1; arr[i].type != TOK_EOF; i++)
    {
      tok_t c = arr[i];

      if (c.type == TOK_NEWLINE && !gb)
        {
          cei = i;
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

  assert (cei); // Syntax error

  res.v.blk_for.cond = sfmalloc (sizeof (expr_t));
  *res.v.blk_for.cond = sf_ast_exprgen (arr + idx + 1, cei - idx - 1);

  tok_t *body_end
      = _sf_stmt_getblkend (arr + cei, _sf_stmt_gettbsp (arr, idx));

  tok_t pres_end = *body_end;
  *body_end = (tok_t){
    .type = TOK_EOF,
  }; // end file at end of block (temporarily)

  size_t sts = 0; // statement tree size
  stmt_t *stree = sf_ast_stmtgen (arr + cei, &sts);

  *body_end = pres_end;
  res.v.blk_for.body = stree;
  res.v.blk_for.body_count = sts;

end:
  if (jptr != NULL)
    {
      *jptr = (body_end - arr) - 1;
    }

  return res;
}

stmt_t
_sf_stmt_fundeclgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;

  res.type = STMT_FUN_DECL;
  res.v.fun_decl.arg_count = 0;
  res.v.fun_decl.args = NULL;
  res.v.fun_decl.body = NULL;
  res.v.fun_decl.body_count = 0;
  res.v.fun_decl.name = NULL;

  size_t name_end_idx = idx;
  int gb = 0;
  int takes_no_args = 0;

  for (size_t i = idx + 1; arr[i].type != TOK_EOF; i++)
    {
      tok_t d = arr[i];

      if (d.type == TOK_OPERATOR)
        {
          sf_charptr q = d.v.t_op.v;

          if (sf_str_eq_rCp (q, "(") && !gb)
            {
              name_end_idx = i;
              break;
            }

          if (sf_str_inStr ("([{", q))
            gb++;

          else if (sf_str_inStr (")]}", q))
            gb--;
        }

      /*
        fun test
          return "Takes no arguments"
      */
      if (d.type == TOK_NEWLINE && !gb)
        {
          takes_no_args = 1;
          name_end_idx = i - 1;
          break;
        }
    }

  assert (name_end_idx != idx);

  expr_t *nam = sfmalloc (sizeof (*nam));
  *nam = sf_ast_exprgen (arr + idx + 1, name_end_idx - idx - 1);

  res.v.fun_decl.name = nam;

  expr_t *args = NULL;
  size_t argc = 0;
  size_t arg_end_idx = idx;
  size_t last_arg_idx = name_end_idx + 1;

  gb = 0;

  if (!takes_no_args)
    {
      for (size_t i = name_end_idx + 1; arr[i].type != TOK_EOF; i++)
        {
          tok_t d = arr[i];

          if (d.type == TOK_OPERATOR)
            {
              sf_charptr q = d.v.t_op.v;

              if (sf_str_eq_rCp (q, ")") && !gb)
                {
                  if (i != name_end_idx + 1)
                    {
                      args = sfrealloc (args, (argc + 1) * sizeof (*args));
                      args[argc++] = sf_ast_exprgen (arr + last_arg_idx,
                                                     i - last_arg_idx);
                    }

                  arg_end_idx = i;
                  break;
                }

              if (sf_str_eq_rCp (q, ",") && !gb)
                {
                  args = sfrealloc (args, (argc + 1) * sizeof (*args));
                  args[argc++]
                      = sf_ast_exprgen (arr + last_arg_idx, i - last_arg_idx);

                  last_arg_idx = i + 1;
                }

              if (sf_str_inStr ("([{", q))
                gb++;

              else if (sf_str_inStr (")]}", q))
                gb--;
            }
        }
    }
  else
    {
      arg_end_idx = name_end_idx;
    }

  assert (arg_end_idx != idx);

  res.v.fun_decl.args = args;
  res.v.fun_decl.arg_count = argc;

  tok_t *body_end = _sf_stmt_getblkend (arr + arg_end_idx + 1,
                                        _sf_stmt_gettbsp (arr, idx));

  tok_t pres_end = *body_end;
  *body_end = (tok_t){
    .type = TOK_EOF,
  }; // end file at end of block (temporarily)

  size_t sts = 0; // statement tree size
  stmt_t *stree = sf_ast_stmtgen (arr + arg_end_idx + 1, &sts);

  *body_end = pres_end;
  res.v.fun_decl.body = stree;
  res.v.fun_decl.body_count = sts;

end:
  if (jptr != NULL)
    {
      *jptr = (body_end - arr) - 1;
    }

  return res;
}

size_t
_sf_stmt_gettbsp (tok_t *arr, size_t idx)
{
  size_t r = 0;
  int gb = 0;

  for (int i = (int)idx; i >= 0; i--)
    {
      tok_t c = arr[i];

      if (c.type == TOK_NEWLINE && !gb)
        {
          break;
        }

      if (c.type == TOK_SPACE && !gb)
        {
          r = (size_t)c.v.t_space.v;
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

  return r;
}

tok_t *
_sf_stmt_getblkend (tok_t *arr, size_t tbs)
{
  // printf ("(%d)\n", tbs);
  int gb = 0;
  int saw_nl = 0;

  size_t i;
  for (i = 0; arr[i].type != TOK_EOF; i++)
    {
      tok_t c = arr[i];

      if (c.type != TOK_SPACE && c.type != TOK_NEWLINE && saw_nl)
        break;

      if (c.type == TOK_NEWLINE && !gb)
        {
          if (saw_nl)
            continue;
          saw_nl = 1;
        }

      if (c.type == TOK_SPACE && !gb)
        {
          saw_nl = 0;
          if (c.v.t_space.v <= tbs)
            {
              if (arr[i + 1].type
                  == TOK_NEWLINE) // empty line with some tabspaces
                ;
              else
                break;
            }
        }

      if (c.type == TOK_OPERATOR)
        {
          if (sf_str_inStr ("({[", c.v.t_op.v))
            gb++;
          else if (sf_str_inStr (")}]", c.v.t_op.v))
            gb--;
        }
    }

  return arr + i;
}

SF_API void
sf_ast_freeObj (obj_t **obj)
{
  obj_t *p = *obj;

  if (p == NULL)
    {
      e_printf ("*obj is NULL at sf_ast_freeObj()\n");
      return;
    }

  switch (p->type)
    {
    case OBJ_ARRAY:
      {
        sf_array_free (p->v.o_array.v);
      }
      break;

    case OBJ_FUN:
      {
        sf_fun_free (p->v.o_fun.f);
      }
      break;

    case OBJ_CONST:
      {
        switch (p->v.o_const.type)
          {
          case CONST_STRING:
            {
              sf_str_free (&p->v.o_const.v.c_string.v);
            }
            break;

          default:
            break;
          }
      }
      break;

    default:
      break;
    }

  sffree (*obj);
}

void
__sfexprprintarith (void *arg)
{
  struct _sfa_treetok_s *cur = arg;

  if (cur->is_op)
    printf ("op: %s\n", cur->v.op);

  else if (cur->is_llnode)
    {
      char *r = sf_parser_objRepr (NULL, (obj_t *)cur->node->val);
      printf ("llnode: %s\n", r);

      sffree (r);
    }

  else
    sf_ast_exprprint (*cur->v.val);
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
            printf ("const_float %f\n", e.v.e_const.v.c_float.v);
            break;

          case CONST_INT:
            printf ("const_int %d\n", e.v.e_const.v.c_int.v);
            break;

          case CONST_STRING:
            printf ("const_string %s\n", e.v.e_const.v.c_string.v);
            break;

          case CONST_NONE:
            printf ("const_none\n");
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
            sf_ast_exprprint (e.v.fun_call.args[i]);
          }
      }
      break;

    case EXPR_CONDITIONAL_EQEQ:
      {
        printf ("conditional_eqeq\n");

        printf ("lval\n");
        sf_ast_exprprint (*e.v.expr_conditional.lval);

        printf ("rval\n");
        sf_ast_exprprint (*e.v.expr_conditional.rval);
      }
      break;

    case EXPR_CONDITIONAL_NEQ:
      {
        printf ("conditional_neq\n");

        printf ("lval\n");
        sf_ast_exprprint (*e.v.expr_conditional.lval);

        printf ("rval\n");
        sf_ast_exprprint (*e.v.expr_conditional.rval);
      }
      break;

    case EXPR_CONDITIONAL_GT:
      {
        printf ("conditional_gt\n");

        printf ("lval\n");
        sf_ast_exprprint (*e.v.expr_conditional.lval);

        printf ("rval\n");
        sf_ast_exprprint (*e.v.expr_conditional.rval);
      }
      break;

    case EXPR_CONDITIONAL_LT:
      {
        printf ("conditional_lt\n");

        printf ("lval\n");
        sf_ast_exprprint (*e.v.expr_conditional.lval);

        printf ("rval\n");
        sf_ast_exprprint (*e.v.expr_conditional.rval);
      }
      break;

    case EXPR_CONDITIONAL_GTEQ:
      {
        printf ("conditional_gteq\n");

        printf ("lval\n");
        sf_ast_exprprint (*e.v.expr_conditional.lval);

        printf ("rval\n");
        sf_ast_exprprint (*e.v.expr_conditional.rval);
      }
      break;

    case EXPR_CONDITIONAL_LTEQ:
      {
        printf ("conditional_lteq\n");

        printf ("lval\n");
        sf_ast_exprprint (*e.v.expr_conditional.lval);

        printf ("rval\n");
        sf_ast_exprprint (*e.v.expr_conditional.rval);
      }
      break;

    case EXPR_TOSTEPTYPE:
      {
        printf ("to_step\n");

        printf ("lval\n");
        sf_ast_exprprint (*e.v.to_step.lval);

        printf ("rval\n");
        sf_ast_exprprint (*e.v.to_step.rval);

        printf ("step\n");

        if (e.v.to_step.e_step)
          sf_ast_exprprint (*e.v.to_step.e_step);
        else
          printf ("null\n");

        printf ("type\n");

        if (e.v.to_step.e_type)
          sf_ast_exprprint (*e.v.to_step.e_type);
        else
          printf ("null\n");
      }
      break;

    case EXPR_INCLAUSE:
      {
        printf ("in_clause\n");

        printf ("lval\n");
        sf_ast_exprprint (*e.v.in_clause.lval);

        printf ("rval\n");
        sf_ast_exprprint (*e.v.in_clause.rval);
      }
      break;

    case EXPR_ARRAY:
      {
        printf ("array\n");

        printf ("args (%d)\n", e.v.e_array.val_count);
        for (size_t i = 0; i < e.v.e_array.val_count; i++)
          {
            printf ("[%d] ", i);
            sf_ast_exprprint (e.v.e_array.vals[i]);
          }
      }
      break;

    case EXPR_IDX_ACCESS:
      {
        printf ("idx_access\n");

        printf ("name\n");
        sf_ast_exprprint (*e.v.e_idx_access.name);

        printf ("val\n");
        sf_ast_exprprint (*e.v.e_idx_access.val);
      }
      break;

    case EXPR_ARITHMETIC:
      {
        printf ("expr_arith\n");
        sf_tree_traverse_pre (e.v.e_arith.tree, __sfexprprintarith);
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
        printf ("fun_decl\nname\n");
        sf_ast_exprprint (*s.v.fun_decl.name);

        printf ("args (%d)\n", s.v.fun_decl.arg_count);
        for (size_t i = 0; i < s.v.fun_decl.arg_count; i++)
          {
            sf_ast_exprprint (s.v.fun_decl.args[i]);
          }

        printf ("body (%d)\n", s.v.fun_decl.body_count);
        for (size_t i = 0; i < s.v.fun_decl.body_count; i++)
          {
            sf_ast_stmtprint (s.v.fun_decl.body[i]);
          }
      }
      break;

    case STMT_IF_BLOCK:
      {
        printf ("if_block\n");

        printf ("condition\n");
        sf_ast_exprprint (*s.v.blk_if.cond);

        printf ("body %d\n", s.v.blk_if.body_count);
        for (size_t i = 0; i < s.v.blk_if.body_count; i++)
          {
            printf ("(%d) ", i);
            sf_ast_stmtprint (s.v.blk_if.body[i]);
          }
      }
      break;

    case STMT_ELSEIF_BLOCK:
      {
        printf ("elseif_block\n");

        printf ("condition\n");
        sf_ast_exprprint (*s.v.blk_elseif.cond);

        printf ("body %d\n", s.v.blk_elseif.body_count);
        for (size_t i = 0; i < s.v.blk_elseif.body_count; i++)
          {
            printf ("(%d) ", i);
            sf_ast_stmtprint (s.v.blk_elseif.body[i]);
          }
      }
      break;

    case STMT_ELSE_BLOCK:
      {
        printf ("else_block\n");

        printf ("body %d\n", s.v.blk_else.body_count);
        for (size_t i = 0; i < s.v.blk_else.body_count; i++)
          {
            printf ("(%d) ", i);
            sf_ast_stmtprint (s.v.blk_else.body[i]);
          }
      }
      break;

    case STMT_FOR_BLOCK:
      {
        printf ("for_block\n");

        printf ("condition\n");
        sf_ast_exprprint (*s.v.blk_for.cond);

        printf ("body %d\n", s.v.blk_for.body_count);
        for (size_t i = 0; i < s.v.blk_for.body_count; i++)
          {
            printf ("(%d) ", i);
            sf_ast_stmtprint (s.v.blk_for.body[i]);
          }
      }
      break;

    case STMT_OPEQ:
      {
        printf ("op_eq_decl\n");

        printf ("name\n");
        sf_ast_exprprint (*s.v.opeq_decl.name);

        printf ("op\n%s\n", s.v.opeq_decl.op);

        printf ("value\n");
        sf_ast_exprprint (*s.v.opeq_decl.val);
      }
      break;

    default:
      printf ("unknown statement %d\n", s.type);
      break;
    }
}