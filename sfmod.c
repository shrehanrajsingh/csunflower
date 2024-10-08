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

  return m;
}

SF_API llnode_t *
sf_mod_addVar (mod_t *mod, char *name, llnode_t *ref)
{
  llnode_t *l = sf_mod_getVar (mod, name);

  if (l != NULL)
    {
      // here;
      sf_ll_set_meta_refcount (l, l->meta.ref_count - 1);
    }

#if !defined(SF_DISABLE_THIS)
  obj_t *v = (obj_t *)ref->val;
  v->meta.last_kwtr = _kwtr_get_back ();
#endif

  sf_trie_add (mod->vtable, name, (void *)ref);
  sf_ll_set_meta_refcount (ref, ref->meta.ref_count + 1);

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
  mod->parent = NULL;
  char **allkeys = sf_trie_getKeys (mod->vtable);

  for (size_t i = 0; allkeys[i] != NULL; i++)
    {
      llnode_t *v = sf_trie_getVal (mod->vtable, allkeys[i]);

      if (v == NULL)
        continue;

      obj_t *vo = (obj_t *)v->val;
      if (vo->meta.pa_size > 0)
        {
          for (size_t i = 0; i < vo->meta.pa_size; i++)
            {
              llnode_t *t = vo->meta.passargs[i]->meta.mem_ref;
              sf_ll_set_meta_refcount (t, t->meta.ref_count - 1);
            }

          sffree (vo->meta.passargs);
          vo->meta.passargs = NULL;
          vo->meta.pa_size = 0;
        }
    }

  /**
   * ! BUG
   * * Resolved
   * class objects might have a `_kill` routine
   * which might involve variables of the module.
   * If those variables are removed before class object
   * is destroyed, it would lead to undefined behaviour.
   *
   * ? Solution
   * Call the `_kill` routine of all class objects before
   * destroying any variables
   */

  for (size_t i = 0; allkeys[i] != NULL; i++)
    {
      if (allkeys[i][0] == '\0')
        continue;

      llnode_t *v = sf_trie_getVal (mod->vtable, allkeys[i]);

      if (v == NULL)
        continue;

      obj_t *vo = (obj_t *)v->val;

      if (vo->type == OBJ_CLASSOBJ && v->meta.ref_count == 1)
        {
          // printf ("%s %d %d\n", allkeys[i], vo->type, v->meta.ref_count);
          sf_ll_set_meta_refcount (
              v, 0); // kill routine will be called in objfree routine

          allkeys[i][0] = '\0';
        }
    }

  for (size_t i = 0; allkeys[i] != NULL; i++)
    {
      if (allkeys[i][0] == '\0')
        {
          sffree (allkeys[i]);
          continue;
        }

      llnode_t *v = sf_trie_getVal (mod->vtable, allkeys[i]);

      if (v == NULL)
        continue;

      // printf ("%s %d\n", allkeys[i], v->meta.ref_count);
      sf_ll_set_meta_refcount (v, v->meta.ref_count - 1);

      sffree (allkeys[i]);
    }

  sffree (allkeys);

  if (mod->retv != NULL)
    sf_ll_set_meta_refcount (mod->retv, mod->retv->meta.ref_count - 1);

  sf_trie_free (mod->vtable);
  sffree (mod);
}