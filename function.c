#include "function.h"
#include "sfmod.h"

fun_t **FUN_STACK;
size_t fs_count;

int FUN_CACHE[SF_FUN_CACHE_SIZE];
int fcch_count;

SF_API void
sf_fun_init (void)
{
  FUN_STACK = NULL;
  fs_count = 0;
  fcch_count = 0;
}

SF_API fun_t *
sf_fun_new (char *name, int type, struct _mod_s *mod, void *rtin)
{
  fun_t *f = sfmalloc (sizeof (*f));

  *f = (fun_t){
    .name = sfstrdup (name),
    .mod = mod,
    .type = type,
    .native.routine = rtin,
    .args = NULL,
    .argc = 0,
    .meta = {
      .has_id = 0,
      .id = 0,
    },
  };

  return f;
}

SF_API void
sf_fun_addarg (fun_t *fun, char *name)
{
  fun->args = sfrealloc (fun->args, (fun->argc + 1) * sizeof (*fun->args));
  fun->args[fun->argc++] = sfstrdup (name);
}

SF_API fun_t *
sf_fun_add (fun_t *_Fun)
{
  _Fun->meta.has_id = 1;

  if (fcch_count)
    {
      int p = FUN_CACHE[--fcch_count];
      FUN_STACK[p] = _Fun;
      _Fun->meta.id = p;

      return FUN_STACK[p];
    }
  else
    {
      FUN_STACK = sfrealloc (FUN_STACK, (fs_count + 1) * sizeof (*FUN_STACK));
      FUN_STACK[fs_count] = _Fun;
      _Fun->meta.id = fs_count;

      return FUN_STACK[fs_count++];
    }

  return NULL;
}

SF_API fun_t ***
sf_fun_getStack (void)
{
  return &FUN_STACK;
}

SF_API void
sf_fun_free (fun_t *fun)
{
  if (fcch_count < SF_FUN_CACHE_SIZE && fun->meta.has_id)
    {
      FUN_CACHE[fcch_count++] = fun->meta.id;
    }

  for (size_t i = 0; i < fun->argc; i++)
    {
      sffree (fun->args[i]);
    }

  sffree (fun->args);
  sffree (fun->name);
  sf_mod_free (fun->mod);
  sffree (fun);
}