#include "sfmap.h"

map_t **MAP_STACK;
size_t mp_count;

int MAP_CACHE[SF_MAP_CACHE_SIZE];
int mch_count;

SF_API void
sf_map_init (void)
{
  MAP_STACK = NULL;
  mp_count = 0;
  mch_count = 0;
}

SF_API map_t *
sf_map_new (void)
{
  map_t *t = sfmalloc (sizeof (*t));
  t->t = sf_trie_new ();

  return t;
}

SF_API void
sf_map_addKeyVal (map_t *m, sf_charptr n, llnode_t *v)
{
  sf_trie_add (m->t, SFCPTR_TOSTR (n), v);
}

SF_API map_t *
sf_map_add (map_t *m)
{
  m->meta.has_id = 1;

  if (mch_count)
    {
      int l = MAP_CACHE[--mch_count];
      MAP_STACK[l] = m;

      m->meta.id = l;

      return MAP_STACK[l];
    }

  else
    {
      MAP_STACK = sfrealloc (MAP_STACK, (mp_count + 1) * sizeof (*MAP_STACK));
      MAP_STACK[mp_count] = m;

      m->meta.id = mp_count;
    }

  return MAP_STACK[mp_count++];
}

SF_API void
sf_map_free (map_t *m)
{
  if (mch_count < SF_MAP_CACHE_SIZE && m->meta.has_id)
    MAP_CACHE[mch_count++] = m->meta.id;

  char **skeys = sf_trie_getKeys (m->t);

  for (size_t i = 0; skeys[i] != NULL; i++)
    {
      llnode_t *v = (llnode_t *)sf_trie_getVal (m->t, skeys[i]);
      sf_ll_set_meta_refcount (v, v->meta.ref_count - 1);

      sffree (skeys[i]);
    }

  sffree (skeys);

  sf_trie_free (m->t);

  sffree (m);
}