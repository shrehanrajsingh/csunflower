#include "function.h"

fun_t *FUN_STACK;
size_t fs_count;

SF_API void
sf_fun_init (void)
{
  FUN_STACK = NULL;
  fs_count = 0;
}

SF_API fun_t
sf_fun_new (int type, struct _mod_s *mod, void *rtin)
{
  fun_t f = (fun_t){
    .mod = mod,
    .type = type,
    .native.routine = rtin,
  };

  return f;
}

SF_API fun_t *
sf_fun_add (fun_t _Fun)
{
  FUN_STACK = sfrealloc (FUN_STACK, (fs_count + 1) * sizeof (*FUN_STACK));
  FUN_STACK[fs_count++] = _Fun;

  return &FUN_STACK[fs_count - 1];
}