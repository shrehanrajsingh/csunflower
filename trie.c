#include "trie.h"

SF_API trie_t *
sf_trie_new (void)
{
  trie_t *t = sfmalloc (sizeof (*t));

  t->isval = 0;
  memset (t->key, 0, TRIE_KEYMAXSIZE * sizeof (char));
  t->kl = 0;
  t->nodes = NULL;
  t->val = NULL;
  t->meta.i = 0;

  return t;
}

SF_API void
sf_trie_makeVal (trie_t *t, void *v)
{
  t->isval = 1;
  t->val = v;
}

SF_API void
sf_trie_add (trie_t *t, char *k, void *v)
{
  if (t == NULL)
    {
      e_printf ("Trie is null in sf_trie_add()\n");
      return;
    }

  if (*k == '\0')
    {
      sf_trie_makeVal (t, v);
      return;
    }

  int n_idx = -1;

  for (size_t i = 0; i < t->kl; i++)
    {
      if (t->key[i] == *k)
        {
          n_idx = i;
          break;
        }
    }

  if (n_idx == -1) // add new node
    {
      t->key[t->kl] = *k;
      t->nodes = sfrealloc (t->nodes, (t->kl + 1) * sizeof (*t->nodes));
      t->nodes[t->kl] = sf_trie_new ();
      t->kl++;

      sf_trie_add (t->nodes[t->kl - 1], k + 1, v);
    }
  else
    {
      sf_trie_add (t->nodes[n_idx], k + 1, v);
    }
}

char **
_sftgk (trie_t *t, char *k)
{
  char **res = NULL;
  size_t rc = 0;

  if (t->isval)
    {
      res = sfrealloc (res, (rc + 1) * sizeof (*res));
      res[rc++] = sfstrdup (k);
    }

  for (size_t i = 0; i < t->kl; i++)
    {
      char *mk = NULL;

      if (k == NULL)
        {
          mk = sfmalloc (2 * sizeof (*mk));
          mk[0] = t->key[i];
          mk[1] = '\0';
        }
      else
        {
          size_t kl = strlen (k);

          mk = sfstrdup (k);
          mk = sfrealloc (mk, (kl + 2) * sizeof (*mk));

          mk[kl] = t->key[i];
          mk[kl + 1] = '\0';
        }

      char **rl = _sftgk (t->nodes[i], sfstrdup (mk));
      size_t rlc = 0;

      while (rl[rlc] != NULL)
        rlc++;

      res = sfrealloc (res, (rc + rlc) * sizeof (*res));
      size_t j = 0;

      while (rl[j] != NULL)
        {
          res[rc + j] = rl[j];
          j++;
        }

      rc += j;
      sffree (mk);
    }

  res = sfrealloc (res, (rc + 1) * sizeof (*res));
  res[rc] = NULL;

  return res;
}

SF_API char **
sf_trie_getKeys (trie_t *t)
{
  if (t == NULL)
    return NULL;

  return _sftgk (t, NULL);
}

SF_API void *
sf_trie_getVal (trie_t *t, char *k)
{
  if (t == NULL)
    {
      e_printf ("Trie is null (key does not exist) in sf_trie_getVal()\n");
      return NULL;
    }

  if (*k == '\0')
    {
      if (t->isval)
        return t->val;

      e_printf ("Key '%s' does not exist in trie in sf_trie_getVal()\n", k);
      return NULL;
    }

  int n_idx = -1;
  for (size_t i = 0; i < t->kl; i++)
    {
      if (t->key[i] == *k)
        {
          n_idx = i;
          break;
        }
    }

  if (n_idx == -1)
    {
      e_printf ("Key '%s' does not exist in trie in sf_trie_getVal()\n", k);
      return NULL;
    }

  return sf_trie_getVal (t->nodes[n_idx], k + 1);
}

SF_API void
sf_trie_free (trie_t *t)
{
  if (t == NULL)
    return;

  for (size_t i = 0; i < t->kl; i++)
    {
      sf_trie_free (t->nodes[i]);
    }

  sffree (t);
}