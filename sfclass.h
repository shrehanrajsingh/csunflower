#pragma once

#include "header.h"
#include "llist.h"
#include "sfmem.h"
#include "sfstr.h"

struct _mod_s;

struct _sf_class_s
{
  char *name;
  int type;
  struct _mod_s *mod;

  llnode_t **inh_list;
  size_t il_c;
};

typedef struct _sf_class_s class_t;

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif
