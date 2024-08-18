#pragma once

#include "header.h"
#include "sfmod.h"
#include "objtable.h"
#include "sfarray.h"
#include "sfstr.h"
#include "sfmem.h"
#include "function.h"
#include "trie.h"

/**
 * Native method names are stored in a trie.
 * The value of the trie is an array of structures.
 * Each structure has two fields:
 * - type: Indicates which constant type has a method of that name
 *         (multiple types can have the same native method)
 * - ref: A reference to the function (make sure it is stored in sf functions)
 */

struct _nativemethod_val_s
{
  int type; /* value of OBJ_<TYPE> enum */
  fun_t *ref;
};

typedef struct _nativemethod_val_s nmv_t;

struct _nmv_bag_s
{
  nmv_t *val;
  int size;
};

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * \brief Initializes the native method stack.
     *
     * This function sets up the necessary data structures for managing
     * native method names and their associated function references.
     * It should be called before any other native method operations.
     */
    SF_API void sf_nm_init ();

    /**
     * \brief Adds a native method to the stack.
     *
     * This function inserts a native method into the stack with the specified name,
     * type, and function reference. It is used to register native methods so that
     * they can be called by their names.
     *
     * \param name The name of the native method.
     * \param type The type of the native method (value of OBJ_<TYPE> enum).
     * \param ref A reference to the function to be called.
     */
    SF_API void sf_nm_add (const char *name, int type, fun_t *ref);

    /**
     * \brief Retrieves a native method from the stack.
     *
     * This function searches for a native method in the stack by its name and type.
     * If found, it returns a reference to the function associated with the native method.
     *
     * \param name The name of the native method.
     * \param type The type of the native method (value of OBJ_<TYPE> enum).
     * \return A reference to the function if found, otherwise NULL.
     */
    SF_API fun_t *sf_nm_get (const char *name, int type);

#ifdef __cplusplus
}
#endif
