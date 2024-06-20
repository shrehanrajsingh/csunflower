#pragma once

#include "header.h"
#include "sfmem.h"

#define TRIE_KEYMAXSIZE 64 // [a-zA-Z0-9]\b(_)

/**
 * @struct _trienode_s
 * @brief Represents a node in a trie data structure.
 *
 * This struct defines the properties of a trie node, including the key, nodes,
 * value, and metadata.
 */
struct _trienode_s
{
  char key[TRIE_KEYMAXSIZE]; /**< The key associated with the node. */
  size_t kl;                 /**< The length of the key. */

  struct _trienode_s **nodes; /**< An array of child nodes. */

  int isval; /**< Indicates whether the node represents a value. */
  void *val; /**< The value associated with the node. */

  struct // for val
  {
    int i; /**< Additional metadata for the value. */
  } meta;
};

typedef struct _trienode_s trie_t;

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Creates a new trie.
   *
   * @return A pointer to the newly created trie.
   */
  SF_API trie_t *sf_trie_new (void);

  /**
   * Sets the value of a trie node.
   *
   * This function is used to set the value of a trie node in the given trie.
   *
   * @param _Trie The trie structure.
   * @param _Val  The value to be set.
   */
  SF_API void sf_trie_makeVal (trie_t *_Trie, void *_Val);

  /**
   * Adds a key-value pair to the trie.
   *
   * @param _Trie The trie to add the key-value pair to.
   * @param _Key The key to add.
   * @param _Val The value to associate with the key.
   */
  SF_API void sf_trie_add (trie_t *_Trie, char *_Key, void *_Val);

  /**
   * Retrieves an array of keys from the trie.
   *
   * @param _Trie The trie from which to retrieve the keys.
   * @return An array of strings representing the keys in the trie.
   */
  SF_API char **sf_trie_getKeys (trie_t *_Trie);

  /**
   * Retrieves the value associated with a given key in the trie.
   *
   * @param _Trie The trie data structure.
   * @param _Key The key to search for.
   * @return A pointer to the value associated with the key, or NULL if the key
   * is not found.
   */
  SF_API void *sf_trie_getVal (trie_t *_Trie, char *_Key);

  /**
   * @brief Frees the memory allocated for a trie.
   *
   * This function frees the memory allocated for a trie and all its associated
   * nodes.
   *
   * @param _Trie The trie to be freed.
   */
  SF_API void sf_trie_free (trie_t *_Trie);

#ifdef __cplusplus
}
#endif
