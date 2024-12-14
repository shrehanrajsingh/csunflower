#include "sfmod.h"
#include "parser.h"

SF_API mod_t *
sf_mod_new (int type, mod_t *parent)
{
  mod_t *m = sfmalloc (sizeof (*m));
  m->type = type;
  m->body = NULL;
  m->body_len = 0;
  m->parent = parent;
  m->retv = NULL;
  m->vtable = sf_trie_new ();
  m->meta.cref = NULL;
  m->vhcap = SF_MOD_VH_CAP;
  m->vhcount = 0;
  m->varhist = sfmalloc (m->vhcap * sizeof (*m->varhist));

  return m;
}

SF_API llnode_t *
sf_mod_addVar (mod_t *mod, char *name, llnode_t *ref)
{
  llnode_t *l = sf_mod_getVar (mod, name);

  mod_t *par_pres = mod->parent;
  mod->parent = NULL;

  llnode_t *o = sf_mod_getVar (mod, name);
  mod->parent = par_pres;

  sf_ll_set_meta_refcount (ref, ref->meta.ref_count + 1);
  // if (mod->parent && !mod->parent->parent)
  //   printf ("%d\n", ref->meta.ref_count);

  if (l != NULL)
    {
      if (o != NULL)
        {
          char saw_index = 0;
          size_t idx = 0;

          for (size_t i = 0; i < mod->vhcount; i++)
            {
              if (saw_index)
                {
                  mod->varhist[i - 1] = mod->varhist[i];
                }

              if (sf_str_eq_rCp (mod->varhist[i], name))
                {
                  sffree (mod->varhist[i]);
                  saw_index = 1;
                }
            }

          mod->varhist[mod->vhcount - 1] = sf_str_new_fromStr (name);
        }

      sf_ll_set_meta_refcount (l, l->meta.ref_count - 1);
    }
  else
    {
      if (mod->vhcount == mod->vhcap)
        {
          mod->vhcap += SF_MOD_VH_CAP;
          mod->varhist
              = sfrealloc (mod->varhist, mod->vhcap * sizeof (*mod->varhist));
        }

      mod->varhist[mod->vhcount++] = sf_str_new_fromStr (name);
    }

#if !defined(SF_DISABLE_THIS)
  obj_t *v = (obj_t *)ref->val;
  v->meta.last_kwtr = _kwtr_get_back ();
#endif

  sf_trie_add (mod->vtable, name, (void *)ref);

  /**
   * We set refcount to +2 previously
   * so now we decrease it by 1 to maintain refcount
   * FIXED
   * DO NOT UNCOMMENT NOW
   */
  // sf_ll_set_meta_refcount (ref, ref->meta.ref_count - 1);
  // printf ("%s %d %d\n", name, ref->meta.ref_count, l == NULL);

  return ref;
}

SF_API llnode_t *
sf_mod_getVar (mod_t *mod, const char *name)
{
  if (!mod)
    {
      e_printf ("Variable '%s' does not exist in sf_mod_getVar()\n", name);
      return NULL;
    }

  llnode_t *res = (llnode_t *)sf_trie_getVal (mod->vtable, (char *)name);

  if (res == NULL)
    return sf_mod_getVar (mod->parent, name);

  return res;
}

SF_API void
sf_mod_free (mod_t *mod)
{
  // mod_t *mpres = mod->parent;
  // mod->parent = NULL;

  // for (int i = mod->vhcount - 1; i >= 0; i--)
  //   {
  //     llnode_t *v = sf_trie_getVal (mod->vtable, mod->varhist[i]);

  //     if (v == NULL)
  //       continue;

  //     // obj_t *vo = (obj_t *)v->val;
  //     // if (vo->meta.pa_size > 0)
  //     //   {
  //     //     for (size_t i = 0; i < vo->meta.pa_size; i++)
  //     //       {
  //     //         llnode_t *t = vo->meta.passargs[i]->meta.mem_ref;
  //     //         sf_ll_set_meta_refcount (t, t->meta.ref_count - 1);
  //     //       }

  //     //     sffree (vo->meta.passargs);
  //     //     vo->meta.passargs = NULL;
  //     //     vo->meta.pa_size = 0;
  //     //   }
  //   }

  // Call _kill on all classes before destroying any objects
  for (int i = mod->vhcount - 1; i >= 0; i--)
    {
      llnode_t *v = sf_trie_getVal (mod->vtable, mod->varhist[i]);

      if (v == NULL)
        continue;

      obj_t *vo = (obj_t *)v->val;

      if (vo->type == OBJ_CLASSOBJ && v->meta.ref_count == 1)
        {
          // printf ("%s %d %d\n", mod->varhist[i], vo->type,
          // v->meta.ref_count);

          class_t *ct = vo->v.o_cobj.val;

          llnode_t *kln = sf_mod_getVar (ct->mod, "_kill");
          // printf ("%d\n", kln == NULL);
          int drop_f = ct->meta.kill_fun_called;

          if (kln != NULL)
            {
              sf_ll_set_meta_refcount (kln, kln->meta.ref_count + 1);
              obj_t *kv = (obj_t *)kln->val;

              if (kv->type == OBJ_FUN && !ct->meta.kill_fun_called)
                {
                  ct->meta.kill_fun_called = 1;
                  fun_t *f = kv->v.o_fun.f;

                  assert (f->argc == 1 && "_kill() expects 1 argument.");
                  mod_t *kmod = sf_mod_new (MOD_TYPE_FUNC, NULL);

                  kmod->body = f->mod->body;
                  kmod->body_len = f->mod->body_len;

                  // obj_t *kp = sf_ast_objnew (OBJ_CLASSOBJ);
                  // kp->v.o_cobj.val = ct;

                  // sf_mod_addVar (kmod, "self", sf_ot_addobj (kp));
                  sf_mod_addVar (kmod, "self", v);
                  kmod->parent = f->mod->parent;

                  sf_parser_exec (kmod);
                  sf_mod_free (kmod);
                }

              sf_ll_set_meta_refcount (kln, kln->meta.ref_count - 1);
            }

          if (vo->v.o_fun.uses_self)
            {
              llnode_t *t = vo->v.o_fun.selfarg;
              sf_ll_set_meta_refcount (t, t->meta.ref_count - 1);
            }
        }
    }

  for (int i = mod->vhcount - 1; i >= 0; i--)
    {
      llnode_t *v = sf_trie_getVal (mod->vtable, mod->varhist[i]);

      if (v == NULL)
        continue;

      // printf ("%s %d\n", mod->varhist[i], v->meta.ref_count);
      sf_ll_set_meta_refcount (v, v->meta.ref_count - 1);

      sffree (mod->varhist[i]);
    }

  if (mod->retv != NULL)
    sf_ll_set_meta_refcount (mod->retv, mod->retv->meta.ref_count - 1);

  sffree (mod->varhist);
  sf_trie_free (mod->vtable);
  sffree (mod);
}