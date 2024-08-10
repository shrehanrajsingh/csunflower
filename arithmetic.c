
#include "arithmetic.h"
#include "ast.h"

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

SF_API double
sf_arith_eval_tree (tree_t *t)
{
  struct _sfa_treetok_s *c = t->val;

  if (c->is_op)
    {
      char *op = c->v.op;

      switch (*op)
        {
        case '+':
          {
            return sf_arith_eval_tree (t->right)
                   + sf_arith_eval_tree (t->left);
          }
          break;

        case '-':
          {
            return sf_arith_eval_tree (t->right)
                   - sf_arith_eval_tree (t->left);
          }
          break;

        case '*':
          {
            return sf_arith_eval_tree (t->left)
                   * sf_arith_eval_tree (t->right);
          }
          break;

        case '/':
          {
            return sf_arith_eval_tree (t->right)
                   / sf_arith_eval_tree (t->left);
          }
          break;

        case '%':
          {
            return (int)sf_arith_eval_tree (t->right)
                   % (int)sf_arith_eval_tree (t->left);
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
          assert (v->type == OBJ_CONST); // ? for now

          switch (v->v.o_const.type)
            {
            case CONST_INT:
              return v->v.o_const.v.c_int.v;
              break;

            case CONST_FLOAT:
              return v->v.o_const.v.c_float.v;
              break;

            default:
              e_printf (
                  "Unsupported operand of type '%d' in sf_arith_eval_tree()\n",
                  v->v.o_const.type);
              break;
            }
        }
      else
        {
          expr_t *v = c->v.val;
          assert (v->type == EXPR_CONSTANT);

          switch (v->v.e_const.type)
            {
            case CONST_INT:
              return v->v.e_const.v.c_int.v;
              break;

            case CONST_FLOAT:
              return v->v.e_const.v.c_float.v;
              break;

            default:
              e_printf (
                  "Unsupported operand of type '%d' in sf_arith_eval_tree()\n",
                  v->v.e_const.type);
              break;
            }
        }
    }

  return 0;
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