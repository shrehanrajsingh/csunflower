#include "sfclass.h"
#include "sfmod.h"

class_t **CLASS_STACK;
size_t classstack_count;
size_t classstack_capacity;

int CLASS_CACHE[SF_CLASSCACHE_SIZE];
int cch_count;

SF_API void
sf_class_init (void)
{
  classstack_count = 0;
  classstack_capacity = SF_CLASSSTACK_CAPACITY;

  CLASS_STACK = sfmalloc (classstack_capacity * sizeof (*CLASS_STACK));
  cch_count = 0;
}

void
_clstack_resz (void)
{
  classstack_capacity += SF_CLASSSTACK_CAPACITY;
  CLASS_STACK
      = sfrealloc (CLASS_STACK, classstack_capacity * sizeof (*CLASS_STACK));
}

SF_API class_t *
sf_class_new (const char *_Name, int _Type)
{
  class_t *t = sfmalloc (sizeof (*t));

  t->il_c = 0;
  t->inh_list = NULL;
  t->meta.hasid = 0;
  t->meta.id = -1;
  t->mod = NULL;
  t->name = sfstrdup ((char *)_Name);
  t->type = _Type;
  t->meta.iscobj = 0;
  t->meta.clref = NULL;

  return t;
}

SF_API class_t *
sf_class_add (class_t *t)
{
  if (cch_count)
    {
      int p = CLASS_CACHE[--cch_count];

      t->meta.hasid = 1;
      t->meta.id = p;

      CLASS_STACK[p] = t;
    }
  else
    {
      if (classstack_count == classstack_capacity)
        _clstack_resz ();

      t->meta.hasid = 1;
      t->meta.id = classstack_count;

      CLASS_STACK[classstack_count++] = t;
    }

  return t;
}

SF_API void
sf_class_free (class_t *c)
{
  if (c->meta.hasid && cch_count < SF_CLASSCACHE_SIZE)
    {
      CLASS_CACHE[cch_count++] = c->meta.id;
    }

  if (c->meta.iscobj)
  {
    // ? Call destructor
    // TODO
  }

  sffree (c->name);

  // TODO

  if (c->mod)
    sf_mod_free (c->mod);

  sffree (c);
}