#include "parser.h"
#include "nativemethods.h"

llnode_t *_sf_fcall (mod_t *, expr_t *);
llnode_t *_sf_ev_arith (mod_t *, expr_t *);
llnode_t *_sf_class_construct_noInit (mod_t *, obj_t *);
llnode_t *_sf_class_construct (mod_t *, obj_t *, expr_t *);

llnode_t *_sf_exec_compare (mod_t *, expr_t *);

void _sf_exec_block_for (mod_t *, int);
void _sf_exec_block_if (mod_t *, size_t *);
void _sf_exec_block_while (mod_t *, int);

int _sf_obj_cmp (mod_t *, obj_t *, obj_t *);
int _sf_obj_isfalse (mod_t *, obj_t *);

SF_API void
sf_parser_init (void)
{
  /* empty utility */
}

SF_API parser_rt
sf_parser_exec (mod_t *mod)
{
  size_t i = 0;

  while (i < mod->body_len)
    {
      const stmt_t t = mod->body[i];
      // sf_ast_stmtprint (t);

      if (mod->retv != NULL)
        goto end;

      switch (t.type)
        {
        case STMT_VAR_DECL:
          {
            llnode_t *val_eval = eval_expr (mod, t.v.var_decl.val);
            expr_t *tvn = t.v.var_decl.name;
            obj_t *vo = (obj_t *)val_eval->val;

            // if (vo->type != OBJ_FUN)
            //   {
            //   l2:
            //     if (vo->meta.pa_size)
            //       {
            //         obj_t *co = vo->meta.passargs[0];
            //         sf_ll_set_meta_refcount (co->meta.mem_ref,
            //                                  co->meta.mem_ref->meta.ref_count
            //                                      - 1);

            //         sffree (vo->meta.passargs);
            //         vo->meta.pa_size = 0;
            //         vo = co;

            //         goto l2;
            //       }
            //   }

            switch (tvn->type)
              {
              case EXPR_VAR:
                {
                  sf_charptr vname = tvn->v.var.name;

                  sf_mod_addVar (mod, SFCPTR_TOSTR (vname), val_eval);
                }
                break;

              case EXPR_MEM_ACCESS:
                {
                  llnode_t *pl = eval_expr (mod, tvn->v.mem_access.parent);
                  obj_t *pl_o = (obj_t *)pl->val;

                  // if (mod->parent->parent == NULL)
                  //   printf ("%s\n", sf_parser_objRepr (mod, pl_o));

                  switch (pl_o->type)
                    {
                    case OBJ_CLASSOBJ:
                      {
                        expr_t *vl = tvn->v.mem_access.val;
                        class_t *co = pl_o->v.o_cobj.val;

                        switch (vl->type)
                          {
                          case EXPR_VAR:
                            {
                              sf_mod_addVar (
                                  co->mod,
                                  (char *)SFCPTR_TOSTR (vl->v.var.name),
                                  val_eval);
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

                  obj_t *po = pl_o;

                  // l1:
                  //   if (po->meta.pa_size)
                  //     {
                  //       llnode_t *pol = po->meta.passargs[0]->meta.mem_ref;
                  //       sf_ll_set_meta_refcount (pol, pol->meta.ref_count -
                  //       1);

                  //       obj_t *pres_po = po->meta.passargs[0];
                  //       po->meta.pa_size = 0;
                  //       sffree (po->meta.passargs);
                  //       po->meta.passargs = NULL;

                  //       po = pres_po; /* pa_size is atmost 1 */
                  //       goto l1;
                  //     }
                }
                break;

              case EXPR_IDX_ACCESS:
                {
                  llnode_t *nam = eval_expr (mod, tvn->v.e_idx_access.name);
                  llnode_t *idx = eval_expr (mod, tvn->v.e_idx_access.val);

                  sf_ll_set_meta_refcount (nam, nam->meta.ref_count + 1);
                  sf_ll_set_meta_refcount (idx, idx->meta.ref_count + 1);

                  obj_t *o_nam = (obj_t *)nam->val;
                  obj_t *o_idx = (obj_t *)idx->val;

                  switch (o_nam->type)
                    {
                    case OBJ_ARRAY:
                      {
                        array_t *t = o_nam->v.o_array.v;

                        assert (o_idx->type == OBJ_CONST
                                && o_idx->v.o_const.type == CONST_INT);

                        int vidx = o_idx->v.o_const.v.c_int.v;

                        while (vidx < 0)
                          vidx += t->len;

                        if (vidx >= t->len)
                          {
                            e_printf ("array index out of bounds.\tLength: "
                                      "%d, accessing element %d\n",
                                      t->len, vidx);
                            exit (1);
                          }

                        llnode_t *prev_idx = t->vals[vidx];
                        sf_ll_set_meta_refcount (prev_idx,
                                                 prev_idx->meta.ref_count - 1);

                        sf_ll_set_meta_refcount (val_eval,
                                                 val_eval->meta.ref_count + 1);
                        t->vals[vidx] = val_eval;
                      }
                      break;

                    case OBJ_MAP:
                      {
                        map_t *m = o_nam->v.o_map.v;
                        assert (o_idx->type == OBJ_CONST
                                && o_idx->v.o_const.type == CONST_STRING);

                        sf_charptr p = o_idx->v.o_const.v.c_string.v;

                        llnode_t *prev_v = sf_map_getVal (m, p);

                        if (prev_v != NULL)
                          sf_ll_set_meta_refcount (prev_v,
                                                   prev_v->meta.ref_count - 1);

                        sf_ll_set_meta_refcount (val_eval,
                                                 val_eval->meta.ref_count + 1);
                        sf_map_addKeyVal (m, p, val_eval);
                      }
                      break;

                    default:
                      break;
                    }

                  sf_ll_set_meta_refcount (nam, nam->meta.ref_count - 1);
                  sf_ll_set_meta_refcount (idx, idx->meta.ref_count - 1);
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
            obj_t *ro = (obj_t *)r->val;

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

                      obj_t *ev = sf_ast_objnew (OBJ_CONST);

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

                obj_t *ov = sf_ast_objnew (OBJ_FUN);
                ov->v.o_fun.f = nf;

                if (mod->type == MOD_TYPE_CLASS)
                  {
                    ov->v.o_fun.uses_self = 1;

                    /*
                      selfarg will be used with class objects.
                    */
                    ov->v.o_fun.selfarg = NULL;
                  }

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

        case STMT_WHILE_BLOCK:
          {
            _sf_exec_block_while (mod, i);

            /**
             * Might need to change signature in _sf_exec_block_while
             * from `_sf_exec_block_while (mod_t *, int)` to
             * `_sf_exec_block_while (mod_t *, int *)` to track
             * else blocks.
             * (same with `for` block)
             */
          }
          break;

        case STMT_CLASS_DECL:
          {
            expr_t *nam = t.v.class_decl.name;
            char *cname = NULL;
            llnode_t **inhlst = NULL;
            size_t ilc = 0;

            if (nam->type == EXPR_FUN_CALL)
              {
                /* class inherits */

                assert (nam->v.fun_call.name->type == EXPR_VAR);
                cname = sfstrdup (
                    SFCPTR_TOSTR (nam->v.fun_call.name->v.var.name));

                expr_t *fargs = nam->v.fun_call.args;
                inhlst = sfmalloc ((ilc = nam->v.fun_call.arg_count)
                                   * sizeof (*inhlst));

                for (size_t j = 0; j < nam->v.fun_call.arg_count; j++)
                  {
                    llnode_t *t = eval_expr (mod, &fargs[j]);
                    inhlst[j] = t;
                  }
              }

            else if (nam->type == EXPR_VAR)
              {
                cname = sfstrdup (SFCPTR_TOSTR (nam->v.var.name));
              }

            assert (cname != NULL);

            mod_t *cmod = sf_mod_new (MOD_TYPE_CLASS, mod);
            cmod->body = t.v.class_decl.body;
            cmod->body_len = t.v.class_decl.body_count;

            /* for (size_t j = 0; j < cmod->body_len; j++)
              {
                sf_ast_stmtprint (cmod->body[j]);
              } */

            sf_parser_exec (cmod);

            class_t *nt = sf_class_new (cname, SF_CLASS_CODED);
            nt->mod = cmod;
            nt->inh_list = inhlst;
            nt->il_c = ilc;

            sf_class_add (nt);

            obj_t *nobj = sf_ast_objnew (OBJ_CLASS);
            nobj->v.o_class.val = nt;

            sf_mod_addVar (mod, cname, sf_ot_addobj (nobj));

            sffree (cname);
          }
          break;

        case STMT_IMPORT:
          {
            assert (t.v.stmt_import.path != NULL
                    && t.v.stmt_import.alias != NULL);

            char *fconts = sf_module_getfilecontents (t.v.stmt_import.path);

            assert (fconts != NULL && "File not found.\n");

            mod_t *nmd = sf_mod_new (MOD_TYPE_FILE, *sf_module_getparent ());

            tok_t *tk = sf_tokenizer_gen (fconts);
            size_t sptr = 0;
            stmt_t *st = sf_ast_stmtgen (tk, &sptr);

            nmd->body = st;
            nmd->body_len = sptr;

            sf_parser_exec (nmd);

            sfmodule_t *mt = sf_module_new (SF_MD_CODED, nmd);

            sf_module_set_path (mt, t.v.stmt_import.path);
            sf_module_set_alias (mt, t.v.stmt_import.alias);

            sf_module_add (mt);

            obj_t *v = sf_ast_objnew (OBJ_MODULE);
            v->v.o_mod.val = mt;

            sf_mod_addVar (mod, mt->alias, sf_ot_addobj (v));
          }
          break;

        case STMT_RETURN:
          {
            llnode_t *n = eval_expr (mod, t.v.stmt_return.val);
            // printf ("%d\n", ((obj_t *)n->val)->meta.pa_size);
            obj_t *no = (obj_t *)n->val;

            // if (no->type != OBJ_FUN)
            //   {
            //   l4:
            //     if (no->meta.pa_size)
            //       {
            //         obj_t *co = no->meta.passargs[0];
            //         sf_ll_set_meta_refcount (co->meta.mem_ref,
            //                                  co->meta.mem_ref->meta.ref_count
            //                                      - 1);

            //         sffree (no->meta.passargs);
            //         no->meta.pa_size = 0;
            //         no = co;

            //         goto l4;
            //       }
            //   }

            if (mod->retv != NULL)
              sf_ll_set_meta_refcount (mod->retv,
                                       mod->retv->meta.ref_count - 1);

            mod->retv = n;
            sf_ll_set_meta_refcount (mod->retv, mod->retv->meta.ref_count + 1);
            goto end;
          }
          break;

        case STMT_WITH_BLOCK:
          {
            llnode_t *val_ll = eval_expr (mod, t.v.blk_with.val);
            mod_t *wmod = sf_mod_new (mod->type, NULL);

            if (t.v.blk_with.alias == NULL)
              {
                sf_ll_set_meta_refcount (val_ll, val_ll->meta.ref_count + 1);
                wmod->wb_gvref = val_ll;
              }
            else
              {
                expr_t *ar = t.v.blk_with.alias;
                if (ar->type == EXPR_VAR)
                  {
                    sf_charptr nref = ar->v.var.name;

                    sf_mod_addVar (wmod, SFCPTR_TOSTR (nref), val_ll);
                  }
                else
                  assert (0 && "case not implemented.");
              }

            wmod->body = t.v.blk_with.body;
            wmod->body_len = t.v.blk_with.body_count;

            wmod->parent = mod;

            sf_parser_exec (wmod);

            if (t.v.blk_with.alias == NULL)
              {
                sf_ll_set_meta_refcount (val_ll, val_ll->meta.ref_count - 1);
              }

            if (wmod->retv != NULL)
              {
                mod->retv = wmod->retv;
              }

            // TODO: handle other signals

            wmod->parent = NULL;
            sf_mod_free (wmod);
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

SF_API llnode_t *
eval_expr (mod_t *mod, expr_t *e)
{
  llnode_t *r = NULL;

  switch (e->type)
    {
    case EXPR_CONSTANT:
      {
        obj_t *v = sf_ast_objnew (OBJ_CONST);
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
      }
      break;

    case EXPR_VAR_DECL:
      {
        size_t pres_sz = mod->body_len;
        stmt_t pres_fs = *mod->body;

        mod_t *par_pres = mod->parent;
        mod->parent = NULL;

        mod->body_len = 1;
        *mod->body = (stmt_t){ .type = STMT_VAR_DECL,
                               .v = { .var_decl = {
                                          .name = e->v.var_decl.name,
                                          .val = e->v.var_decl.val,
                                      } } };

        sf_parser_exec (mod);

        *mod->body = pres_fs;
        mod->body_len = pres_sz;

        mod->parent = par_pres;

        r = eval_expr (mod, e->v.var_decl.name);
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

        obj_t *o = sf_ast_objnew (OBJ_ARRAY);
        o->v.o_array.v = rt;

        r = sf_ot_addobj (o);
      }
      break;

    case EXPR_MAP:
      {
        map_t *t = sf_map_new ();

        for (size_t j = 0; j < e->v.e_map.count; j++)
          {
            llnode_t *kl = eval_expr (mod, &e->v.e_map.keys[j]);
            sf_ll_set_meta_refcount (kl, kl->meta.ref_count + 1);
            obj_t *kobj = (obj_t *)kl->val;

            if (kobj->type != OBJ_CONST
                || (kobj->v.o_const.type != CONST_STRING))
              {
                e_printf ("key must be a string\n");
                exit (1);
              }

            llnode_t *vl = eval_expr (mod, &e->v.e_map.vals[j]);

            // printf ("%s %s\n", sf_parser_objRepr (mod, kobj),
            //         sf_parser_objRepr (mod, (obj_t *)vl->val));

            sf_ll_set_meta_refcount (vl, vl->meta.ref_count + 1);
            sf_map_addKeyVal (t, kobj->v.o_const.v.c_string.v, vl);
            sf_ll_set_meta_refcount (kl, kl->meta.ref_count - 1);
          }

        obj_t *o = sf_ast_objnew (OBJ_MAP);
        o->v.o_map.v = sf_map_add (t);

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

          case OBJ_MAP:
            {
              map_t *m = name->v.o_map.v;

              assert (val->type == OBJ_CONST
                      && val->v.o_const.type == CONST_STRING);

              r = (llnode_t *)sf_map_getVal (m, val->v.o_const.v.c_string.v);
              assert (r != NULL && "map does not contain key.");
            }
            break;

          case OBJ_CONST:
            {
              switch (name->v.o_const.type)
                {
                case CONST_STRING:
                  {
                    switch (val->type)
                      {
                      case OBJ_CONST:
                        {
                          switch (val->v.o_const.type)
                            {
                            case CONST_INT:
                              {
                                int p = val->v.o_const.v.c_int.v;

                                assert (p > -1
                                        && p < strlen (SFCPTR_TOSTR (
                                               name->v.o_const.v.c_string.v)));

                                obj_t *ro = sf_ast_objnew (OBJ_CONST);
                                ro->v.o_const.type = CONST_STRING;
                                ro->v.o_const.v.c_string.v
                                    = sf_str_new_empty ();

                                sf_str_pushchr (
                                    &ro->v.o_const.v.c_string.v,
                                    SFCPTR_TOSTR (
                                        name->v.o_const.v.c_string.v)[p]);

                                r = sf_ot_addobj (ro);
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
                    && (ob_o->v.o_const.type == CONST_INT
                        || ob_o->v.o_const.type == CONST_FLOAT));

            step_c = ob_o->v.o_const.v.c_int.v;

            if (ob_o->v.o_const.type == CONST_INT)
              {
                step_c = ob_o->v.o_const.v.c_int.v;
              }
            else
              {
                assert (ceilf (ob_o->v.o_const.v.c_float.v)
                        == ob_o->v.o_const.v.c_float.v);

                step_c = ob_o->v.o_const.v.c_float.v;
              }
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

        assert (lv_o->type == OBJ_CONST && lv_o->v.o_const.type == CONST_INT
                || lv_o->v.o_const.type == CONST_FLOAT);

        assert (rv_o->type == OBJ_CONST && rv_o->v.o_const.type == CONST_INT
                || rv_o->v.o_const.type == CONST_FLOAT);

        if (lv_o->v.o_const.type == CONST_INT)
          {
            l_val = lv_o->v.o_const.v.c_int.v;
          }
        else
          {
            assert (ceilf (lv_o->v.o_const.v.c_float.v)
                    == lv_o->v.o_const.v.c_float.v);
            l_val = lv_o->v.o_const.v.c_float.v;
          }

        if (rv_o->v.o_const.type == CONST_INT)
          {
            r_val = rv_o->v.o_const.v.c_int.v;
          }
        else
          {
            assert (ceilf (rv_o->v.o_const.v.c_float.v)
                    == rv_o->v.o_const.v.c_float.v);
            r_val = rv_o->v.o_const.v.c_float.v;
          }

        array_t *narr = sf_array_new ();

        if (l_val + (tp_val[0] != '[') < r_val + (tp_val[1] == ']'))
          {
            // assert (step_c > 0);

            if (step_c > 0)
              {
                for (int i = l_val + (tp_val[0] != '[');
                     i < r_val + (tp_val[1] == ']'); i += step_c)
                  {
                    obj_t *v = sf_ast_objnew (OBJ_CONST);
                    v->v.o_const.type = CONST_INT;
                    v->v.o_const.v.c_int.v = i;

                    sf_array_pushVal (narr, sf_ot_addobj (v));
                  }
              }
          }

        else
          {
            // assert (step_c < 0);

            if (step_c < 0)
              {
                for (int i = l_val + (tp_val[0] != '[');
                     i > r_val + (tp_val[1] == ']'); i += step_c)
                  {
                    obj_t *v = sf_ast_objnew (OBJ_CONST);
                    v->v.o_const.type = CONST_INT;
                    v->v.o_const.v.c_int.v = i;

                    sf_array_pushVal (narr, sf_ot_addobj (v));
                  }
              }
          }

        obj_t *res = sf_ast_objnew (OBJ_ARRAY);
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

                  /**
                   * This is intentional.
                   * If `_sf_obj_cmp (mod, l_obj, co)` evaluates to true,
                   * then l_in_r = true and loop breaks.
                   * This means lval is present in rval.
                   * Thus, l_val in r_val => true
                   * Break the loop and return as bool
                   */
                  if ((l_in_r = _sf_obj_cmp (mod, l_obj, co)))
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

        obj_t *res = sf_ast_objnew (OBJ_CONST);
        res->v.o_const.type = CONST_BOOL;
        res->v.o_const.v.c_bool.v = l_in_r;

        r = sf_ot_addobj (res);

        sf_ll_set_meta_refcount (lv, lv->meta.ref_count - 1);
        sf_ll_set_meta_refcount (rv, rv->meta.ref_count - 1);
      }
      break;

    case EXPR_ARITHMETIC:
      {
        r = _sf_ev_arith (mod, e);
      }
      break;

    case EXPR_MEM_ACCESS:
      {
        expr_t *par = e->v.mem_access.parent;
        expr_t *vl = e->v.mem_access.val;

        llnode_t *par_ll;

        if (par->type == -1)
          {
            mod_t *p = mod;
            while (p && p->wb_gvref == NULL)
              p = p->parent;

            assert (!!p);
            assert (p->wb_gvref != NULL);

            par_ll = p->wb_gvref;
          }
        else
          par_ll = eval_expr (mod, par);

        sf_ll_set_meta_refcount (par_ll, par_ll->meta.ref_count + 1);

        obj_t *par_obj = (obj_t *)par_ll->val;
        assert (par_obj != NULL);
        assert (par_obj->meta.mem_ref == par_ll);

        // if (!mod->parent->parent)
        //   printf ("%s\n", sf_parser_objRepr (mod, par_obj));

        switch (par_obj->type)
          {
          case OBJ_CLASS:
            {
              class_t *cl = par_obj->v.o_class.val;

              switch (vl->type)
                {
                case EXPR_VAR:
                  {
                    sf_charptr vn = vl->v.var.name;
                    r = sf_mod_getVar (cl->mod, SFCPTR_TOSTR (vn));
                  }
                  break;

                default:
                  break;
                }
            }
            break;

          case OBJ_CLASSOBJ:
            {
              class_t *cl = par_obj->v.o_cobj.val;
              assert (cl->meta.iscobj);

              switch (vl->type)
                {
                case EXPR_VAR:
                  {
                    sf_charptr vn = vl->v.var.name;
                    r = sf_mod_getVar (cl->mod, SFCPTR_TOSTR (vn));

                    if (r == NULL)
                      {
                        /* check inherit list */
                        for (size_t j = 0; j < cl->il_c; j++)
                          {
                            mod_t *cm = ((obj_t *)cl->inh_list[j]->val)
                                            ->v.o_cobj.val->mod;

                            r = sf_mod_getVar (cm,
                                               SFCPTR_TOSTR (vl->v.var.name));
                          }
                      }
                  }
                  break;

                default:
                  break;
                }
            }
            break;

          case OBJ_MODULE:
            {
              sfmodule_t *md = par_obj->v.o_mod.val;

              switch (vl->type)
                {
                case EXPR_VAR:
                  {
                    sf_charptr vn = vl->v.var.name;
                    r = sf_mod_getVar (md->mod, SFCPTR_TOSTR (vn));
                  }
                  break;

                default:
                  break;
                }
            }
            break;

          case OBJ_CONST:
            {
              fun_t *f = sf_nm_get (vl->v.var.name, par_obj->v.o_const.type);
              assert (f->meta.has_id);

              obj_t *fo = sf_ast_objnew (OBJ_FUN);
              fo->v.o_fun.f = f;

              r = sf_ot_addobj (fo);
            }
            break;

          default:
            break;
          }

        if (r != NULL)
          {
            obj_t *rv = (obj_t *)r->val;
            assert (rv->meta.mem_ref == r);

            // rv->meta.passargs
            //     = sfrealloc (rv->meta.passargs, rv->meta.pa_size + 1);
            // rv->meta.passargs[rv->meta.pa_size++] = par_obj;

            // if (!mod->parent->parent)
            //   printf ("%s\n", sf_parser_objRepr (mod, rv));

            // if (rv->meta.pa_size)
            //   /*rv->meta.passargs = sfrealloc (
            //       rv->meta.passargs,
            //       (rv->meta.pa_size + 1) * sizeof (*rv->meta.passargs));*/
            //   {
            //     // if (!mod->parent->parent)
            //     //   here;
            //     for (size_t i = 0; i < rv->meta.pa_size; i++)
            //       {
            //         obj_t *cr = rv->meta.passargs[i];
            //         // printf ("[%s]\n", sf_parser_objRepr (mod, cr));

            //         if (cr->meta.mem_ref != NULL)
            //           {
            //             sf_ll_set_meta_refcount (
            //                 cr->meta.mem_ref,
            //                 cr->meta.mem_ref->meta.ref_count - 1);
            //           }
            //       }

            //     sffree (rv->meta.passargs);
            //     rv->meta.pa_size = 0;
            //   }
            // else

            // if (!rv->meta.pa_size)
            //   {
            //     if (rv->type == OBJ_FUN)
            //       {
            //         rv->meta.passargs = sfmalloc (sizeof
            //         (*rv->meta.passargs));

            //         rv->meta.passargs[rv->meta.pa_size++] = par_obj;
            //         par_obj->meta.mem_ref = par_ll;

            //         // if (!mod->parent->parent)
            //         // printf ("%s %d\n", sf_parser_objRepr (mod, par_obj),
            //         //         par_ll->meta.ref_count);

            //         sf_ll_set_meta_refcount (par_ll,
            //                                  par_ll->meta.ref_count + 1);
            //       }
            //   }
          }

        sf_ll_set_meta_refcount (par_ll, par_ll->meta.ref_count - 1);
      }
      break;

      // case EXPR_THIS:
      //   {
      //     // TODO
      //   }
      //   break;

    case EXPR_CONDITIONAL_EQEQ:
    case EXPR_CONDITIONAL_GT:
    case EXPR_CONDITIONAL_NEQ:
    case EXPR_CONDITIONAL_LT:
    case EXPR_CONDITIONAL_GTEQ:
    case EXPR_CONDITIONAL_LTEQ:
      {
        r = _sf_exec_compare (mod, e);
      }
      break;

    default:
      e_printf ("Unknown expression '%d' in eval_expr()\n", e->type);
      break;
    }

end:
  if (r == NULL)
    {
      sf_ast_exprprint (*e);

      assert (0 && "r is NULL in eval_expr");
    }

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
              const array_t *rt = robj->v.o_array.v;

              for (size_t i = 0; i < rt->len; i++)
                {
                  if (lv->type == EXPR_VAR)
                    {
                      const sf_charptr vname = lv->v.var.name;

                      sf_mod_addVar (mod, SFCPTR_TOSTR (vname), rt->vals[i]);
                    }
                  else
                    {
                      // TODO
                      // ? other types of expressions
                      /*
                        for i, j = 2 in [1, [1, 3], [1]]
                               ^^^^^ -> VAR_DECL
                      */
                    }

                  sf_parser_exec (mod);
                  // TODO: check for signals like continue, break
                }
            }
            break;

          case OBJ_CONST:
            {
              switch (robj->v.o_const.type)
                {
                case CONST_STRING:
                  {
                    const char *vl
                        = SFCPTR_TOSTR (robj->v.o_const.v.c_string.v);
                    const size_t d = strlen (vl);

                    for (size_t i = 0; i < d; i++)
                      {
                        const char c = vl[i];

                        if (lv->type == EXPR_VAR)
                          {
                            const sf_charptr vname = lv->v.var.name;

                            obj_t *oref = sf_ast_objnew (OBJ_CONST);
                            oref->v.o_const.type = CONST_STRING;
                            oref->v.o_const.v.c_string.v = sf_str_new_empty ();
                            sf_str_pushchr (&oref->v.o_const.v.c_string.v, c);

                            sf_mod_addVar (mod, SFCPTR_TOSTR (vname),
                                           sf_ot_addobj (oref));
                          }
                        else
                          {
                            // TODO
                            // ? other types of expressions
                            /*
                              for i, j = 2 in [1, [1, 3], [1]]
                                     ^^^^^ -> VAR_DECL
                            */
                          }

                        sf_parser_exec (mod);
                        // TODO: check for signals like continue, break
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

        sf_ll_set_meta_refcount (rnode, rnode->meta.ref_count - 1);
      }
      break;

    default:
      {
        /**
         * for (i = 0, i < 10, i = i + 1)
         *    <body>
         */
      }
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

void
_sf_exec_block_while (mod_t *mod, int i)
{
  stmt_t *t = &mod->body[i];
  llnode_t *cll = eval_expr (mod, t->v.blk_while.cond);

  sf_ll_set_meta_refcount (cll, cll->meta.ref_count + 1);

  obj_t *clv = (obj_t *)cll->val;

  stmt_t *pres_st = mod->body;
  size_t pres_size = mod->body_len;

  mod->body = t->v.blk_while.body;
  mod->body_len = t->v.blk_while.body_count;

  while (!_sf_obj_isfalse (mod, clv))
    {
      sf_parser_exec (mod);

      sf_ll_set_meta_refcount (cll, cll->meta.ref_count - 1);
      cll = eval_expr (mod, t->v.blk_while.cond);
      clv = (obj_t *)cll->val;
      sf_ll_set_meta_refcount (cll, cll->meta.ref_count + 1);
    }

  mod->body = pres_st;
  mod->body_len = pres_size;

  sf_ll_set_meta_refcount (cll, cll->meta.ref_count - 1);
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

          case CONST_FLOAT:
            return o->v.o_const.v.c_float.v == 0.0;

          case CONST_BOOL:
            return o->v.o_const.v.c_bool.v == 0;

          case CONST_STRING:
            return sf_str_eq_rCp (o->v.o_const.v.c_string.v, "");

          case CONST_NONE:
            return 1;

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

    default:
      break;
    }

end:
  return 0;
}

void
_sf_exec_block_if (mod_t *mod, size_t *ip)
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
              else
                (*ip)--;
            }
          else
            (*ip)--;
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
              (*ip)++;
            }

          else if (ntok->type == STMT_ELSEIF_BLOCK)
            {
              do
                {
                  (*ip)++;
                }
              while (*ip < mod->body_len
                     && mod->body[*ip].type == STMT_ELSEIF_BLOCK);

              if (*ip < mod->body_len
                  && mod->body[*ip].type == STMT_ELSE_BLOCK)
                (*ip)++;
            }
        }
    }

  sf_ll_set_meta_refcount (o, o->meta.ref_count - 1);
}

llnode_t *
_sf_fcall (mod_t *mod, expr_t *e)
{
  if (e->v.fun_call.name->type != -1) // function call
    {
      llnode_t *name = eval_expr (mod, e->v.fun_call.name);
      obj_t *fref = (obj_t *)name->val;

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

            // printf ("(%d %d)\n", fn->argc,
            //         fref->meta.pa_size + e->v.fun_call.arg_count);
            // assert (fn->argc == fref->meta.pa_size +
            // e->v.fun_call.arg_count);

            assert (fn->argc
                    == e->v.fun_call.arg_count + fref->v.o_fun.uses_self);

            // for (size_t j = 0; j < fref->meta.pa_size; j++)
            //   {
            //     assert (fref->meta.passargs[j]->meta.mem_ref != NULL);
            //     sf_mod_addVar (nmod, fn->args[j],
            //                    fref->meta.passargs[j]->meta.mem_ref);
            //   }

            if (fref->v.o_fun.uses_self)
              {
                sf_mod_addVar (nmod, fn->args[0], fref->v.o_fun.selfarg);
              }

            for (size_t j = fref->v.o_fun.uses_self;
                 j < e->v.fun_call.arg_count; j++)
              {
                // expr_t *ee = sfmalloc (sizeof (*ee));
                // *ee = e->v.fun_call.args[j];

                llnode_t *on = eval_expr (mod, &e->v.fun_call.args[j]);

                // sffree (ee);
                // printf ("%s\n", fn->args[j]);
                // printf ("%d\n", on->meta.ref_count);
                sf_mod_addVar (nmod, fn->args[j + fref->v.o_fun.uses_self],
                               on);
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
                    obj_t *rv = sf_ast_objnew (OBJ_CONST);
                    rv->v.o_const.type = CONST_NONE;

                    res = sf_ot_addobj (rv);
                  }
              }

            // for (size_t j = 0; j < fref->meta.pa_size; j++)
            //   {
            //     llnode_t *c = fref->meta.passargs[j]->meta.mem_ref;

            //     sf_ll_set_meta_refcount (c, c->meta.ref_count - 1);
            //   }

            // sffree (fref->meta.passargs);
            // fref->meta.passargs = NULL;
            // fref->meta.pa_size = 0;

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

        case OBJ_CLASS:
          {
            res = _sf_class_construct (mod, fref, e);
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

      /**
       * DO NOT UNCOMMENT.
       * It breaks the code
       * Causes deallocation twice
       * Source: tested it.
       * Reason for double deallocation: currently unknown
       */
      // if (res != NULL)
      //   res->meta.ref_count--;
      // printf ("[%d]\n", res->meta.ref_count);

      return res;
    }
  else
    {
      /**
       * Currently we are treating brackets as a
       * function call with no name.
       * The arguments of this function are evaluated ideally.
       * ! The first argument of arglist is returned.
       * TODO: Convert this arglist to tuple and return the tuple (if argcount
       * > 1).
       */

      llnode_t *res = NULL;
      for (size_t i = 0; i < e->v.fun_call.arg_count; i++)
        {
          llnode_t *c = eval_expr (mod, &e->v.fun_call.args[i]);

          /*
            Instead of setting ref_count to this+1 and then
            resetting it to this-1, I will just set ref_count
            as ref_count only to check for rvalues (and destroy them).
          */
          if (!i)
            res = c; // no self assignment as we are returning first index
          else
            sf_ll_set_meta_refcount (c, c->meta.ref_count); // self assignment
        }

      if (res == NULL)
        {
          obj_t *ro = sf_ast_objnew (OBJ_CONST);
          ro->v.o_const.type = CONST_NONE;

          res = sf_ot_addobj (ro);
        }

      return res;
    }

  return NULL; // unreachable code
}

llnode_t *
_sf_class_construct_noInit (mod_t *mod, obj_t *o)
{
  class_t *t = o->v.o_class.val;
  class_t *nobj = sf_class_new (t->name, SF_CLASS_CODED);

  nobj->meta.iscobj = 1;
  nobj->meta.clref = t;
  nobj->mod
      = sf_mod_new (MOD_TYPE_CLASS, (t->mod != NULL ? t->mod->parent : NULL));

  nobj->mod->meta.cref = nobj;

  char **vars = sf_trie_getKeys (t->mod->vtable);
  mod_t *par_pres = nobj->mod->parent;
  nobj->mod->parent = NULL;

  for (size_t i = 0; vars[i] != NULL; i++)
    {
      sf_mod_addVar (nobj->mod, vars[i], sf_mod_getVar (t->mod, vars[i]));
      sffree (vars[i]);
    }

  nobj->il_c = t->il_c;
  nobj->inh_list = sfmalloc (nobj->il_c * sizeof (llnode_t *));

  /* add inherits */
  for (size_t i = 0; i < t->il_c; i++)
    {
      llnode_t *l
          = _sf_class_construct_noInit (mod, (obj_t *)t->inh_list[i]->val);

      nobj->inh_list[i] = l;
    }

  nobj->mod->parent = par_pres;

  sf_class_add (nobj);

  obj_t *oj = sf_ast_objnew (OBJ_CLASSOBJ);
  oj->v.o_cobj.val = nobj;

  return sf_ot_addobj (oj);
}

llnode_t *
_sf_class_construct (mod_t *mod, obj_t *o, expr_t *e)
{
  class_t *t = o->v.o_class.val;
  class_t *nobj = sf_class_new (t->name, SF_CLASS_CODED);

  nobj->meta.iscobj = 1;
  nobj->meta.clref = t;
  nobj->mod
      = sf_mod_new (MOD_TYPE_CLASS, (t->mod != NULL ? t->mod->parent : NULL));

  nobj->mod->meta.cref = nobj;

  nobj->il_c = t->il_c;
  nobj->inh_list = sfmalloc (nobj->il_c * sizeof (llnode_t *));

  /* add inherits */
  for (size_t i = 0; i < t->il_c; i++)
    {
      llnode_t *l
          = _sf_class_construct_noInit (mod, (obj_t *)t->inh_list[i]->val);

      nobj->inh_list[i] = l;
    }

  char **vars = sf_trie_getKeys (t->mod->vtable);
  mod_t *par_pres = nobj->mod->parent;
  nobj->mod->parent = NULL;

  obj_t *oj = sf_ast_objnew (OBJ_CLASSOBJ);
  oj->v.o_cobj.val = nobj;

  llnode_t *oll = sf_ot_addobj (oj);

  for (size_t i = 0; vars[i] != NULL; i++)
    {
      llnode_t *vl = sf_mod_getVar (t->mod, vars[i]);
      obj_t *vlo = (obj_t *)vl->val;

      if (vlo->type == OBJ_FUN)
        {
          /**
           * There are instances when this condition if false.
           * consider,
           * class abc
           *    a = putln
           */
          if (vlo->v.o_fun.uses_self)
            {
              obj_t *on = sf_ast_objnew (OBJ_FUN);
              on->v.o_fun.uses_self = 1;
              on->v.o_fun.f = vlo->v.o_fun.f;
              on->v.o_fun.selfarg = oll;

              vl = sf_ot_addobj (on);
              sf_ll_set_meta_refcount (vl, vl->meta.ref_count + 1);
            }
        }
      sf_mod_addVar (nobj->mod, vars[i], vl);
      sffree (vars[i]);
    }

  nobj->mod->parent = par_pres;

  sf_class_add (nobj);

  /**
   * We pass `oll` to _init ()
   * In order to prevent being free'd
   * We set it's reference count to 1
   * So when passed as an argument it's reference count
   * goes to 2 and upon destruction, it falls back to 1
   * thereby preventing deallocation.
   *
   * Later, we will return this same object
   * but with reference count = 0
   */
  sf_ll_set_meta_refcount (oll, oll->meta.ref_count + 1);

  /*
    Call class._init()
  */
  /*llnode_t *fr = eval_expr (
      nobj->mod,
      (expr_t *)(expr_t[]){ (expr_t){
          .type = EXPR_FUN_CALL,
          .v.fun_call = { .arg_count = e->v.fun_call.arg_count,
                          .args = e->v.fun_call.args,
                          .name = (expr_t *)(expr_t[]){ (expr_t){
                              .type = EXPR_VAR,
                              .v.var.name = (
                                  sf_charptr)SF_CLASSCONSTRUCTOR_NAME,
                          } } } } });

  sf_ll_set_meta_refcount (fr, fr->meta.ref_count - 1);*/

  llnode_t *_init_ll = sf_mod_getVar (nobj->mod, SF_CLASSCONSTRUCTOR_NAME);

  if (_init_ll != NULL)
    {
      sf_ll_set_meta_refcount (_init_ll, _init_ll->meta.ref_count + 1);
      obj_t *init_o = (obj_t *)_init_ll->val;

      if (init_o->type != OBJ_FUN)
        {
          e_printf ("_init() is not callable in _sf_class_construct.\n");
        }
      else
        {
          fun_t *f = init_o->v.o_fun.f;
          mod_t *fmd = sf_mod_new (MOD_TYPE_FUNC, NULL);
          fmd->body = f->mod->body;
          fmd->body_len = f->mod->body_len;

          assert (f->argc > 0);
          sf_mod_addVar (fmd, f->args[0], oll);

          for (size_t i = 1; i < f->argc; i++)
            {
              expr_t *ee = &e->v.fun_call.args[i - 1];
              llnode_t *t = eval_expr (mod, ee);

              sf_mod_addVar (fmd, f->args[i], t);
            }

          fmd->parent = nobj->mod;
          sf_parser_exec (fmd);

          fmd->body = NULL;
          fmd->body_len = 0;
          fmd->parent = NULL;
          sf_mod_free (fmd);
        }

      sf_ll_set_meta_refcount (_init_ll, _init_ll->meta.ref_count - 1);
    }

  // ! WARNING: Possibly unsafe
  /**
   * Manually reset flags (and not using functions)
   * because I do not wish to lose the object.
   * (Basically, reset flags but don't check if it is 0
   */
  oll->meta.ref_count = 0;

  sffree (vars);
  return oll;
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
          {
            if (OBJ_IS_NUMBER (o1) && OBJ_IS_NUMBER (o2))
              {
                double d1, d2;

                d1 = o1->v.o_const.type == CONST_INT
                         ? o1->v.o_const.v.c_int.v
                         : o1->v.o_const.v.c_float.v;

                d2 = o2->v.o_const.type == CONST_INT
                         ? o2->v.o_const.v.c_int.v
                         : o2->v.o_const.v.c_float.v;

                return d1 == d2;
              }

            goto end;
          }

        switch (o1->v.o_const.type)
          {
          case CONST_BOOL:
            return o1->v.o_const.v.c_bool.v == o2->v.o_const.v.c_bool.v;

          case CONST_FLOAT:
            return o1->v.o_const.v.c_float.v == o2->v.o_const.v.c_float.v;

          case CONST_INT:
            return o1->v.o_const.v.c_int.v == o2->v.o_const.v.c_int.v;

          case CONST_STRING:
            return sf_str_eq (o1->v.o_const.v.c_string.v,
                              o2->v.o_const.v.c_string.v);

          case CONST_NONE:
            return o2->v.o_const.type == CONST_NONE;

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

llnode_t *
_sf_exec_compare (mod_t *mod, expr_t *e)
{
  int r = 0;
  llnode_t *lv = eval_expr (mod, e->v.expr_conditional.lval);
  llnode_t *rv = eval_expr (mod, e->v.expr_conditional.rval);

  sf_ll_set_meta_refcount (lv, lv->meta.ref_count + 1);
  sf_ll_set_meta_refcount (rv, rv->meta.ref_count + 1);

  /**
   * Write logic for, say, `>=` operator.
   * Now, for `<`, set swch to 1 and jump to logic of `>=` and execute
   * with switched logic.
   */
  int swch = 0; // switch logic

  obj_t *lvo = (obj_t *)lv->val;
  obj_t *rvo = (obj_t *)rv->val;

  switch (e->type)
    {
    case EXPR_CONDITIONAL_GT:
      {
      cnd_lab_gt:
        if (OBJ_IS_NUMBER (lvo) && OBJ_IS_NUMBER (rvo))
          {
            const float l1 = lvo->v.o_const.type == CONST_INT
                                 ? (float)lvo->v.o_const.v.c_int.v
                                 : lvo->v.o_const.v.c_float.v;

            const float l2 = rvo->v.o_const.type == CONST_INT
                                 ? (float)rvo->v.o_const.v.c_int.v
                                 : rvo->v.o_const.v.c_float.v;

            r = swch ? (l1 - l2 <= 0.0f) : (l1 - l2 > 0.0f);
          }
      }
      break;

    case EXPR_CONDITIONAL_LT:
      {
      cnd_lab_lt:
        if (OBJ_IS_NUMBER (lvo) && OBJ_IS_NUMBER (rvo))
          {
            const float l1 = lvo->v.o_const.type == CONST_INT
                                 ? (float)lvo->v.o_const.v.c_int.v
                                 : lvo->v.o_const.v.c_float.v;

            const float l2 = rvo->v.o_const.type == CONST_INT
                                 ? (float)rvo->v.o_const.v.c_int.v
                                 : rvo->v.o_const.v.c_float.v;

            r = swch ? (l1 - l2 >= 0.0f) : (l1 - l2 < 0.0f);
          }
      }
      break;

    case EXPR_CONDITIONAL_NEQ:
      {
        r = !_sf_obj_cmp (mod, (obj_t *)lv->val, (obj_t *)rv->val);
      }
      break;

    case EXPR_CONDITIONAL_EQEQ:
      {
        r = _sf_obj_cmp (mod, (obj_t *)lv->val, (obj_t *)rv->val);
      }
      break;

    case EXPR_CONDITIONAL_GTEQ:
      {
        swch = 1;
        goto cnd_lab_lt;
      }
      break;

    case EXPR_CONDITIONAL_LTEQ:
      {
        swch = 1;
        goto cnd_lab_gt;
      }
      break;

    default:
      {
        e_printf ("Cannot compare types '%d' and '%d'",
                  e->v.expr_conditional.lval->type,
                  e->v.expr_conditional.rval->type);
      }
      break;
    }

  sf_ll_set_meta_refcount (lv, lv->meta.ref_count - 1);
  sf_ll_set_meta_refcount (rv, rv->meta.ref_count - 1);

  obj_t *o_res = sf_ast_objnew (OBJ_CONST);
  o_res->v.o_const.type = CONST_BOOL;
  o_res->v.o_const.v.c_bool.v = r;

  return sf_ot_addobj (o_res);
}

void *
vcr (void *v)
{
  struct _sfa_treetok_s *n = (void *)v;

  struct _sfa_treetok_s *r = (struct _sfa_treetok_s *)sfmalloc (sizeof (*r));
  r->is_llnode = n->is_llnode;
  r->is_op = n->is_op;
  r->node = n->node;
  r->v = n->v;

  return r;
}

void
vcr2 (void *v)
{
  sffree (v);
}

llnode_t *
_sf_ev_arith (mod_t *m, expr_t *e)
{
  tree_t *ac = sf_tree_copy_deep (e->v.e_arith.tree, vcr);
  int c = sf_tree_countNodes (ac);

  tree_t **nodes = sfmalloc (c * sizeof (*nodes));
  int nc = 0;
  nodes[nc++] = ac;

  while (nc > 0)
    {
      tree_t *last_node = nodes[--nc];

      struct _sfa_treetok_s *nval = (struct _sfa_treetok_s *)last_node->val;

      if (!nval->is_op)
        {
          nval->is_llnode = 1;
          expr_t *ecpy = nval->v.val;

          nval->node = eval_expr (m, ecpy);

          // printf ("%s\n", sf_parser_objRepr (m, nval->node->val));
          sf_ll_set_meta_refcount (nval->node, nval->node->meta.ref_count + 1);
        }

      if (last_node->left)
        nodes[nc++] = last_node->left;

      if (last_node->right)
        nodes[nc++] = last_node->right;
    }

  __sfaarith_res res = sf_arith_eval_tree (m, ac);

  obj_t *robj = sf_ast_objnew (OBJ_CONST);
  robj->v.o_const.type = CONST_FLOAT;

  llnode_t *retval;
  if (res.type == SF_ARITH_RES_DOUBLE)
    {
      robj->v.o_const.v.c_float.v = res.v.dres.v;
      retval = sf_ot_addobj (robj);
    }

  else
    {
      robj = res.v.ores.v;

      assert (robj->meta.mem_ref != NULL);
      retval = robj->meta.mem_ref;
    }

  nc = 0;
  nodes[nc++] = ac;

  while (nc > 0)
    {
      tree_t *last_node = nodes[--nc];

      struct _sfa_treetok_s *nval = (struct _sfa_treetok_s *)last_node->val;

      if (!nval->is_op)
        {
          sf_ll_set_meta_refcount (nval->node, nval->node->meta.ref_count - 1);
          nval->node = NULL;
          nval->is_llnode = 0;
        }

      if (last_node->left)
        nodes[nc++] = last_node->left;

      if (last_node->right)
        nodes[nc++] = last_node->right;
    }

  sffree (nodes);

  /* obj_t *noneobj = sfmalloc (sizeof (*noneobj));
  noneobj->type = OBJ_CONST;
  noneobj->v.o_const.type = CONST_NONE;
  llnode_t *retval = sf_ot_addobj (noneobj); */
  sf_tree_free (ac, vcr2);

  return retval;
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

              sprintf (r, "%g", obj->v.o_const.v.c_float.v);

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

          case CONST_NONE:
            res = sf_str_new_fromStr (SF_DTYPE_NONE_REPR);
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

    case OBJ_MAP:
      {
        map_t *t = obj->v.o_map.v;
        assert (t != NULL);

        res = sf_str_new_fromStr ("{");

        char **skeys = sf_trie_getKeys (t->t);

        for (size_t i = 0; skeys[i] != NULL; i++)
          {
            obj_t *ko
                = (obj_t *)((llnode_t *)sf_trie_getVal (t->t, skeys[i]))->val;

            char *p = sf_parser_objRepr (mod, ko);

            sf_str_pushchr (&res, '\'');
            sf_str_push (&res, skeys[i]);

            if (ko->type == OBJ_CONST && ko->v.o_const.type == CONST_STRING)
              sf_str_push (&res, "': '");
            else
              sf_str_push (&res, "': ");

            sf_str_push (&res, p);

            if (ko->type == OBJ_CONST && ko->v.o_const.type == CONST_STRING)
              sf_str_push (&res, "'");

            sffree (p);

            if (skeys[i + 1] != NULL)
              sf_str_push (&res, ", ");

            sffree (skeys[i]);
          }

        sf_str_push (&res, "}");
        sffree (skeys);
      }
      break;

    case OBJ_CLASS:
      {
        class_t *t = obj->v.o_class.val;
        res = sf_str_new_fromSize ((13 + strlen (t->name)) * sizeof (char));

        sprintf (res, "<class '%s'>", t->name);
      }
      break;

    case OBJ_CLASSOBJ:
      {
        class_t *t = obj->v.o_cobj.val;
        res = sf_str_new_fromSize ((22 + strlen (t->name)) * sizeof (char));

        sprintf (res, "<class instance '%s'>", t->name);
      }
      break;

    case OBJ_MODULE:
      {
        res = sf_str_new_fromSize (128 * sizeof (char));

        sprintf (res, "<module '%s' ('%s')>", obj->v.o_mod.val->alias,
                 obj->v.o_mod.val->path);
      }
      break;

    default:
      e_printf ("Unknown type '%d' in sf_parser_objRepr()\n", obj->type);
      break;
    }

  return SFCPTR_TOSTR (res);
}