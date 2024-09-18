#pragma once

#include "header.h"
#include "llist.h"
#include "sfmem.h"
#include "sfstr.h"

struct _mod_s;

enum
{
  SF_CLASS_NATIVE = 0,
  SF_CLASS_CODED = 1,
};

struct _sf_class_s
{
  char *name;
  int type;
  struct _mod_s *mod;

  llnode_t **inh_list;
  size_t il_c;

  struct
  {
    int id;
    int hasid;

    int iscobj;
    struct _sf_class_s *clref;

    int kill_fun_called;

  } meta;
};

typedef struct _sf_class_s class_t;

#define SF_CLASSSTACK_CAPACITY 32
#define SF_CLASSCACHE_SIZE 32

#define SF_CLASSCONSTRUCTOR_NAME ("_init")

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API void sf_class_init (void);

  SF_API class_t *sf_class_new (const char *_Name, int _Type);

  SF_API class_t *sf_class_add (class_t *);

  SF_API void sf_class_free (class_t *);

#ifdef __cplusplus
}
#endif
