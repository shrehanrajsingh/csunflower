#include "tree.h"

SF_API tree_t *
sf_tree_new (void *val, tree_t *l, tree_t *r)
{
  tree_t *t = sfmalloc (sizeof (*t));
  t->val = val;
  t->left = l;
  t->right = r;

  return t;
}

SF_API void
sf_tree_addleft_leaf (tree_t *t, void *v)
{
  if (t->left == NULL)
    t->left = sf_tree_new (v, NULL, NULL);

  else
    sf_tree_addleft_leaf (t->left, v);
}

SF_API void
sf_tree_addright_leaf (tree_t *t, void *v)
{
  if (t->right == NULL)
    t->right = sf_tree_new (v, NULL, NULL);

  else
    sf_tree_addright_leaf (t->right, v);
}

SF_API void
sf_tree_traverse_pre (tree_t *t, __sftreetraversal_ret_routine r)
{
  r (t->val);

  if (t->left)
    sf_tree_traverse_pre (t->left, r);

  if (t->right)
    sf_tree_traverse_pre (t->right, r);
}

SF_API void
sf_tree_traverse_in (tree_t *t, __sftreetraversal_ret_routine r)
{
  if (t->left)
    sf_tree_traverse_in (t->left, r);

  r (t->val);

  if (t->right)
    sf_tree_traverse_in (t->right, r);
}

SF_API void
sf_tree_traverse_post (tree_t *t, __sftreetraversal_ret_routine r)
{
  if (t->left)
    sf_tree_traverse_post (t->left, r);

  if (t->right)
    sf_tree_traverse_post (t->right, r);

  r (t->val);
}

SF_API int
sf_tree_height (tree_t *t)
{
  if (t == NULL)
    return 0;

  return max (sf_tree_height (t->left), sf_tree_height (t->right)) + 1;
}

SF_API int
sf_tree_countNodes (tree_t *t)
{
  if (t == NULL)
    return 0;

  return sf_tree_countNodes (t->left) + sf_tree_countNodes (t->right) + 1;
}

struct _hgt
{
  int height;
  tree_t *node;
};

// sf tree traverse levelord sub routine
void
_sttll_sr (tree_t *n, int h, struct _hgt *arr, int *ac)
{
  if (n == NULL)
    return;
  //   arr = sfrealloc (arr, ((*ac) + 1) * sizeof (*arr));
  arr[(*ac)++] = (struct _hgt){ .height = h, .node = n };

  _sttll_sr (n->left, h + 1, arr, ac);
  _sttll_sr (n->right, h + 1, arr, ac);
}

SF_API void
sf_tree_traverse_levelord (tree_t *t, __sftreetraversal_ret_routine r)
{
  // 2^n - 1 maximum nodes
  int h = sf_tree_height (t);
  int cn = sf_tree_countNodes (t);

  struct _hgt *ar = sfmalloc (cn * sizeof (*ar));
  int ars = 0;
  _sttll_sr (t, 0, ar, &ars);

  for (size_t i = 0; i < h; i++)
    {
      for (size_t j = 0; j < ars; j++)
        {
          struct _hgt *cur = &ar[j];
          // printf ("%d, ", cur->height);

          if (cur->height == i)
            {
              //   printf ("height (%d) ", cur->height);
              r (cur->node->val);
            }
        }
      //   printf ("\n");
    }

  sffree (ar);
}

SF_API void
sf_tree_free (tree_t *t)
{
  if (t->left)
    sf_tree_free (t->left);

  if (t->right)
    sf_tree_free (t->right);

  sffree (t);
}