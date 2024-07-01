#pragma once

#include "header.h"
#include "llist.h"
#include "sfmem.h"
#include "sfstr.h"

struct _mod_s;

typedef llnode_t *(*fnret_t) (struct _mod_s *);

#define SF_FUN_ARG_LIMIT 256

enum
{
  SF_FUN_NATIVE = 0,
  SF_FUN_CODED = 1,
};

struct _sf_fun_s
{
  char *name;
  int type;
  struct _mod_s *mod;

  char **args;
  size_t argc;

  struct
  {
    fnret_t routine;

  } native;

  struct
  {
    size_t id;
    int has_id;

  } meta;

  // .mod can store function body itself
  // no struct required for coded functions
};

typedef struct _sf_fun_s fun_t;

#define SF_FUN_CACHE_SIZE 32

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Initializes the sunflower function.
   *
   * This function initializes the sunflower function utility by performing any
   * necessary setup or initialization steps.
   */
  SF_API void sf_fun_init (void);

  /**
   * Creates a new function object.
   *
   * @param _Name Name of function
   * @param _Type The type of the function.
   * @param _Mod The module associated with the function.
   * @param _RoutineIfNative The native routine if the function is
   * native. (pass null if function is coded)
   * @return The newly created function object.
   */
  SF_API fun_t *sf_fun_new (char *_Name, int _Type, struct _mod_s *_Mod,
                            void *_RoutineIfNative);

  SF_API void sf_fun_addarg (fun_t *_Fun, char *_ArgName);

  /**
   * Adds a function to the sunflower library.
   *
   * @param _Fun The function to be added.
   * @return A pointer to the added function.
   */
  SF_API fun_t *sf_fun_add (fun_t *_Fun);

  SF_API fun_t ***sf_fun_getStack (void);

  SF_API void sf_fun_free (fun_t *_Fun);

#ifdef __cplusplus
}
#endif
