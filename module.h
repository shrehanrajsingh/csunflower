#pragma once

#include "header.h"
#include "llist.h"
#include "sfmem.h"
#include "sfstr.h"

struct _mod_s;

enum
{
  SF_MD_NATIVE = 0,
  SF_MD_CODED = 1,
};

struct _sf_module_s
{
  int type;
  struct _mod_s *mod;
  char *path;
  char *alias;

  struct
  {
    int has_id;
    int id;
  } meta;
};

typedef struct _sf_module_s sfmodule_t;

#define SFMODULE_CAPACITY 32
#define SFMODULE_CACHE_SIZE 32

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API void sf_module_init (void);

  SF_API sfmodule_t *sf_module_new (int _Mod_Type, struct _mod_s *_Mod);

  SF_API void sf_module_set_path (sfmodule_t *, char *);  // uses strdup
  SF_API void sf_module_set_alias (sfmodule_t *, char *); // uses strdup

  SF_API sfmodule_t *sf_module_add (sfmodule_t *);
  SF_API void sf_module_addlookuppath (const char *);

  SF_API void sf_module_dest (void);

  SF_API void sf_module_setparent (struct _mod_s *);
  SF_API struct _mod_s **sf_module_getparent (void);

  SF_API sf_charptr sf_module_getfilecontents (sf_charptr);

#ifdef __cplusplus
}
#endif
