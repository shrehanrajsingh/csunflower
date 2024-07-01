#pragma once

#include "ast.h"
#include "header.h"
#include "llist.h"
#include "objtable.h"
#include "sfmem.h"
#include "sfstr.h"
#include "trie.h"

enum
{
  MOD_TYPE_FILE = 0,
  MOD_TYPE_INTERACTIVE = 1,
  MOD_TYPE_FUNC = 2,
  MOD_TYPE_CLASS = 3,
};

/**
 * @brief Structure representing a mod.
 *
 * This structure defines the properties of a mod, which is a module or
 * function in the Sunflower project. It contains information such as the mod
 * type, variable table, body, return value, and parent mod.
 */
struct _mod_s
{
  int type;       // type of the mod
  trie_t *vtable; // variable table

  stmt_t *body;    // body of the mod
  size_t body_len; // length of the body

  llnode_t *retv; // return value of the mod (return value of a function)
  struct _mod_s *parent; // parent mod
};

typedef struct _mod_s mod_t;

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Creates a new mod_t object.
   *
   * @param _Type The type of the mod_t object.
   * @param _Parent The parent mod_t object.
   * @return A pointer to the newly created mod_t object.
   */
  SF_API mod_t *sf_mod_new (int _Type, mod_t *_Parent);

  /**
   * Adds a variable to the given module.
   *
   * @param _Mod The module to add the variable to.
   * @param _Name The name of the variable.
   * @param _Ref The reference to the memory node of the object associated with
   * the variable.
   * @return A pointer to the newly added llnode_t structure.
   */
  SF_API llnode_t *sf_mod_addVar (mod_t *_Mod, char *_Name, llnode_t *_Ref);

  /**
   * Retrieves a variable from a module.
   *
   * This function searches for a variable with the given name in the specified
   * module.
   *
   * @param _Mod The module from which to retrieve the variable.
   * @param _Name The name of the variable to retrieve.
   * @return A pointer to the `llnode_t` structure representing the variable,
   * or `NULL` if the variable is not found.
   */
  SF_API llnode_t *sf_mod_getVar (mod_t *_Mod, const char *_Name);

  /**
   * Frees the memory allocated for a mod_t object.
   *
   * @param _Mod The mod_t object to be freed.
   */
  SF_API void sf_mod_free (mod_t *_Mod);

#ifdef __cplusplus
}
#endif
