
#include "arithmetic.h"
#include "ast.h"
#include "nativemethods.h"
#include "parser.h"
#include "sfmod.h"

SF_API int
sf_arith_get_precedence (char *op)
{
  struct
  {
    char *op;
    int prec;
  } ops[] = {
    { "+", 10 }, { "-", 10 }, { "*", 20 }, { "/", 20 }, { NULL, -1 },
  };

  for (size_t i = 0; ops[i].prec != -1; i++)
    {
      if (!strcmp (ops[i].op, op))
        return ops[i].prec;
    }

  return -1;
}

SF_API tree_t *
sf_arith_pft_to_tree (__sfapostfix_tree *t, int len)
{
  tree_t **stack = sfmalloc (len * sizeof (*stack)); /* this is an array */
  size_t sc = 0;

  for (int i = 0; i < len; i++)
    {
      struct _sfa_treetok_s *ctk = sfmalloc (sizeof (*ctk));

      *ctk = (struct _sfa_treetok_s){
        .is_op = t[i].is_op,
        .is_llnode = 0,
      };

      if (ctk->is_op)
        ctk->v.op = sfstrdup (t[i].v.op);
      else
        ctk->v.val = (void *)t[i].v.val;

      if (t[i].is_op)
        {
          //   printf ("op: %s\n", t[i].v.op);
          assert (sc > 1);

          tree_t *back1 = stack[--sc];
          tree_t *back2 = stack[--sc];

          tree_t *nt = sf_tree_new (ctk, back1, back2);

          stack[sc++] = nt;
        }

      else
        {
          //   sf_ast_exprprint (*t[i].v.val);
          stack[sc++] = sf_tree_new (ctk, NULL, NULL);
        }
    }

  //   printf ("%d\n", sc);
  assert (sc == 1);
  tree_t *v = *stack;

  sffree (stack);

  return v;
}

SF_API void
sf_arith_infix_to_postfix (__sfapostfix_tree **tref, size_t *s)
{
  size_t ac = *s;

  __sfapostfix_tree *res = sfmalloc (ac * sizeof (*res));
  size_t rc = 0;

  char **op_stack = sfmalloc (ac * sizeof (*op_stack));
  int osc = 0;

  for (size_t i = 0; i < ac; i++)
    {
      if ((*tref)[i].is_op)
        {
          if (!osc)
            {
              op_stack[osc++] = (*tref)[i].v.op;
            }
          else
            {
              while (osc
                     && sf_arith_get_precedence (op_stack[osc - 1])
                            >= sf_arith_get_precedence ((*tref)[i].v.op))
                {
                  res[rc++] = (__sfapostfix_tree){
                    .is_op = 1,
                    .v.op = op_stack[--osc],
                  };
                }

              op_stack[osc++] = (*tref)[i].v.op;
            }
        }
      else
        {
          res[rc++] = (*tref)[i];
        }
    }

  while (osc)
    {
      res[rc++] = (__sfapostfix_tree){
        .is_op = 1,
        .v.op = op_stack[--osc],
      };
    }

  for (size_t i = 0; i < rc; i++)
    (*tref)[i] = res[i];

  *s = rc;

  sffree (op_stack);
}

