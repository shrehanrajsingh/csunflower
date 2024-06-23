#include "parser.h"

llnode_t *eval_expr (mod_t *, expr_t *);

SF_API parser_rt
sf_parser_exec (mod_t *mod)
{
  size_t i = 0;

  while (i < mod->body_len)
    {
      stmt_t t = mod->body[i];
      //   sf_ast_stmtprint (t);

      switch (t.type)
        {
        case STMT_VAR_DECL:
          {
            llnode_t *val_eval = eval_expr (mod, t.v.var_decl.val);

            switch (t.v.var_decl.name->type)
              {
              case EXPR_VAR:
                {
                  sf_charptr vname = t.v.var_decl.name->v.var.name;

                  sf_mod_addVar (mod, SFCPTR_TOSTR (vname), val_eval);
                }
                break;

              default:
                break;
              }
          }
          break;

        case STMT_FUN_CALL:
          {
            llnode_t *name = eval_expr (mod, t.v.fun_call.name);
            obj_t *fref = name->val;

            // printf ("[%s]\n", sf_parser_objRepr (mod, fref));
            // printf ("%s\n", t.v.fun_call.name->v.var.name);

            switch (fref->type)
              {
              case OBJ_FUN:
                {
                  fun_t *fn = fref->v.o_fun.f;
                  // mod_t *par_pres = fn->mod->parent;

                  for (size_t j = 0; j < t.v.fun_call.arg_count; j++)
                    {
                      llnode_t *on = eval_expr (mod, &t.v.fun_call.args[j]);

                      sf_mod_addVar (fn->mod, fn->args[j], on);
                    }

                  if (fn->type == SF_FUN_NATIVE)
                    {
                      llnode_t *ret = fn->native.routine (fn->mod);

                      sf_ot_removeobj (ret);
                    }

                  else
                    {
                    }
                }
                break;

              default:
                e_printf ("Cannot call object of type %d\n", fref->type);
                break;
              }
          }
          break;

        default:
          break;
        }

    loop_end:
      i++;
    }

end:
  return (parser_rt){ .i = -1 };
}

llnode_t *
eval_expr (mod_t *mod, expr_t *e)
{
  llnode_t *r = NULL;

  switch (e->type)
    {
    case EXPR_CONSTANT:
      {
        obj_t *v = sfmalloc (sizeof (*v));
        v->type = OBJ_CONST;
        v->v.o_const.type = e->v.e_const.type;

        switch (v->v.o_const.type)
          {
          case CONST_INT:
            {
              v->v.o_const.v.c_int.v = e->v.e_const.v.c_int.v;
            }
            break;

          case CONST_FLOAT:
            {
              v->v.o_const.v.c_float.v = e->v.e_const.v.c_float.v;
            }
            break;

          case CONST_STRING:
            {
              v->v.o_const.v.c_string.v = sf_str_new_fromStr (
                  SFCPTR_TOSTR (e->v.e_const.v.c_string.v));
            }
            break;

          case CONST_BOOL:
            {
              v->v.o_const.v.c_bool.v = e->v.e_const.v.c_bool.v;
            }
            break;

          default:
            e_printf ("Unknown constant type '%d' in eval_expr()\n",
                      v->v.o_const.type);
            break;
          }

        r = sf_ot_addobj (v);
      }
      break;

    case EXPR_VAR:
      {
        r = sf_mod_getVar (mod, e->v.var.name);
        // printf ("%d\n", ((obj_t *)r->val)->type);
      }
      break;

    default:
      e_printf ("Unknown expression '%d' in eval_expr()\n", e->type);
      break;
    }

end:
  assert (r != NULL);
  return r;
}

SF_API char *
sf_parser_objRepr (mod_t *mod, obj_t *obj)
{
  sf_charptr res;

  switch (obj->type)
    {
    case OBJ_CONST:
      {
        int ot = obj->v.o_const.type;

        switch (ot)
          {
          case CONST_INT:
            {
              int i = obj->v.o_const.v.c_int.v;
              int size = 1; /* for \0 */
              int isneg = 0;

              if (i < 0)
                {
                  size++; // '-'
                  isneg = 1;
                }

              do
                {
                  size++;
                  i /= 10;
                }
              while (i);

              char *r = sfmalloc (size * sizeof (*r));
              sprintf (r, "%d", obj->v.o_const.v.c_int.v);

              res = sf_str_new_fromStr (r);

              sffree (r);
            }
            break;

          case CONST_FLOAT:
            {
              char *r = sfmalloc (32 * sizeof (*r));
              sprintf (r, "%d", obj->v.o_const.v.c_float.v);

              res = sf_str_new_fromStr (r);

              sffree (r);
            }
            break;

          case CONST_BOOL:
            {
              if (obj->v.o_const.v.c_bool.v)
                res = sf_str_new_fromStr ("True");

              else
                res = sf_str_new_fromStr ("False");
            }
            break;

          case CONST_STRING:
            {
              res = sf_str_new_fromStr (
                  SFCPTR_TOSTR (obj->v.o_const.v.c_string.v));
            }
            break;

          default:
            break;
          }
      }
      break;

    case OBJ_FUN:
      {
        char *r
            = sfmalloc ((16 + strlen (obj->v.o_fun.f->name)) * sizeof (char));

        sprintf (r, "<function '%s'>", obj->v.o_fun.f->name);

        res = sf_str_new_fromStr (r);

        sffree (r);
      }
      break;

    default:
      e_printf ("Unknown type '%d' in sf_parser_objRepr()\n", obj->type);
      break;
    }

  return SFCPTR_TOSTR (res);
}