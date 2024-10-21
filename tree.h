#pragma once

#include "header.h"
#include "sfmem.h"
#include "sfstr.h"

struct _sf_treenode_s
{
  void *val;

  struct _sf_treenode_s *left;
  struct _sf_treenode_s *right;
};

typedef struct _sf_treenode_s tree_t;
typedef void (*__sftreetraversal_ret_routine) (void *);

#ifdef __cplusplus
extern "C"
{
#endif

  SF_API tree_t *sf_tree_new (void *_Val, tree_t *_Left, tree_t *_Right);
  SF_API tree_t *sf_tree_copy_deep (tree_t *,
                                    void *(_ValCopy_Routine)(void *));

  SF_API void sf_tree_addleft_leaf (tree_t *_Tree, void *_Val);

  SF_API void sf_tree_addright_leaf (tree_t *_Tree, void *_Val);

  SF_API void sf_tree_traverse_pre (tree_t *_Tree,
                                    __sftreetraversal_ret_routine _Callback);

  SF_API void sf_tree_traverse_in (tree_t *_Tree,
                                   __sftreetraversal_ret_routine _Callback);

  SF_API void sf_tree_traverse_post (tree_t *_Tree,
                                     __sftreetraversal_ret_routine _Callback);

  SF_API void
  sf_tree_traverse_levelord (tree_t *_Tree,
                             __sftreetraversal_ret_routine _Callback);

  SF_API int sf_tree_height (tree_t *_Tree);
  SF_API int sf_tree_countNodes (tree_t *_Tree);

  SF_API void sf_tree_free (tree_t *_Tree, void (_ValFree_Routine) (void *));

#ifdef __cplusplus
}
#endif
