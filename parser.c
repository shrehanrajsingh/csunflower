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
            here;
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

    default:
      e_printf ("Unknown expression '%d' in eval_expr()\n", e->type);
      break;
    }

end:
  assert (r != NULL);
  return r;
}