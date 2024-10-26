#include "objtable.h"

llnode_t *SFOBJ_MEM;

SF_API void
sf_ot_init (void)
{
  SFOBJ_MEM = sf_ll_new (NULL, NULL, NULL);
}

SF_API llnode_t *
sf_ot_addobj (obj_t *obj)
{
  // TEST
  if (obj->meta.mem_ref == NULL)
    {
      llnode_t *t = sf_ll_add_next_r (SFOBJ_MEM, (void *)obj);
      obj->meta.mem_ref = t;

      return t;
    }

  return obj->meta.mem_ref;
}

SF_API void
sf_ot_removeobj (llnode_t *node)
{
  obj_t *p = (obj_t *)node->val;
  p->meta.mem_ref = node;
  sf_ast_freeObj (&p);

  sf_ll_unlink_node (&node);

  sffree (node);
}