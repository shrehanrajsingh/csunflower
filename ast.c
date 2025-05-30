#include "ast.h"
#include "parser.h"
#include "sfarray.h"

/* forward declarations */
stmt_t _sf_stmt_vdgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_opeqgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_fcgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_ifblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_elseblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_elseifblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_forblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_fundeclgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_classdeclgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_whileblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_importgen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_returngen (tok_t *_arr, size_t _idx, size_t *_jumper);
stmt_t _sf_stmt_withblockgen (tok_t *_arr, size_t _idx, size_t *_jumper);

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

/* sf add to stmt arr */
void
satsr (stmt_t *arr, size_t *p, size_t *cap, stmt_t t)
{
  if ((*p) == (*cap))
    {
      *cap += 32;
      arr = sfrealloc (arr, (*cap) * sizeof (*arr));
    }

  arr[(*p)++] = t;
}

SF_API obj_t *
sf_ast_objnew (int type)
{
  obj_t *t = sfmalloc (sizeof (*t));

  t->type = type;
  t->meta.pa_size = 0;
  t->meta.passargs = NULL;
  t->meta.mem_ref = NULL;

  return t;
}

SF_API stmt_t *
sf_ast_stmtgen (tok_t *arr, size_t *sptr)
{
  size_t rc = 0;
  size_t rcap = 32;
  stmt_t *res = sfmalloc (rcap * sizeof (*res));

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

                /* res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t; */

                satsr (res, &rc, &rcap, t);
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

                /* res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t; */

                satsr (res, &rc, &rcap, t);
                // }
              }

            else if (sf_str_eq_rCp (c.v.t_op.v, "("))
              {
                stmt_t t = _sf_stmt_fcgen (arr, i, &i);

                /* res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t; */

                int tt = t.v.fun_call.name->type;
                if (tt == EXPR_MEM_ACCESS || tt == EXPR_FUN_CALL)
                  {
                    if (tt == EXPR_FUN_CALL)
                      res[rc - 1] = t;

                    else
                      {
                        expr_t *y = t.v.fun_call.name->v.mem_access.parent;

                        if (y && y->type == EXPR_FUN_CALL)
                          res[rc - 1] = t;
                        else
                          satsr (res, &rc, &rcap, t);
                      }
                  }
                else
                  satsr (res, &rc, &rcap, t);
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

                    /* res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = t; */

                    satsr (res, &rc, &rcap, t);
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

                    /* res = sfrealloc (res, (rc + 1) * sizeof (*res));
                      res[rc++] = t; */

                    satsr (res, &rc, &rcap, t);
                  }

                else if (sf_str_eq_rCp (cv, "for"))
                  {
                    stmt_t t = _sf_stmt_forblockgen (arr, i, &i);

                    /* res = sfrealloc (res, (rc + 1) * sizeof (*res));
                        res[rc++] = t; */

                    satsr (res, &rc, &rcap, t);
                  }

                else if (sf_str_eq_rCp (cv, "while"))
                  {
                    stmt_t t = _sf_stmt_whileblockgen (arr, i, &i);

                    satsr (res, &rc, &rcap, t);
                  }

                else if (sf_str_eq_rCp (cv, "fun"))
                  {
                    stmt_t t = _sf_stmt_fundeclgen (arr, i, &i);

                    /* res = sfrealloc (res, (rc + 1) * sizeof (*res));
                        res[rc++] = t; */

                    satsr (res, &rc, &rcap, t);
                  }

                else if (sf_str_eq_rCp (cv, "class"))
                  {
                    stmt_t t = _sf_stmt_classdeclgen (arr, i, &i);

                    /* res = sfrealloc (res, (rc + 1) * sizeof (*res));
                        res[rc++] = t; */

                    satsr (res, &rc, &rcap, t);
                  }

                else if (sf_str_eq_rCp (cv, "import"))
                  {
                    stmt_t t = _sf_stmt_importgen (arr, i, &i);

                    satsr (res, &rc, &rcap, t);
                  }

                else if (sf_str_eq_rCp (cv, "return"))
                  {
                    stmt_t t = _sf_stmt_returngen (arr, i, &i);

                    satsr (res, &rc, &rcap, t);
                  }

                else if (sf_str_eq_rCp (cv, "with"))
                  {
                    stmt_t t = _sf_stmt_withblockgen (arr, i, &i);

                    satsr (res, &rc, &rcap, t);
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

  /*
    Precedence in sunflower follows:
    * `where`
    * `and`, `or`
    * `in` clause
    * `to step type` clause
    * arithmetic operators
  */

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
        goto l4;

      if (c.type == TOK_OPERATOR)
        {
          sf_charptr p = c.v.t_op.v;

          if (sf_str_inStr ("([{", p))
            gb++;
        }

      if (c.type == TOK_IDENTIFIER && c.v.t_ident.is_reserved
          && sf_str_eq_rCp (c.v.t_ident.v, "where") && !gb)
        {
          /*
            `where` clause uses strict syntax structures
            Usage:
            <preceding_expr> where <vname_1> = <vval>, <vname_2> = <vval>
           */
          int comma_count = 0;
          int gb1 = 0;

          for (size_t j = i + 1; j < len; j++)
            {
              tok_t *d = &arr[j];

              if (d->type == TOK_OPERATOR)
                {
                  sf_charptr *op = &d->v.t_op.v;

                  if (sf_str_inStr ("([{", *op))
                    gb1++;

                  if (sf_str_inStr (")]}", *op))
                    gb1--;

                  if (sf_str_eq_rCp (*op, ",") && !gb1)
                    comma_count++;
                }
            }

          sf_charptr *vnms
              = sfmalloc ((comma_count + 1) * sizeof (sf_charptr));
          expr_t *vls = sfmalloc ((comma_count + 1) * sizeof (expr_t));
          size_t vc = 0;
          gb1 = 0;

          for (size_t j = i + 1; j < len; j++)
            {
              tok_t *d = &arr[j];

              if (d->type == TOK_OPERATOR)
                {
                  sf_charptr *op = &d->v.t_op.v;

                  if (sf_str_inStr ("([{", *op))
                    gb1++;

                  if (sf_str_inStr (")]}", *op))
                    gb1--;

                  if (sf_str_eq_rCp (*op, "=") && !gb1)
                    {
                      assert (arr[j - 1].type == TOK_IDENTIFIER
                              && "Syntax Error.");

                      vnms[vc] = arr[j - 1].v.t_ident.v;

                      int gb2 = 0;
                      size_t edidx = j;

                      for (size_t k = j + 1; k < len; k++)
                        {
                          tok_t *e = &arr[k];

                          if (e->type == TOK_OPERATOR)
                            {
                              sf_charptr *op = &e->v.t_op.v;

                              if (sf_str_inStr ("([{", *op))
                                gb2++;

                              if (sf_str_inStr (")]}", *op))
                                gb2--;

                              if (sf_str_eq_rCp (*op, ",") && !gb2)
                                {
                                  edidx = k;
                                  break;
                                }
                            }
                        }

                      if (edidx == j)
                        edidx = len;

                      vls[vc] = sf_ast_exprgen (arr + j + 1, edidx - j - 1);

                      j = edidx;
                      vc++;
                    }
                }
            }

          res.type = EXPR_WHERE;
          res.v.e_where.prev_expr = sfmalloc (sizeof (expr_t));

          *res.v.e_where.prev_expr = sf_ast_exprgen (arr, i);
          res.v.e_where.vnames = vnms;
          res.v.e_where.vsize = vc;
          res.v.e_where.vvals = vls;

          goto end;
        }

    l4:
      i++;
    }

  i = 0;
  gb = 0;

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
        goto l3;

      if (c.type == TOK_OPERATOR)
        {
          sf_charptr p = c.v.t_op.v;

          if (sf_str_inStr ("([{", p))
            gb++;
        }

      if (c.type == TOK_IDENTIFIER && c.v.t_ident.is_reserved
          && sf_str_eq_rCp (c.v.t_ident.v, "and") && !gb)
        {
          expr_t l = sf_ast_exprgen (arr, i),
                 r = sf_ast_exprgen (arr + i + 1, len - i - 1);

          res.type = EXPR_AND;
          res.v.e_and.lhs = sfmalloc (sizeof (expr_t));
          *res.v.e_and.lhs = l;

          res.v.e_and.rhs = sfmalloc (sizeof (expr_t));
          *res.v.e_and.rhs = r;

          goto end;
        }

      if (c.type == TOK_IDENTIFIER && c.v.t_ident.is_reserved
          && sf_str_eq_rCp (c.v.t_ident.v, "or") && !gb)
        {
          expr_t l = sf_ast_exprgen (arr, i),
                 r = sf_ast_exprgen (arr + i + 1, len - i - 1);

          res.type = EXPR_OR;
          res.v.e_or.lhs = sfmalloc (sizeof (expr_t));
          *res.v.e_or.lhs = l;

          res.v.e_or.rhs = sfmalloc (sizeof (expr_t));
          *res.v.e_or.rhs = r;

          goto end;
        }

    l3:
      i++;
    }

  i = 0;
  gb = 0;

  /* check for `in` clause */
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
        goto l0;

      if (c.type == TOK_OPERATOR)
        {
          sf_charptr p = c.v.t_op.v;

          if (sf_str_inStr ("([{", p))
            gb++;
        }

      if (c.type == TOK_IDENTIFIER && c.v.t_ident.is_reserved
          && sf_str_eq_rCp (c.v.t_ident.v, "in") && !gb)
        {
          res = sf_ast_exprgen (arr, i);

          i--; // to counter i++ at l_end
          goto l_end;
        }

    l0:
      i++;
    }

  i = 0;
  gb = 0;

  /* check for `to step type` clause */
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
        goto l1;

      if (c.type == TOK_OPERATOR)
        {
          sf_charptr p = c.v.t_op.v;

          if (sf_str_inStr ("([{", p))
            gb++;
        }

      if (c.type == TOK_IDENTIFIER && c.v.t_ident.is_reserved
          && sf_str_eq_rCp (c.v.t_ident.v, "to") && !gb)
        {
          res = sf_ast_exprgen (arr, i);

          i--; // to counter i++ at l_end
          goto l_end;
        }

    l1:
      i++;
    }

  i = 0;
  gb = 0;

  while (i < len)
    {
      tok_t c = arr[i];

      if (c.type == TOK_OPERATOR)
        {
          sf_charptr p = c.v.t_op.v;

          if (sf_str_inStr ("([{", p))
            gb++;

          if (sf_str_inStr (")]}", p))
            gb--;

          if (!gb)
            {
              if (sf_str_eq_rCp (p, "=="))
                {
                  expr_t pres_res = sf_ast_exprgen (arr, i);

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
                  expr_t pres_res = sf_ast_exprgen (arr, i);

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
                  expr_t pres_res = sf_ast_exprgen (arr, i);

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
                  expr_t pres_res = sf_ast_exprgen (arr, i);

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
                  expr_t pres_res = sf_ast_exprgen (arr, i);

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
                  expr_t pres_res = sf_ast_exprgen (arr, i);

                  res.type = EXPR_CONDITIONAL_LTEQ;
                  res.v.expr_conditional.lval = sfmalloc (sizeof (expr_t));
                  *res.v.expr_conditional.lval = pres_res;
                  res.v.expr_conditional.rval = NULL;

                  expr_t rv = sf_ast_exprgen (arr + i + 1, len - i - 1);
                  res.v.expr_conditional.rval = sfmalloc (sizeof (expr_t));
                  *res.v.expr_conditional.rval = rv;

                  goto end;
                }
            }
        }

    l2:
      i++;
    }

  i = 0;
  gb = 0;

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

