#include "parser.h"

llnode_t *eval_expr (mod_t *, expr_t *);
llnode_t *_sf_fcall (mod_t *, expr_t *);

void _sf_exec_block_for (mod_t *, int);
void _sf_exec_block_if (mod_t *, int *);

int _sf_obj_cmp (mod_t *, obj_t *, obj_t *);
int _sf_obj_isfalse (mod_t *, obj_t *);

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
            expr_t *mke_p = sfmalloc (sizeof (*mke_p));
            *mke_p = (expr_t){ .type = EXPR_FUN_CALL,
                               .v.fun_call = {
                                   .arg_count = t.v.fun_call.arg_count,
                                   .args = t.v.fun_call.args,
                                   .name = t.v.fun_call.name,
                               } };

            llnode_t *r = _sf_fcall (mod, mke_p);

            // assert (r != NULL);
            // printf ("%d\n", r->meta.ref_count);
            sffree (mke_p);

            if (r != NULL)
              sf_ll_set_meta_refcount (r, r->meta.ref_count - 1);
          }
          break;

        case STMT_FUN_DECL:
          {
            mod_t *fmod = sf_mod_new (MOD_TYPE_FUNC, NULL);

            fmod->body = t.v.fun_decl.body;
            fmod->body_len = t.v.fun_decl.body_count;

            char *fname = NULL;

            if (t.v.fun_decl.name && t.v.fun_decl.name->type == EXPR_VAR)
              {
                fname = sf_str_new_fromStr (
                    SFCPTR_TOSTR (t.v.fun_decl.name->v.var.name));
              }

            size_t argc = t.v.fun_decl.arg_count;
            char **args = sfmalloc (argc * sizeof (*args));

            for (size_t j = 0; j < argc; j++)
              {
                expr_t *ca = &t.v.fun_decl.args[j];

                switch (ca->type)
                  {
                  case EXPR_VAR:
                    {
                      args[j]
                          = sf_str_new_fromStr (SFCPTR_TOSTR (ca->v.var.name));

                      obj_t *ev = sfmalloc (sizeof (*ev));

                      ev->type = OBJ_CONST;
                      ev->v.o_const.type = CONST_INT;
                      ev->v.o_const.v.c_int.v = 0;

                      sf_mod_addVar (fmod, args[j], sf_ot_addobj (ev));
                    }
                    break;

                  default:
                    break;
                  }
              }

            fmod->parent = mod;

            if (fname != NULL)
              {
                fun_t *nf = sf_fun_new (fname, SF_FUN_CODED, fmod, NULL);

                nf->argc = argc;
                nf->args = args;

                nf = sf_fun_add (nf);

                obj_t *ov = sfmalloc (sizeof (*ov));
                ov->type = OBJ_FUN;
                ov->v.o_fun.f = nf;

                sf_mod_addVar (mod, fname, sf_ot_addobj (ov));
              }
            else
              {
                e_printf ("Function has no name at sf_parser_exec()\n");
              }

            sffree (fname);
          }
          break;

        case STMT_FOR_BLOCK:
          {
            _sf_exec_block_for (mod, i);
          }
          break;

        case STMT_IF_BLOCK:
          {
            _sf_exec_block_if (mod, &i);
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

    case EXPR_INCLAUSE:
      {
        llnode_t *lv = eval_expr (mod, e->v.in_clause.lval);
        llnode_t *rv = eval_expr (mod, e->v.in_clause.rval);

        sf_ll_set_meta_refcount (lv, lv->meta.ref_count + 1);
        sf_ll_set_meta_refcount (rv, rv->meta.ref_count + 1);

        obj_t *l_obj = (obj_t *)lv->val;
        obj_t *r_obj = (obj_t *)rv->val;

        int l_in_r = 0;

        switch (r_obj->type)
          {
          case OBJ_ARRAY:
            {
              array_t *rt = r_obj->v.o_array.v;

              for (size_t j = 0; j < rt->len; j++)
                {
                  obj_t *co = (obj_t *)rt->vals[j]->val;

                  if (l_in_r = _sf_obj_cmp (mod, l_obj, co))
                    break;
                }
            }
            break;

          case OBJ_CONST:
            {
              assert (
                  r_obj->v.o_const.type == CONST_STRING
                  && l_obj->v.o_const.type
                         == CONST_STRING); // the only iterable constant (yet)

              sf_charptr ls = l_obj->v.o_const.v.c_string.v;
              sf_charptr rs = r_obj->v.o_const.v.c_string.v;

              l_in_r = sf_str_inStr (rs, ls);
            }
            break;

          default:
            e_printf ("No `in` rule for rvalue of type '%d'\n", r_obj->type);
            break;
          }

        obj_t *res = sfmalloc (sizeof (*res));
        res->type = OBJ_CONST;
        res->v.o_const.type = CONST_BOOL;
        res->v.o_const.v.c_bool.v = l_in_r;

        r = sf_ot_addobj (res);

        sf_ll_set_meta_refcount (lv, lv->meta.ref_count - 1);
        sf_ll_set_meta_refcount (rv, rv->meta.ref_count - 1);
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

void
_sf_exec_block_for (mod_t *mod, int i)
{
  stmt_t *t = &mod->body[i];
  expr_t *cn = t->v.blk_for.cond;

  stmt_t *body_pres = mod->body;
  size_t bl_pres = mod->body_len;

  mod->body = t->v.blk_for.body;
  mod->body_len = t->v.blk_for.body_count;

  int got_break_signal = 0;

  switch (cn->type)
    {
    case EXPR_INCLAUSE:
      {
        expr_t *lv = cn->v.in_clause.lval;
        llnode_t *rnode = eval_expr (mod, cn->v.in_clause.rval);

        sf_ll_set_meta_refcount (rnode, rnode->meta.ref_count + 1);

        obj_t *robj = (obj_t *)rnode->val;

        switch (robj->type)
          {
          case OBJ_ARRAY:
            {
              array_t *rt = robj->v.o_array.v;

              for (size_t i = 0; i < rt->len; i++)
                {
                  if (lv->type == EXPR_VAR)
                    {
                      sf_charptr vname = lv->v.var.name;

                      sf_mod_addVar (mod, SFCPTR_TOSTR (vname), rt->vals[i]);
                    }
                  else
                    {
                      // TODO
                    }

                  sf_parser_exec (mod);
                  // TODO: check for signals like continue, break
                }
            }
            break;

          default:
            break;
          }

        sf_ll_set_meta_refcount (rnode, rnode->meta.ref_count - 1);
      }
      break;

    default:
      break;
    }

  mod->body = body_pres;
  mod->body_len = bl_pres;

  if (i + 1 < mod->body_len && !got_break_signal
      && mod->body[i + 1].type == STMT_ELSE_BLOCK)
    {
      body_pres = mod->body;
      bl_pres = mod->body_len;

      mod->body = body_pres[i + 1].v.blk_else.body;
      mod->body_len = body_pres[i + 1].v.blk_else.body_count;

      sf_parser_exec (mod);

      mod->body = body_pres;
      mod->body_len = bl_pres;
    }
}

int
_sf_obj_isfalse (mod_t *mod, obj_t *o)
{
  switch (o->type)
    {
    case OBJ_CONST:
      {
        switch (o->v.o_const.type)
          {
          case CONST_INT:
            return o->v.o_const.v.c_int.v == 0;
            break;

          case CONST_FLOAT:
            return o->v.o_const.v.c_float.v == 0.0;
            break;

          case CONST_BOOL:
            return o->v.o_const.v.c_bool.v == 0;
            break;

          case CONST_STRING:
            return sf_str_eq_rCp (o->v.o_const.v.c_string.v, "");
            break;

          default:
            break;
          }
      }
      break;

    case OBJ_ARRAY:
      {
        array_t *t = o->v.o_array.v;

        return t->len == 0;
      }
      break;

    case OBJ_FUN:
      goto end;
      break;

    default:
      break;
    }

end:
  return 0;
}

void
_sf_exec_block_if (mod_t *mod, int *ip)
{
  int i = *ip;
  stmt_t *t = &mod->body[i];

  llnode_t *o = eval_expr (mod, t->v.blk_if.cond);
  sf_ll_set_meta_refcount (o, o->meta.ref_count + 1);

  obj_t *cval = (obj_t *)o->val;

  if (_sf_obj_isfalse (mod, cval))
    {
      // Condition is false

      if (i < mod->body_len - 1)
        {
          (*ip)++;
          stmt_t *ntok = &mod->body[*ip];

          if (ntok->type == STMT_ELSE_BLOCK)
            {
              stmt_t *body_pres = mod->body;
              size_t blen_pres = mod->body_len;

              mod->body = ntok->v.blk_else.body;
              mod->body_len = ntok->v.blk_else.body_count;

              sf_parser_exec (mod);

              mod->body = body_pres;
              mod->body_len = blen_pres;
            }

          else if (ntok->type == STMT_ELSEIF_BLOCK)
            {
              int state = 0;
              do
                {
                  if (ntok->type != STMT_ELSEIF_BLOCK)
                    break;

                  llnode_t *elo = eval_expr (mod, ntok->v.blk_elseif.cond);
                  sf_ll_set_meta_refcount (elo, elo->meta.ref_count + 1);

                  obj_t *elo_obj = (obj_t *)elo->val;

                  if (!_sf_obj_isfalse (mod, elo_obj))
                    {
                      stmt_t *body_pres = mod->body;
                      size_t blen_pres = mod->body_len;

                      mod->body = ntok->v.blk_elseif.body;
                      mod->body_len = ntok->v.blk_elseif.body_count;

                      sf_parser_exec (mod);

                      mod->body = body_pres;
                      mod->body_len = blen_pres;

                      state = 1;
                      sf_ll_set_meta_refcount (elo, elo->meta.ref_count - 1);

                      while (*ip < mod->body_len
                             && (mod->body[*ip].type == STMT_ELSEIF_BLOCK
                                 || mod->body[*ip].type == STMT_ELSE_BLOCK))
                        {
                          (*ip)++;
                        }

                      break;
                    }

                  sf_ll_set_meta_refcount (elo, elo->meta.ref_count - 1);
                  (*ip)++;
                  ntok = &mod->body[*ip];
                }
              while (*ip < mod->body_len);

              if (!state && ntok->type == STMT_ELSE_BLOCK)
                {
                  // execute else (if a block exists)

                  stmt_t *body_pres = mod->body;
                  size_t blen_pres = mod->body_len;

                  mod->body = ntok->v.blk_else.body;
                  mod->body_len = ntok->v.blk_else.body_count;

                  sf_parser_exec (mod);

                  mod->body = body_pres;
                  mod->body_len = blen_pres;
                }
            }
        }
    }

  else
    {
      // Condition is true
      stmt_t *body_pres = mod->body;
      size_t blen_pres = mod->body_len;

      mod->body = t->v.blk_if.body;
      mod->body_len = t->v.blk_if.body_count;

      sf_parser_exec (mod);

      mod->body = body_pres;
      mod->body_len = blen_pres;

      if (i < mod->body_len - 1)
        {
          stmt_t *ntok = &mod->body[i + 1];

          if (ntok->type == STMT_ELSE_BLOCK)
            {
              *ip++;
            }

          else if (ntok->type == STMT_ELSEIF_BLOCK)
            {
              do
                {
                  *ip++;
                }
              while (*ip < mod->body_len
                     && mod->body[*ip].type == STMT_ELSEIF_BLOCK);

              if (*ip < mod->body_len
                  && mod->body[*ip].type == STMT_ELSE_BLOCK)
                *ip++;
            }
        }
    }

  sf_ll_set_meta_refcount (o, o->meta.ref_count - 1);
}

llnode_t *
_sf_fcall (mod_t *mod, expr_t *e)
{
  llnode_t *name = eval_expr (mod, e->v.fun_call.name);
  obj_t *fref = name->val;

  sf_ll_set_meta_refcount (name, name->meta.ref_count + 1);

  // printf ("[%s]\n", sf_parser_objRepr (mod, fref));
  // printf ("%s\n", e->v.fun_call.name->v.var.name);

  llnode_t *res = NULL;

  mod_t *nmod = sf_mod_new (MOD_TYPE_FUNC, NULL);

  switch (fref->type)
    {
    case OBJ_FUN:
      {
        fun_t *fn = fref->v.o_fun.f;
        // mod_t *par_pres = fn->mod->parent;

        nmod->type = fn->mod->type;
        nmod->body = fn->mod->body;
        nmod->body_len = fn->mod->body_len;

        assert (fn->argc == e->v.fun_call.arg_count);

        for (size_t j = 0; j < fn->argc; j++)
          {
            expr_t *ee = sfmalloc (sizeof (*ee));
            *ee = e->v.fun_call.args[j];

            llnode_t *on = eval_expr (mod, ee);

            sffree (ee);
            // printf ("%s\n", fn->args[j]);
            sf_mod_addVar (nmod, fn->args[j], on);
          }

        nmod->parent = fn->mod->parent;

        if (fn->type == SF_FUN_NATIVE)
          {
            res = fn->native.routine (nmod);
          }

        else
          {
            // printf ("%d\n", fn->mod->body_len);
            sf_parser_exec (nmod);

            if (nmod->retv != NULL)
              {
                res = nmod->retv;
              }
            else
              {
                obj_t *rv = sfmalloc (sizeof (*rv));
                rv->type = OBJ_CONST;
                rv->v.o_const.type = CONST_INT;
                rv->v.o_const.v.c_int.v = 0;

                res = sf_ot_addobj (rv);
              }
          }

        /*
         If return value is an argument (lvalue)
         and if a return reference is not counted
         then argument will lose it's object in the next step
         (acts as rvalue).
         This will return a free'd object if argument's value is not
         referenced by other symbols.
        */
        if (res != NULL)
          sf_ll_set_meta_refcount (res, res->meta.ref_count + 1);
      }
      break;

    default:
      e_printf ("Cannot call object of type %d\n", fref->type);
      break;
    }

  nmod->parent = NULL;
  nmod->body = NULL;
  nmod->body_len = 0;
  nmod->retv = NULL;

  sf_mod_free (nmod);

  sf_ll_set_meta_refcount (name, name->meta.ref_count - 1);

  // printf ("[%d]\n", res->meta.ref_count);

  return res;
}

int
_sf_obj_cmp (mod_t *mod, obj_t *o1, obj_t *o2)
{
  if (o1->type != o2->type)
    goto end;

  switch (o1->type)
    {
    case OBJ_ARRAY:
      {
        array_t *t1 = o1->v.o_array.v;
        array_t *t2 = o2->v.o_array.v;

        for (size_t i = 0; i < t1->len; i++)
          {
            if (!_sf_obj_cmp (mod, (obj_t *)t1->vals[i]->val,
                              (obj_t *)t2->vals[i]->val))
              goto end;
          }
      }
      break;

    case OBJ_CONST:
      {
        if (o1->v.o_const.type != o2->v.o_const.type)
          goto end;

        switch (o1->v.o_const.type)
          {
          case CONST_BOOL:
            return o1->v.o_const.v.c_bool.v == o2->v.o_const.v.c_bool.v;
            break;

          case CONST_FLOAT:
            return o1->v.o_const.v.c_float.v == o2->v.o_const.v.c_float.v;
            break;

          case CONST_INT:
            return o1->v.o_const.v.c_int.v == o2->v.o_const.v.c_int.v;
            break;

          case CONST_STRING:
            return sf_str_eq (o1->v.o_const.v.c_string.v,
                              o2->v.o_const.v.c_string.v);
            break;

          default:
            e_printf ("Invalid constant type '%d' at _sf_obj_cmp()\n",
                      o1->v.o_const.type);
            break;
          }
      }
      break;

    case OBJ_FUN:
      {
        fun_t *f1 = o1->v.o_fun.f;
        fun_t *f2 = o2->v.o_fun.f;

        if (f1->argc == f2->argc && f1->type == f2->type
            && !strcmp (f1->name, f2->name))
          {
            // change one parameter of f1 temporarily
            // if the same is changed in f2, then both
            // are alike

            int f1t_pres = f1->type;
            f1->type = -1;

            if (f2->type == -1)
              {
                f1->type = f1t_pres;
                return 1;
              }

            f1->type = f1t_pres;
            goto end;
          }
        else
          goto end;
      }
      break;

    default:
      e_printf ("Invalid object type '%d' at _sf_obj_cmp()\n", o1->type);
      break;
    }

end:
  return 0;
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
                res = sf_str_new_fromStr (SF_BOOL_TRUE_REPR);

              else
                res = sf_str_new_fromStr (SF_BOOL_FALSE_REPR);
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