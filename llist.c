#include "llist.h"
#include "objtable.h"

SF_API llnode_t *
sf_ll_new (void *val, llnode_t *prev, llnode_t *nxt)
{
  llnode_t *t = sfmalloc (sizeof (*t));
  t->val = val;
  t->prev = prev;
  t->next = nxt;

  t->meta.ref_count = 0;

  return t;
}

SF_API llnode_t *
sf_ll_add_next_r (llnode_t *head, void *val)
{
  if (head->next == NULL)
    {
      head->next = sf_ll_new (val, head, NULL);
      return head->next;
    }

  return sf_ll_add_next_r (head->next, val);
}

SF_API llnode_t *
sf_ll_add_prev_r (llnode_t *head, void *val)
{
  if (head->prev == NULL)
    {
      head->prev = sf_ll_new (val, NULL, head);
      return head->prev;
    }

  return sf_ll_add_prev_r (head->prev, val);
}

SF_API void
sf_ll_unlink_node (llnode_t **child)
{
  llnode_t *c = *child;

  if (c->prev != NULL && c->next != NULL)
    {
      c->prev->next = c->next;
      c->next->prev = c->prev;
    }

  else if (c->prev == NULL && c->next != NULL) // top node
    {
      *child = c->next;
    }

  else if (c->next == NULL && c->prev != NULL)
    {
      c->prev->next = NULL;
    }

  else // both are NULL
    {
      e_printf ("[PENDING] child->prev == child->next == NULL\n");
    }
}

SF_API llnode_t *
sf_ll_insert (llnode_t *top, int idx, void *val)
{
  if (!idx)
    {
      llnode_t *n = sf_ll_new (val, top->prev, top);
      top->prev->next = n;
      top->next->prev = n;

      return n;
    }

  return sf_ll_insert (top->next, --idx, val);
}

SF_API void
sf_ll_reverse (llnode_t **top)
{
  llnode_t *t = *top;
  llnode_t *n = sf_ll_new (t->val, NULL, NULL);
  t = t->next;

  while (t)
    {
      n->prev = sf_ll_new (t->val, NULL, n);
      n = n->prev;
      t = t->next;
    }

  *top = n;
}

SF_API llnode_t *
sf_ll_getnode_fromval (llnode_t *top, void *val, int (*cmrt) (void *, void *))
{
  while (top)
    {
      // printf ("%s %s %d\n", top->val, val, cmrt (top->val, val));
      if (cmrt (top->val, val))
        return top;

      top = top->next;
    }

  return NULL;
}

SF_API void
sf_ll_set_meta_refcount (llnode_t *node, int nval)
{
  // if (node->meta.ref_count > nval && nval)
  //   {
  //     obj_t *p = (obj_t *)node->val;
  //     printf ("%d\n", p->type);

  //     if (p->meta.pa_size)
  //       {
  //         for (size_t i = 0; i < p->meta.pa_size; i++)
  //           {
  //             llnode_t *t = p->meta.passargs[i]->meta.mem_ref;
  //             sf_ll_set_meta_refcount (t, t->meta.ref_count - 1);
  //           }

  //         sffree (p->meta.passargs);
  //         p->meta.passargs = NULL;
  //         p->meta.pa_size = 0;
  //       }
  //   }

  node->meta.ref_count = nval;

  if (node->meta.ref_count < 1)
    {
      obj_t *p = (obj_t *)node->val;

      if (p->meta.pa_size)
        {
          // for (size_t i = 0; i < p->meta.pa_size; i++)
          //   {
          //     llnode_t *t = p->meta.passargs[i]->meta.mem_ref;
          //     sf_ll_set_meta_refcount (t, t->meta.ref_count - 1);
          //   }

          sffree (p->meta.passargs);
          p->meta.passargs = NULL;
          p->meta.pa_size = 0;
        }

      // here;
      sf_ot_removeobj (node);
    }
}