#include "sfarray.h"

array_t **ARRAY_STACK;
size_t as_count;

int ARR_CACHE[SF_ARR_CACHE_SIZE];
int achc_count;

SF_API void
sf_array_init (void)
{
  ARRAY_STACK = NULL;
  as_count = 0;
  achc_count = 0;
}

SF_API array_t *
sf_array_new (void)
{
  array_t *t = sfmalloc (sizeof (*t));
  t->len = 0;
  t->vals = NULL;

  t->meta.has_id = 0;
  t->meta.id = 0;

  return t;
}

SF_API void
sf_array_pushVal (array_t *arr, llnode_t *val)
{
  arr->vals = sfrealloc (arr->vals, (arr->len + 1) * sizeof (*arr->vals));
  arr->vals[arr->len++] = val;

  sf_ll_set_meta_refcount (val, val->meta.ref_count + 1);
}

SF_API array_t *
sf_array_add (array_t *arr)
{
  arr->meta.has_id = 1;

  if (achc_count)
    {
      int l = ARR_CACHE[--achc_count];
      ARRAY_STACK[l] = arr;

      arr->meta.id = l;

      return ARRAY_STACK[l];
    }

  else
    {
      ARRAY_STACK
          = sfrealloc (ARRAY_STACK, (as_count + 1) * sizeof (*ARRAY_STACK));
      ARRAY_STACK[as_count] = arr;

      arr->meta.id = as_count;
    }

  return ARRAY_STACK[as_count++];
}

SF_API void
sf_array_free (array_t *arr)
{
  if (achc_count < SF_ARR_CACHE_SIZE && arr->meta.has_id)
    ARR_CACHE[achc_count++] = arr->meta.id;

  for (size_t i = 0; i < arr->len; i++)
    {
      //   printf ("%d\n", arr->vals[i]->meta.ref_count);
      sf_ll_set_meta_refcount (arr->vals[i], arr->vals[i]->meta.ref_count - 1);
    }

  if (arr->vals != NULL)
    sffree (arr->vals);

  arr->len = 0;
  arr->vals = NULL;

  sffree (arr);
}