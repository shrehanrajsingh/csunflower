#include "sfmod.h"

SF_API llnode_t *
sf_mod_addVar (mod_t *mod, char *name, obj_t *ref)
{
  llnode_t *r = sf_ot_addobj (ref);

  sf_trie_add (mod->vtable, sfstrdup (name), (void *)r);
  obj_t *rt = sf_ll_set_meta_refcount (r, r->meta.ref_count + 1);

  if (rt != NULL)
    {
      // TODO
      // Node was unlinked from memory
      // Free rt now
    }

  return r;
}

SF_API llnode_t *
sf_mod_getVar (mod_t *mod, const char *name)
{
  if (!mod)
    {
      e_printf ("Variable '%s' does not exist in sf_mod_getVar()\n", name);
      return NULL;
    }

  llnode_t *res = sf_trie_getVal (mod->vtable, name);

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

  sf_trie_free (mod->vtable);
  sffree (mod);
}