SF_API __sfaarith_res
sf_arith_eval_tree (mod_t *m, tree_t *t)
{
  struct _sfa_treetok_s *c = t->val;
  __sfaarith_res res;

  // default
  res.type = SF_ARITH_RES_DOUBLE;
  res.v.dres.v = 0;

  if (c->is_op)
    {
      char *op = c->v.op;

      switch (*op)
        {
        case '+':
          {
            __sfaarith_res lres = sf_arith_eval_tree (m, t->right);
            __sfaarith_res rres = sf_arith_eval_tree (m, t->left);

            if (lres.type == rres.type && lres.type == SF_ARITH_RES_DOUBLE)
              {
                res.type = SF_ARITH_RES_DOUBLE;
                res.v.dres.v = lres.v.dres.v + rres.v.dres.v;
              }

            else
              {
                if (lres.type == SF_ARITH_RES_OBJ
                    && lres.v.ores.v->type == OBJ_CONST)
                  {
                    fun_t *f = sf_nm_get (SF_OP_OVRLD_PLUS,
                                          lres.v.ores.v->v.o_const.type);

                    if (f == NULL)
                      {
                        e_printf ("No suitable overload found for loperand in"
                                  "sf_arith_eval_tree()\n");

                        assert (0 && "Check logs for more information.");
                      }

                    mod_t *md = sf_mod_new (m->type, f->mod->parent);
                    assert (f->argc == 2);

                    sf_mod_addVar (md, f->args[0],
                                   lres.v.ores.v->meta.mem_ref != NULL
                                       ? lres.v.ores.v->meta.mem_ref
                                       : sf_ot_addobj (lres.v.ores.v));

                    assert (rres.v.ores.v->type == OBJ_CONST);
                    sf_mod_addVar (md, f->args[1],
                                   rres.v.ores.v->meta.mem_ref != NULL
                                       ? rres.v.ores.v->meta.mem_ref
                                       : sf_ot_addobj (rres.v.ores.v));

                    if (f->type == SF_FUN_NATIVE)
                      {
                        llnode_t *nl = f->native.routine (md);

                        res.type = SF_ARITH_RES_OBJ;
                        res.v.ores.v = (obj_t *)nl->val;
                      }
                    else
                      {
                        md->type = f->mod->type;
                        md->body = f->mod->body;
                        md->body_len = f->mod->body_len;

                        sf_parser_exec (md);

                        if (md->retv != NULL)
                          {
                            res.type = SF_ARITH_RES_OBJ;
                            res.v.ores.v = (obj_t *)md->retv->val;
                          }

                        else
                          {
                            obj_t *rv = sf_ast_objnew (OBJ_CONST);
                            rv->v.o_const.type = CONST_NONE;

                            res.type = SF_ARITH_RES_OBJ;
                            res.v.ores.v = rv;
                          }
                      }

                    md->body = NULL;
                    md->parent = NULL;
                    sf_mod_free (md);
                  }
              }
          }
          break;

        case '-':
          {
            __sfaarith_res lres = sf_arith_eval_tree (m, t->right);
            __sfaarith_res rres = sf_arith_eval_tree (m, t->left);

            if (lres.type == rres.type && lres.type == SF_ARITH_RES_DOUBLE)
              {
                res.type = SF_ARITH_RES_DOUBLE;
                res.v.dres.v = lres.v.dres.v - rres.v.dres.v;
              }
          }
          break;

        case '*':
          {
            __sfaarith_res lres = sf_arith_eval_tree (m, t->right);
            __sfaarith_res rres = sf_arith_eval_tree (m, t->left);

            if (lres.type == rres.type && lres.type == SF_ARITH_RES_DOUBLE)
              {
                res.type = SF_ARITH_RES_DOUBLE;
                res.v.dres.v = lres.v.dres.v * rres.v.dres.v;
              }
          }
          break;

        case '/':
          {
            __sfaarith_res lres = sf_arith_eval_tree (m, t->right);
            __sfaarith_res rres = sf_arith_eval_tree (m, t->left);

            if (lres.type == rres.type && lres.type == SF_ARITH_RES_DOUBLE)
              {
                res.type = SF_ARITH_RES_DOUBLE;
                res.v.dres.v = (float)lres.v.dres.v / rres.v.dres.v;
              }
          }
          break;

        case '%':
          {
            __sfaarith_res lres = sf_arith_eval_tree (m, t->right);
            __sfaarith_res rres = sf_arith_eval_tree (m, t->left);

            if (lres.type == rres.type && lres.type == SF_ARITH_RES_DOUBLE)
              {
                res.type = SF_ARITH_RES_DOUBLE;
                res.v.dres.v = (int)lres.v.dres.v % (int)rres.v.dres.v;
              }
          }
          break;

        default:
          e_printf ("Unsupported operator '%s' in sf_arith_eval_tree()\n", op);
          break;
        }
    }
  else
    {
      if (c->is_llnode)
        {
          obj_t *v = (obj_t *)c->node->val;
          // assert (v->type == OBJ_CONST); // ? for now
          //
          // switch (v->v.o_const.type)
          //   {
          //   case CONST_INT:
          //     return v->v.o_const.v.c_int.v;
          //     break;
          //
          //   case CONST_FLOAT:
          //     return v->v.o_const.v.c_float.v;
          //     break;
          //
          //   default:
          //     e_printf (
          //         "Unsupported operand of type '%d' in
          //         sf_arith_eval_tree()\n", v->v.o_const.type);
          //     break;
          //   }

          if (v->type == OBJ_CONST)
            //   {
            //     switch (v->v.o_const.type)
            //       {
            //       case CONST_INT:
            //         {
            //           res.type = SF_ARITH_RES_DOUBLE;
            //           res.v.dres.v = v->v.o_const.v.c_int.v;
            //         }
            //         break;
            //
            //       case CONST_FLOAT:
            //         {
            //           res.type = SF_ARITH_RES_DOUBLE;
            //           res.v.dres.v = v->v.o_const.v.c_float.v;
            //         }
            //         break;
            //
            //       default:
            //         goto fb;
            //       }
            //   }
            //
            // else
            {
            fb:;
              res.type = SF_ARITH_RES_OBJ;
              res.v.ores.v = v;
            }
        }
      else
        {
          assert (0 && "Illegal zone in arithmetic.");

          expr_t *v = c->v.val;
          assert (v->type == EXPR_CONSTANT);

          // switch (v->v.e_const.type)
          //   {
          //   case CONST_INT:
          //     return v->v.e_const.v.c_int.v;
          //     break;
          //
          //   case CONST_FLOAT:
          //     return v->v.e_const.v.c_float.v;
          //     break;
          //
          //   default:
          //     e_printf (
          //         "Unsupported operand of type '%d' in
          //         sf_arith_eval_tree()\n", v->v.e_const.type);
          //     break;
          //   }

          switch (v->v.e_const.type)
            {
            case CONST_INT:
              {
                res.type = SF_ARITH_RES_DOUBLE;
                res.v.dres.v = v->v.e_const.v.c_int.v;
              }
              break;

            case CONST_FLOAT:
              {
                res.type = SF_ARITH_RES_DOUBLE;
                res.v.dres.v = v->v.e_const.v.c_float.v;
              }
              break;

            default:
              {
                res.type = SF_ARITH_RES_OBJ;
                obj_t *o = sf_ast_objnew (v->v.e_const.type);
                o->v.o_const = v->v.e_const;
              }
              break;
            }
        }
    }

  return res;
}

SF_API tree_t *
sf_arith_tree_copyd (tree_t *t)
{
  tree_t *r = sf_tree_new (t->val, NULL, NULL);

  if (t->left)
    r->left = sf_arith_tree_copyd (t->left);

  if (t->right)
    r->right = sf_arith_tree_copyd (t->right);

  return r;
}