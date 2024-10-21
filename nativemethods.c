#include "nativemethods.h"

trie_t *sfnm_stack = NULL;

SF_API void
sf_nm_init ()
{
  sfnm_stack = sf_trie_new ();
}

SF_API void
sf_nm_add (const char *name, int type, fun_t *ref)
{
  struct _nmv_bag_s *vl
      = (struct _nmv_bag_s *)sf_trie_getVal (sfnm_stack, (char *)name);

  if (vl == NULL)
    {
      vl = sfmalloc (sizeof (*vl));
      vl->val = sfmalloc (sizeof (*vl->val));
      vl->val->type = type;
      vl->val->ref = ref;
      vl->size = 1;

      sf_trie_add (sfnm_stack, (char *)name, (void *)vl);
      sf_fun_add (ref);
    }
  else
    {
      vl->val = sfrealloc (vl->val, (vl->size + 1) * sizeof (*vl->val));
      vl->val[vl->size].type = type;
      vl->val[vl->size].ref = ref;
      vl->size++;
    }
}

SF_API fun_t *
sf_nm_get (const char *name, int type)
{
  struct _nmv_bag_s *vl
      = (struct _nmv_bag_s *)sf_trie_getVal (sfnm_stack, (char *)name);

  if (vl == NULL)
    return NULL;

  for (int i = 0; i < vl->size; i++)
    {
      if (vl->val[i].type == type)
        return vl->val[i].ref;
    }

  return NULL;
}