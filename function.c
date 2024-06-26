#include "function.h"

fun_t **FUN_STACK;
size_t fs_count;

SF_API void
sf_fun_init (void)
{
  FUN_STACK = NULL;
  fs_count = 0;
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
  FUN_STACK = sfrealloc (FUN_STACK, (fs_count + 1) * sizeof (*FUN_STACK));
  FUN_STACK[fs_count] = _Fun;

  return FUN_STACK[fs_count++];
}