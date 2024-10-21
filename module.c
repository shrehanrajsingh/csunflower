#include "module.h"
#include "ast.h"
#include "sfmod.h"

sfmodule_t **SFMODULE_STACK;
size_t sfm_count;
size_t sfm_capacity;

int SFMODULE_CACHE[SFMODULE_CACHE_SIZE];
size_t sfm_chc;

sf_charptr SFMODULE_PATHS[SFMODULE_CACHE_SIZE];
size_t sfmp_c; // sfmodule_path_count

mod_t *SF_PARENT_MOD;

SF_API void
sf_module_init (void)
{
  sfm_count = 0;
  sfm_capacity = SFMODULE_CAPACITY;

  SFMODULE_STACK = sfmalloc (sfm_capacity * sizeof (*SFMODULE_STACK));
  sfm_chc = 0;

  sfmp_c = 0;
  SF_PARENT_MOD = NULL;
}

SF_API void
sf_module_setparent (mod_t *m)
{
  SF_PARENT_MOD = m;
}

SF_API mod_t **
sf_module_getparent (void)
{
  return &SF_PARENT_MOD;
}

SF_API sfmodule_t *
sf_module_new (int type, struct _mod_s *md)
{
  sfmodule_t *t = sfmalloc (sizeof (*t));
  t->type = type;
  t->mod = md;
  t->path = NULL;
  t->alias = NULL;
  t->meta.has_id = 0;
  t->meta.id = -1;

  return t;
}

SF_API void
sf_module_set_path (sfmodule_t *md, char *s)
{
  md->path = sfstrdup (s);
}

SF_API void
sf_module_set_alias (sfmodule_t *md, char *s)
{
  md->alias = sfstrdup (s);
}

SF_API sfmodule_t *
sf_module_add (sfmodule_t *t)
{
  t->meta.has_id = 1;
  if (sfm_chc)
    {
      int p = SFMODULE_CACHE[--sfm_chc];

      t->meta.id = p;

      SFMODULE_STACK[p] = t;
    }
  else
    {
      if (sfm_count == sfm_capacity)
        {
          sfm_capacity += SFMODULE_CAPACITY;

          SFMODULE_STACK = sfrealloc (SFMODULE_STACK,
                                      sfm_capacity * sizeof (*SFMODULE_STACK));
        }

      t->meta.id = sfm_count;
      SFMODULE_STACK[sfm_count++] = t;
    }

  return t;
}

SF_API void
sf_module_addlookuppath (const char *path)
{
  SFMODULE_PATHS[sfmp_c] = sf_str_new_fromStr (path);

  if (path[strlen (path) - 1] != '/')
    sf_str_pushchr (&SFMODULE_PATHS[sfmp_c], '/');

  sfmp_c++;
}

SF_API void
sf_module_dest (void)
{
  for (size_t i = 0; i < sfmp_c; i++)
    sffree (SFMODULE_PATHS[i]);
}

SF_API sf_charptr
sf_module_getfilecontents (sf_charptr p)
{
  FILE *fptr;
  sf_charptr l;

  const size_t CONT_CAP = 512;
  char *cont, c;

  size_t cont_count = 0;
  size_t cont_capacity = CONT_CAP;

  cont = sfmalloc (cont_capacity * sizeof (char));

  for (size_t i = 0; i < sfmp_c; i++)
    {
      l = sf_str_new_fromStr (SFMODULE_PATHS[i]);
      sf_str_push (&l, SFCPTR_TOSTR (p));

      fptr = fopen (SFCPTR_TOSTR (l), "r");

      sf_str_free (&l);

      if (fptr != NULL)
        break;
    }

  while ((c = fgetc (fptr)) != EOF)
    {
      if (cont_count == cont_capacity)
        {
          cont_capacity += CONT_CAP;
          cont = sfrealloc (cont, cont_capacity * sizeof (char));
        }

      cont[cont_count++] = c;
    }

  if (!cont_count)
    {
      sffree (cont);
      cont = NULL;
    }
  else
    cont[cont_count++] = '\0';

  fclose (fptr);

  return cont;
}