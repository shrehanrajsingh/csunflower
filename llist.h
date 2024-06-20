#pragma once

#include "header.h"
#include "sfmem.h"

/**
 * @struct _llnode_s
 * @brief Represents a node in a linked list.
 *
 * This struct defines a node in a linked list. Each node contains a value
 * and pointers to the next and previous nodes in the list.
 */
struct _llnode_s
{
  void *val;              /**< Pointer to the value stored in the node. */
  struct _llnode_s *next; /**< Pointer to the next node in the list. */
  struct _llnode_s *prev; /**< Pointer to the previous node in the list. */

  /**
   * @struct meta
   * @brief Structure representing metadata for the linked list.
   *
   * This structure contains the reference count for the linked list.
   */
  struct
  {
    int ref_count;
  } meta;
};

typedef struct _llnode_s llnode_t;

struct _obj_s;

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Creates a new linked list node.
   *
   * @param _Val The value to be stored in the node.
   * @param _Prev The previous node in the linked list.
   * @param _Next The next node in the linked list.
   * @return A pointer to the newly created linked list node.
   */
  SF_API llnode_t *sf_ll_new (void *_Val, llnode_t *_Prev, llnode_t *_Next);

  /**
   * Adds a new node with the specified value after the given head node
   * recursively.
   *
   * @param _Head The head node of the linked list.
   * @param _Val The value to be added to the linked list.
   * @return A pointer to the newly added linked list node.
   */
  SF_API llnode_t *sf_ll_add_next_r (llnode_t *_Head, void *_Val);

  /**
   * Adds a new node with the specified value before the given head node
   * recursively
   *
   * @param _Head The head node of the linked list.
   * @param _Val The value to be added to the linked list.
   * @return A pointer to the newly added linked list node.
   */
  SF_API llnode_t *sf_ll_add_prev_r (llnode_t *_Head, void *_Val);

  /**
   * Unlinks the specified node from its root in the linked list.
   *
   * This function unlinks the given node, `_Child`, from its root in the
   * linked list. After unlinking, the node will no longer be part of the
   * linked list structure.
   *
   * @param _Child The node to be unlinked.
   */
  SF_API void sf_ll_unlink_node (llnode_t **_Child);

  /**
   * Inserts a new node with the given value at the specified index in the
   * linked list.
   *
   * @param _Top The pointer to the top node of the linked list.
   * @param _Idx The index at which the new node should be inserted.
   * @param _Val The value to be stored in the new node.
   * @return A pointer to the newly added linked list node.
   */
  SF_API llnode_t *sf_ll_insert (llnode_t *_Top, int _Idx, void *_Val);

  /**
   * Reverses the order of the linked list.
   *
   * This function reverses the order of the linked list starting from the
   * given top node.
   *
   * @param _Top The top node of the linked list.
   */
  SF_API void sf_ll_reverse (llnode_t **_Top);

  /**
   * @brief Specifies the API visibility for the function.
   *
   * The SF_API macro is used to specify the visibility of the function. It is
   * typically used to indicate that the function is part of the public API of
   * a library or module.
   *
   * @param _Top The top node of the linked list.
   * @param _Val The value to search for in the linked list.
   * @param _CompareRt A function pointer to the comparison function used to
   * compare values.
   * @return A pointer to the node containing the specified value, or NULL if
   * not found.
   */
  SF_API llnode_t *sf_ll_getnode_fromval (llnode_t *_Top, void *_Val,
                                          int (*_CompareRt) (void *, void *));

  SF_API struct _obj_s *sf_ll_set_meta_refcount (llnode_t *_Node, int _NewVal);

#ifdef __cplusplus
}
#endif
