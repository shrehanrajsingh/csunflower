#include "sfmod.h"

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

  return m;
}

SF_API llnode_t *
sf_mod_addVar (mod_t *mod, char *name, llnode_t *ref)
{
  sf_trie_add (mod->vtable, sfstrdup (name), (void *)ref);
  obj_t *rt = sf_ll_set_meta_refcount (ref, ref->meta.ref_count + 1);

  if (rt != NULL)
    {
      // TODO
      // Node was unlinked from memory
      // Free rt now
    }

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

  llnode_t *res = sf_trie_getVal (mod->vtable, (char *)name);

  if (res == NULL)
    return sf_mod_getVar (mod->parent, name);

  return res;
}

SF_API void
sf_mod_free (mod_t *mod)
{
  char **allkeys = sf_trie_getKeys (mod->vtable);

  for (size_t i = 0; allkeys[i] != NULL; i++)
    {
      llnode_t *v = sf_trie_getVal (mod->vtable, allkeys[i]);
      sf_ll_set_meta_refcount (v, v->meta.ref_count - 1);

      sffree (allkeys[i]);
    }

  sf_ll_set_meta_refcount (mod->retv, mod->retv->meta.ref_count - 1);

  sf_trie_free (mod->vtable);
  sffree (mod);
}