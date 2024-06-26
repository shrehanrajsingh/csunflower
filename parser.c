#include "parser.h"

llnode_t *eval_expr (mod_t *, expr_t *);
llnode_t *_sf_fcall (mod_t *, expr_t *);

SF_API parser_rt
sf_parser_exec (mod_t *mod)
{
  size_t i = 0;

  while (i < mod->body_len)
    {
      stmt_t t = mod->body[i];
      // sf_ast_stmtprint (t);

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
            expr_t mke = (expr_t){ .type = EXPR_FUN_CALL,
                                   .v.fun_call = {
                                       .arg_count = t.v.fun_call.arg_count,
                                       .args = t.v.fun_call.args,
                                       .name = t.v.fun_call.name,
                                   } };

            llnode_t *r = _sf_fcall (mod, &mke);

            assert (r != NULL);
            sf_ll_set_meta_refcount (r, r->meta.ref_count - 1);
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

    case EXPR_ARRAY:
      {
        array_t *t = sf_array_new ();
        t->len = e->v.e_array.val_count;
        t->vals = sfmalloc (t->len * sizeof (*t->vals));

        for (size_t j = 0; j < t->len; j++)
          {
            // printf ("%d\n", e->v.e_array.vals[j].type);
            t->vals[j] = eval_expr (mod, (expr_t *)&e->v.e_array.vals[j]);

            sf_ll_set_meta_refcount (t->vals[j],
                                     t->vals[j]->meta.ref_count + 1);
          }

        array_t *rt = sf_array_add (t);

        // printf ("[%d]\n", rt - *sf_array_getStack ());

        obj_t *o = sfmalloc (sizeof (*o));
        o->type = OBJ_ARRAY;
        o->v.o_array.v = rt;

        r = sf_ot_addobj (o);
      }
      break;

    case EXPR_IDX_ACCESS:
      {
        obj_t *name = (obj_t *)eval_expr (mod, e->v.e_idx_access.name)->val;
        obj_t *val = (obj_t *)eval_expr (mod, e->v.e_idx_access.val)->val;

        // printf ("%d %d\n", name->type, val->type);

        switch (name->type)
          {
          case OBJ_ARRAY:
            {
              array_t *t = name->v.o_array.v;

              switch (val->type)
                {
                case OBJ_CONST:
                  {
                    switch (val->v.o_const.type)
                      {
                      case CONST_INT:
                        {
                          int p = val->v.o_const.v.c_int.v;

                          while (p < 0)
                            p += t->len;
                          p %= t->len;

                          r = t->vals[p];
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
            }
            break;

          default:
            break;
          }
      }
      break;

    case EXPR_TOSTEPTYPE:
      {
        llnode_t *lv = eval_expr (mod, e->v.to_step.lval);
        llnode_t *rv = eval_expr (mod, e->v.to_step.rval);

        sf_ll_set_meta_refcount (lv, lv->meta.ref_count + 1);
        sf_ll_set_meta_refcount (rv, rv->meta.ref_count + 1);

        llnode_t *stp = NULL;
        int l_val, r_val, step_c = 1;

        llnode_t *tp = NULL;
        char tp_val[2] = { '[', ')' }; // default mode

        if (e->v.to_step.e_step != NULL)
          {
            stp = eval_expr (mod, e->v.to_step.e_step);

            sf_ll_set_meta_refcount (stp, stp->meta.ref_count + 1);
            obj_t *ob_o = (obj_t *)stp->val;

            assert (ob_o->type == OBJ_CONST
                    && ob_o->v.o_const.type == CONST_INT);

            step_c = ob_o->v.o_const.v.c_int.v;
          }

        if (e->v.to_step.e_type != NULL)
          {
            tp = eval_expr (mod, e->v.to_step.e_type);

            sf_ll_set_meta_refcount (tp, tp->meta.ref_count + 1);
            obj_t *tp_o = (obj_t *)tp->val;

            assert (tp_o->type == OBJ_CONST
                    && tp_o->v.o_const.type == CONST_STRING);

            char *p = SFCPTR_TOSTR (tp_o->v.o_const.v.c_string.v);

            for (size_t k = 0; p[k] != '\0'; k++)
              {
                if (p[k] == '[' || p[k] == '(')
                  tp_val[0] = p[k];

                if (p[k] == ']' || p[k] == ')')
                  tp_val[1] = p[k];
              }
          }

        obj_t *lv_o = (obj_t *)lv->val;
        obj_t *rv_o = (obj_t *)rv->val;

        assert (lv_o->type == OBJ_CONST && lv_o->v.o_const.type == CONST_INT);
        assert (rv_o->type == OBJ_CONST && rv_o->v.o_const.type == CONST_INT);

        l_val = lv_o->v.o_const.v.c_int.v;
        r_val = rv_o->v.o_const.v.c_int.v;

        array_t *narr = sf_array_new ();

        for (int i = l_val + (tp_val[0] != '[');
             i < r_val + (tp_val[1] == ']'); i += step_c)
          {
            obj_t *v = sfmalloc (sizeof (*v));
            v->type = OBJ_CONST;
            v->v.o_const.type = CONST_INT;
            v->v.o_const.v.c_int.v = i;

            sf_array_pushVal (narr, sf_ot_addobj (v));
          }

        obj_t *res = sfmalloc (sizeof (*res));
        res->type = OBJ_ARRAY;
        res->v.o_array.v = narr;

        r = sf_ot_addobj (res);

        sf_ll_set_meta_refcount (lv, lv->meta.ref_count - 1);
        sf_ll_set_meta_refcount (rv, rv->meta.ref_count - 1);

        if (stp != NULL)
          sf_ll_set_meta_refcount (stp, stp->meta.ref_count - 1);

        if (tp != NULL)
          sf_ll_set_meta_refcount (tp, tp->meta.ref_count - 1);
      }
      break;

    case EXPR_FUN_CALL:
      {
        r = _sf_fcall (mod, e);
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

llnode_t *
_sf_fcall (mod_t *mod, expr_t *e)
{
  llnode_t *name = eval_expr (mod, e->v.fun_call.name);
  obj_t *fref = name->val;

  // printf ("[%s]\n", sf_parser_objRepr (mod, fref));
  // printf ("%s\n", e->v.fun_call.name->v.var.name);

  llnode_t *res = NULL;

  switch (fref->type)
    {
    case OBJ_FUN:
      {
        fun_t *fn = fref->v.o_fun.f;
        // mod_t *par_pres = fn->mod->parent;

        for (size_t j = 0; j < e->v.fun_call.arg_count; j++)
          {
            llnode_t *on = eval_expr (mod, &e->v.fun_call.args[j]);

            sf_mod_addVar (fn->mod, fn->args[j], on);
          }

        if (fn->type == SF_FUN_NATIVE)
          {
            res = fn->native.routine (fn->mod);
          }

        else
          {
            // TODO
          }
      }
      break;

    default:
      e_printf ("Cannot call object of type %d\n", fref->type);
      break;
    }

  return res;
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

    case OBJ_ARRAY:
      {
        array_t *t = obj->v.o_array.v;
        assert (t != NULL);

        res = sf_str_new_fromStr ("[");

        for (size_t i = 0; i < t->len; i++)
          {
            char *p;
            sf_str_push (
                &res, p = sf_parser_objRepr (mod, (obj_t *)(t->vals[i]->val)));

            sffree (p);

            if (i != t->len - 1)
              sf_str_push (&res, ", ");
          }

        sf_str_push (&res, "]");
      }
      break;

    default:
      e_printf ("Unknown type '%d' in sf_parser_objRepr()\n", obj->type);
      break;
    }

  return SFCPTR_TOSTR (res);
}