#if !defined(SF_DISABLE_THIS)
                else if (sf_str_eq_rCp (p, "this"))
                  {
                    res.type = EXPR_THIS;
                  }
#endif

                else if (sf_str_eq_rCp (p, "not"))
                  {
                    assert (res.type == -1);
                    res.type = EXPR_NOT;
                    expr_t r = sf_ast_exprgen (arr + i + 1, len - i - 1);

                    res.v.e_not.v = sfmalloc (sizeof (expr_t));
                    *res.v.e_not.v = r;

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

            /*if (sf_str_eq_rCp(p, "=="))
            {
              expr_t pres_res = res;

              res.type = EXPR_CONDITIONAL_EQEQ;
              res.v.expr_conditional.lval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.lval = pres_res;
              res.v.expr_conditional.rval = NULL;

              expr_t rv = sf_ast_exprgen(arr + i + 1, len - i - 1);
              res.v.expr_conditional.rval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.rval = rv;

              goto end;
            }

            else if (sf_str_eq_rCp(p, "!="))
            {
              expr_t pres_res = res;

              res.type = EXPR_CONDITIONAL_NEQ;
              res.v.expr_conditional.lval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.lval = pres_res;
              res.v.expr_conditional.rval = NULL;

              expr_t rv = sf_ast_exprgen(arr + i + 1, len - i - 1);
              res.v.expr_conditional.rval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.rval = rv;

              goto end;
            }

            else if (sf_str_eq_rCp(p, ">"))
            {
              expr_t pres_res = res;

              res.type = EXPR_CONDITIONAL_GT;
              res.v.expr_conditional.lval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.lval = pres_res;
              res.v.expr_conditional.rval = NULL;

              expr_t rv = sf_ast_exprgen(arr + i + 1, len - i - 1);
              res.v.expr_conditional.rval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.rval = rv;

              goto end;
            }

            else if (sf_str_eq_rCp(p, "<"))
            {
              expr_t pres_res = res;

              res.type = EXPR_CONDITIONAL_LT;
              res.v.expr_conditional.lval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.lval = pres_res;
              res.v.expr_conditional.rval = NULL;

              expr_t rv = sf_ast_exprgen(arr + i + 1, len - i - 1);
              res.v.expr_conditional.rval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.rval = rv;

              goto end;
            }

            else if (sf_str_eq_rCp(p, ">="))
            {
              expr_t pres_res = res;

              res.type = EXPR_CONDITIONAL_GTEQ;
              res.v.expr_conditional.lval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.lval = pres_res;
              res.v.expr_conditional.rval = NULL;

              expr_t rv = sf_ast_exprgen(arr + i + 1, len - i - 1);
              res.v.expr_conditional.rval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.rval = rv;

              goto end;
            }

            else if (sf_str_eq_rCp(p, "<="))
            {
              expr_t pres_res = res;

              res.type = EXPR_CONDITIONAL_LTEQ;
              res.v.expr_conditional.lval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.lval = pres_res;
              res.v.expr_conditional.rval = NULL;

              expr_t rv = sf_ast_exprgen(arr + i + 1, len - i - 1);
              res.v.expr_conditional.rval = sfmalloc(sizeof(expr_t));
              *res.v.expr_conditional.rval = rv;

              goto end;
            }

            else*/
            if (sf_str_eq_rCp (p, "="))
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

            else if (sf_str_eq_rCp (p, "{"))
              {
                if (res.type != -1)
                  {
                    // Possibly a syntax error
                    // TODO
                  }

                int comma_count = 0;
                int gb = 0;
                int i_jmp_idx = i;

                for (size_t j = i + 1; j < len; j++)
                  {
                    tok_t *d = &arr[j];

                    if (d->type == TOK_OPERATOR)
                      {
                        if (*SFCPTR_TOSTR (d->v.t_op.v) == '}' && !gb)
                          {
                            i_jmp_idx = j;
                            break;
                          }

                        if (*SFCPTR_TOSTR (d->v.t_op.v) == ',' && !gb)
                          comma_count++;

                        if (sf_str_inStr ("([{", d->v.t_op.v))
                          gb++;

                        else if (sf_str_inStr (")]}", d->v.t_op.v))
                          gb--;
                      }
                  }

                /*
                 * Number of key value pairs are either
                 * equal to comma_count+1 or comma_count
                 * for those who add a religious ',' at the end of
                 * the last key-value pair.
                 * In any case, comma_count+1 is a good measure.
                 */

                res.type = EXPR_MAP;
                expr_t *ks = sfmalloc ((comma_count + 1) * sizeof (*ks)),
                       *vls = sfmalloc ((comma_count + 1) * sizeof (*vls));

                tok_t *buf = sfmalloc ((i_jmp_idx - i) * sizeof (*buf));

                gb = 0;
                int buf_len = 0;
                int kvc = 0;

                for (size_t j = i + 1; j < i_jmp_idx; j++)
                  {
                    tok_t *d = &arr[j];

                    if (d->type == TOK_NEWLINE || d->type == TOK_SPACE)
                      continue;

                    if (d->type == TOK_OPERATOR)
                      {
                        char *ops = SFCPTR_TOSTR (d->v.t_op.v);

                        if (*ops == ':' && !gb)
                          {
                            ks[kvc] = sf_ast_exprgen (buf, buf_len);
                            buf_len = 0;
                            continue;
                          }
                        else if (*ops == ',' && !gb)
                          {
                            vls[kvc++] = sf_ast_exprgen (buf, buf_len);
                            buf_len = 0;
                            continue;
                          }

                        if (sf_str_inStr ("([{", d->v.t_op.v))
                          gb++;

                        else if (sf_str_inStr (")]}", d->v.t_op.v))
                          gb--;
                      }

                    buf[buf_len++] = *d;
                  }

                if (buf_len)
                  {
                    vls[kvc++] = sf_ast_exprgen (buf, buf_len);
                    buf_len = 0;
                  }

                res.v.e_map.count = kvc;
                res.v.e_map.keys = ks;
                res.v.e_map.vals = vls;

                i = i_jmp_idx;
                sffree (buf);

                goto l_end;
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

                    // sf_tokenizer_print (d);

                    if (d.type == TOK_OPERATOR)
                      {
                        sf_charptr p = d.v.t_op.v;

                        // printf ("%s %d\n", SFCPTR_TOSTR (p), gb);

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
                __sfapostfix_tree* atree = sfmalloc(
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

                // if (all_constants)
                //   {
                //     double arith_res
                //         = sf_arith_eval_tree (res.v.e_arith.tree).v.dres.v;
                //
                //     res.type = EXPR_CONSTANT;
                //     res.v.e_const.type = CONST_FLOAT;
                //     res.v.e_const.v.c_float.v = arith_res;
                //   }

                sffree (atree);
                goto end;
              }

            else if (sf_str_inStr (".", p))
              {
                expr_t res_pres = res;
                res.type = EXPR_MEM_ACCESS;

                expr_t *par = sfmalloc (sizeof (*par));
                *par = res_pres;

                // printf ("%d\n", par->type);

                res.v.mem_access.parent = par;

                expr_t *vl = sfmalloc (sizeof (*vl));
                // *vl = sf_ast_exprgen (arr + i + 1, len - i - 1);
                *vl = sf_ast_exprgen (arr + i + 1,
                                      1); /* get only the next token because
                                             that is the member's name */

                res.v.mem_access.val = vl;

                // goto end;
                i++;
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
_sf_stmt_whileblockgen (tok_t *arr, size_t idx, size_t *jptr)
{
  // function definition is similar to that of `for` block
  stmt_t res;
  res.type = STMT_WHILE_BLOCK;

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

  res.v.blk_while.cond = sfmalloc (sizeof (expr_t));

  // for (int i = 0; i < cei - idx - 1; i++)
  //   {
  //     sf_tokenizer_print (arr[idx + 1 + i]);
  //   }

  *res.v.blk_while.cond = sf_ast_exprgen (arr + idx + 1, cei - idx - 1);

  tok_t *body_end
      = _sf_stmt_getblkend (arr + cei, _sf_stmt_gettbsp (arr, idx));

  tok_t pres_end = *body_end;
  *body_end = (tok_t){
    .type = TOK_EOF,
  }; // end file at end of block (temporarily)

  size_t sts = 0; // statement tree size
  stmt_t *stree = sf_ast_stmtgen (arr + cei, &sts);

  *body_end = pres_end;
  res.v.blk_while.body = stree;
  res.v.blk_while.body_count = sts;

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

stmt_t
_sf_stmt_classdeclgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;

  res.type = STMT_CLASS_DECL;
  res.v.class_decl.body = NULL;
  res.v.class_decl.body_count = 0;
  res.v.class_decl.name = NULL;

  size_t name_end_idx = idx;
  int gb = 0;

  for (size_t i = idx + 1; arr[i].type != TOK_EOF; i++)
    {
      tok_t d = arr[i]; // TODO: could be optimized through pointers

      if (d.type == TOK_OPERATOR)
        {
          sf_charptr q = d.v.t_op.v;

          if (sf_str_inStr ("([{", q))
            gb++;

          else if (sf_str_inStr (")]}", q))
            gb--;
        }

      if (d.type == TOK_NEWLINE && !gb)
        {
          name_end_idx = i;
          break;
        }
    }

  assert (name_end_idx != idx);

  res.v.class_decl.name = sfmalloc (sizeof (expr_t));
  *res.v.class_decl.name
      = sf_ast_exprgen (arr + idx + 1, name_end_idx - idx - 1);

  tok_t *body_end = _sf_stmt_getblkend (arr + name_end_idx + 1,
                                        _sf_stmt_gettbsp (arr, idx));

  tok_t pres_end = *body_end;
  *body_end = (tok_t){
    .type = TOK_EOF,
  };

  size_t sts = 0;
  stmt_t *stree = sf_ast_stmtgen (arr + name_end_idx + 1, &sts);

  *body_end = pres_end;
  res.v.class_decl.body = stree;
  res.v.class_decl.body_count = sts;

  if (jptr != NULL)
    {
      *jptr = (body_end - arr) - 1;
    }

  return res;
}

stmt_t
_sf_stmt_importgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;
  res.type = STMT_IMPORT;
  res.v.stmt_import.alias = NULL;
  res.v.stmt_import.path = NULL;

  tok_t *nt = arr + idx + 1;
  assert (nt->type == TOK_STRING);

  res.v.stmt_import.path = sf_str_new_fromStr (SFCPTR_TOSTR (nt->v.t_str.v));
  nt++;

  if (nt->type == TOK_IDENTIFIER && sf_str_eq_rCp (nt->v.t_str.v, "as"))
    {
      nt++;

      assert (nt->type == TOK_IDENTIFIER);

      res.v.stmt_import.alias
          = sf_str_new_fromStr (SFCPTR_TOSTR (nt->v.t_str.v));
    }

  (*jptr) += nt - arr;
  return res;
}

stmt_t
_sf_stmt_returngen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;
  res.type = STMT_RETURN;
  res.v.stmt_return.val = NULL;

  size_t end_idx = idx;
  int gb = 0;

  for (size_t i = idx + 1; arr[i].type != TOK_EOF; i++)
    {
      tok_t *c = &arr[i];

      if (c->type == TOK_OPERATOR)
        {
          sf_charptr q = c->v.t_op.v;

          if (sf_str_inStr ("([{", q))
            gb++;

          else if (sf_str_inStr (")]}", q))
            gb--;
        }

      if (c->type == TOK_NEWLINE && !gb)
        {
          end_idx = i;
          break;
        }
    }

  if (end_idx == idx)
    {
      /*
        empty return
        return None
      */
      expr_t *none_Expr = sfmalloc (sizeof (*none_Expr));
      none_Expr->type = EXPR_CONSTANT;
      none_Expr->v.e_const.type = CONST_NONE;

      res.v.stmt_return.val = none_Expr;
      (*jptr)++;
    }

  else
    {
      expr_t *r = sfmalloc (sizeof (*r));
      *r = sf_ast_exprgen (arr + idx + 1, end_idx - idx);

      res.v.stmt_return.val = r;
      (*jptr) = end_idx;
    }

  return res;
}

stmt_t
_sf_stmt_withblockgen (tok_t *arr, size_t idx, size_t *jptr)
{
  stmt_t res;
  res.type = STMT_WITH_BLOCK;
  res.v.blk_with.val = NULL;
  res.v.blk_with.alias = NULL;
  res.v.blk_with.body = NULL;
  res.v.blk_with.body_count = 0;

  int gb = 0;
  size_t vei = idx;

  for (size_t i = idx + 1; arr[i].type != TOK_EOF; i++)
    {
      tok_t *t = &arr[i];

      if (t->type == TOK_OPERATOR)
        {
          if (sf_str_inStr ("({[", t->v.t_op.v))
            gb++;
          else if (sf_str_inStr (")}]", t->v.t_op.v))
            gb--;
        }

      if (t->type == TOK_NEWLINE && !gb)
        {
          vei = i;
          break;
        }

      if (t->type == TOK_IDENTIFIER && !gb
          && sf_str_eq_rCp (t->v.t_ident.v, "as"))
        {
          vei = i;
          break;
        }
    }

  assert (vei != idx);

  res.v.blk_with.val = sfmalloc (sizeof (expr_t));
  *res.v.blk_with.val = sf_ast_exprgen (arr + idx + 1, vei - idx - 1);

  if (arr[vei].type == TOK_IDENTIFIER)
    {
      /* has alias */
      size_t eli = vei;
      gb = 0;

      for (size_t j = vei + 1; arr[j].type != TOK_EOF; j++)
        {
          tok_t *t = &arr[j];

          if (t->type == TOK_OPERATOR)
            {
              if (sf_str_inStr ("({[", t->v.t_op.v))
                gb++;
              else if (sf_str_inStr (")}]", t->v.t_op.v))
                gb--;
            }

          if (t->type == TOK_NEWLINE && !gb)
            {
              eli = j;
              break;
            }
        }

      assert (eli != vei);
      res.v.blk_with.alias = sfmalloc (sizeof (expr_t));
      *res.v.blk_with.alias = sf_ast_exprgen (arr + vei + 1, eli - vei - 1);

      vei = eli;
    }

  tok_t *body_end
      = _sf_stmt_getblkend (arr + vei + 1, _sf_stmt_gettbsp (arr, idx));

  tok_t pres_end = *body_end;
  *body_end = (tok_t){
    .type = TOK_EOF,
  };

  size_t sts = 0;
  stmt_t *stree = sf_ast_stmtgen (arr + vei + 1, &sts);

  *body_end = pres_end;
  res.v.blk_with.body = stree;
  res.v.blk_with.body_count = sts;

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

    case OBJ_MAP:
      {
        sf_map_free (p->v.o_map.v);
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

    case OBJ_CLASS:
      {
        sf_class_free (p->v.o_class.val);
      }
      break;

    case OBJ_CLASSOBJ:
      {
        class_t *ct = p->v.o_cobj.val;

        llnode_t *kln = sf_mod_getVar (ct->mod, "_kill");
        int drop_f = ct->meta.kill_fun_called;

        if (kln != NULL)
          {
            sf_ll_set_meta_refcount (kln, kln->meta.ref_count + 1);
            obj_t *kv = (obj_t *)kln->val;

            if (kv->type == OBJ_FUN && !drop_f)
              {
                ct->meta.kill_fun_called = 1;
                fun_t *f = kv->v.o_fun.f;

                assert (f->argc == 1 && "_kill() expects 1 argument.");
                mod_t *kmod = sf_mod_new (MOD_TYPE_FUNC, NULL);

                kmod->body = f->mod->body;
                kmod->body_len = f->mod->body_len;

                // obj_t *kp = sf_ast_objnew (OBJ_CLASSOBJ);
                // kp->v.o_cobj.val = ct;

                // printf ("%d\n", (*obj)->meta.mem_ref->meta.ref_count);
                sf_ll_set_meta_refcount ((*obj)->meta.mem_ref, 1);

                sf_mod_addVar (kmod, "self", (*obj)->meta.mem_ref);
                kmod->parent = f->mod->parent;

                sf_parser_exec (kmod);
                sf_mod_free (kmod);

                // ! CAUTION
                // Manually reset flags to prevent deallocation again
                (*obj)->meta.mem_ref->meta.ref_count = 0;
              }

            sf_ll_set_meta_refcount (kln, kln->meta.ref_count - 1);
          }

        for (size_t i = 0; i < ct->il_c; i++)
          {
            sf_ll_set_meta_refcount (ct->inh_list[i], 0);
          }

        if (!drop_f)
          sf_class_free (p->v.o_cobj.val);
      }
      break;

    default:
      break;
    }

  if (p->meta.pa_size)
    {
      for (size_t i = 0; i < p->meta.pa_size; i++)
        {
          llnode_t *t = p->meta.passargs[i]->meta.mem_ref;
          sf_ll_set_meta_refcount (t, t->meta.ref_count - 1);
        }

      sffree (p->meta.passargs);
      p->meta.passargs = NULL;
      p->meta.pa_size = 0;
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

    case EXPR_MEM_ACCESS:
      {
        printf ("mem_access\nparent\n");
        sf_ast_exprprint (*e.v.mem_access.parent);

        printf ("val\n");
        sf_ast_exprprint (*e.v.mem_access.val);
      }
      break;

    case EXPR_THIS:
      {
        printf ("expr_this\n");
      }
      break;

    case EXPR_MAP:
      {
        printf ("expr_map\n");

        for (size_t i = 0; i < e.v.e_map.count; i++)
          {
            printf ("key %d\n", i);
            sf_ast_exprprint (e.v.e_map.keys[i]);

            printf ("val %d\n", i);
            sf_ast_exprprint (e.v.e_map.vals[i]);
          }
      }
      break;

    case EXPR_OR:
      {
        printf ("expr_or\nlhs\n");
        sf_ast_exprprint (*e.v.e_or.lhs);
        printf ("rhs\n");
        sf_ast_exprprint (*e.v.e_or.rhs);
      }
      break;

    case EXPR_AND:
      {
        printf ("expr_and\nlhs\n");
        sf_ast_exprprint (*e.v.e_and.lhs);
        printf ("rhs\n");
        sf_ast_exprprint (*e.v.e_and.rhs);
      }
      break;

    case EXPR_NOT:
      {
        printf ("expr_not\n");
        sf_ast_exprprint (*e.v.e_not.v);
      }
      break;

    case EXPR_WHERE:
      {
        printf ("expr_where\nprev_expr: ");
        sf_ast_exprprint (*e.v.e_where.prev_expr);
        printf ("variables (%d)\n", e.v.e_where.vsize);

        for (size_t i = 0; i < e.v.e_where.vsize; i++)
          {
            printf ("%s = ", SFCPTR_TOSTR (e.v.e_where.vnames[i]));
            sf_ast_exprprint (e.v.e_where.vvals[i]);
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

        printf ("endif----\n");
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

    case STMT_WHILE_BLOCK:
      {
        printf ("while_block\n");

        printf ("condition\n");
        sf_ast_exprprint (*s.v.blk_while.cond);

        printf ("body: %d\n", s.v.blk_while.body_count);
        for (size_t i = 0; i < s.v.blk_while.body_count; i++)
          {
            printf ("(%d)", i);
            sf_ast_stmtprint (s.v.blk_while.body[i]);
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

    case STMT_CLASS_DECL:
      {
        printf ("class_decl\n");

        printf ("name\n");
        sf_ast_exprprint (*s.v.class_decl.name);

        printf ("body (%d)\n", s.v.class_decl.body_count);

        for (size_t i = 0; i < s.v.class_decl.body_count; i++)
          {
            printf ("[%d] ", i);
            sf_ast_stmtprint (s.v.class_decl.body[i]);
          }
      }
      break;

    case STMT_IMPORT:
      {
        printf ("import\n"
                "path: \'%s\'\n"
                "alias: %s\n",
                s.v.stmt_import.path, s.v.stmt_import.alias);
      }
      break;

    case STMT_RETURN:
      {
        printf ("return: ");
        sf_ast_exprprint (*s.v.stmt_return.val);
      }
      break;

    case STMT_WITH_BLOCK:
      {
        printf ("with:\nval: ");
        sf_ast_exprprint (*s.v.blk_with.val);

        printf ("alias: ");
        if (s.v.blk_with.alias)
          sf_ast_exprprint (*s.v.blk_with.alias);
        else
          printf ("null\n");

        printf ("body (%d): \n", s.v.blk_with.body_count);
        for (size_t i = 0; i < s.v.blk_with.body_count; i++)
          {
            sf_ast_stmtprint (s.v.blk_with.body[i]);
          }
      }
      break;

    default:
      printf ("unknown statement %d\n", s.type);
      break;
    }
}
