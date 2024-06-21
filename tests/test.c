#include <sunflower.h>

#define TEST(X) test##X ()

char *
readfile (const char *fname)
{
  FILE *fptr = fopen (fname, "r");
  sf_charptr res = sf_str_new_empty ();
  char c;

  while ((c = fgetc (fptr)) != EOF)
    {
      sf_str_pushchr (&res, c);
    }

  fclose (fptr);
  return res;
}

void
test1 ()
{
  trie_t *t = sf_trie_new ();

  sf_trie_add (t, "Hello", NULL);
  sf_trie_add (t, "hello", NULL);
  sf_trie_add (t, "test", NULL);
  sf_trie_add (t, "void", NULL);
  sf_trie_add (t, "abc013292__19", NULL);

  char **r = sf_trie_getKeys (t);
  while (*r != NULL)
    {
      printf ("%s\n", *r);
      r++;
    }
}

void
test2 ()
{
  char *cont = readfile ("../../tests/test.sf");

  tok_t *t = sf_tokenizer_gen (cont);
  while (t->type != TOK_EOF)
    {
      sf_tokenizer_print (*t);
      t++;
    }
}

void
test3 ()
{
  char *cont = readfile ("../../tests/test.sf");

  tok_t *t = sf_tokenizer_gen (cont);
  size_t sptr = 0;
  stmt_t *st = sf_ast_stmtgen (t, &sptr);

  for (size_t i = 0; i < sptr; i++)
    {
      sf_ast_stmtprint (st[i]);
    }
}

int
cmrt (void *v1, void *v2)
{
  return !strcmp (v1, v2);
}

void
test4 ()
{
  llnode_t *t = sf_ll_new ("First node", NULL, NULL);

  sf_ll_add_next_r (t, "Second node");
  sf_ll_add_next_r (t, "Third node");
  sf_ll_add_next_r (t, "Fourth node");
  sf_ll_add_next_r (t, "Fifth node");

  llnode_t *temp = t;
  llnode_t *last_node = NULL;

  printf ("print\n");

  while (temp)
    {
      if (temp->next == NULL)
        last_node = temp;
      printf ("%s\n", temp->val);
      temp = temp->next;
    }

  printf ("-----\n");

  // Traverse back
  printf ("traverse back\n");
  while (last_node)
    {
      printf ("%s\n", last_node->val);
      last_node = last_node->prev;
    }

  printf ("-----\n");

  // Insert
  printf ("insert\n");
  sf_ll_insert (t, 2, "Sixth node");
  temp = t;

  while (temp)
    {
      if (temp->next == NULL)
        last_node = temp;
      printf ("%s\n", temp->val);
      temp = temp->next;
    }

  printf ("-----\n");

  // Unlink
  printf ("unlink\n");
  sf_ll_unlink_node (&t->next->next);

  temp = t;

  while (temp)
    {
      if (temp->next == NULL)
        last_node = temp;
      printf ("%s\n", temp->val);
      temp = temp->next;
    }

  printf ("-----\n");

  // Reverse
  printf ("reverse\n");
  sf_ll_reverse (&t);

  temp = t;

  while (temp)
    {
      if (temp->next == NULL)
        last_node = temp;
      printf ("%s\n", temp->val);
      temp = temp->next;
    }

  printf ("-----\n");

  // Re-reverse
  printf ("re-reverse\n");
  sf_ll_reverse (&t);

  temp = t;

  while (temp)
    {
      if (temp->next == NULL)
        last_node = temp;
      printf ("%s\n", temp->val);
      temp = temp->next;
    }

  printf ("-----\n");
  // Get val
  printf ("getval\n");

  llnode_t *v = sf_ll_getnode_fromval (t, "Fourth node", cmrt);
  // printf ("%d\n", v == NULL);

  while (v)
    {
      printf ("%s\n", v->val);
      v = v->next;
    }
}

void
test5 ()
{
  char *cont = readfile ("../../tests/test.sf");

  tok_t *t = sf_tokenizer_gen (cont);
  size_t sptr = 0;
  stmt_t *st = sf_ast_stmtgen (t, &sptr);

  mod_t *m = sf_mod_new (MOD_TYPE_FILE, NULL);
  m->body = st;
  m->body_len = sptr;

  sf_parser_exec (m);

  char **k = sf_trie_getKeys (m->vtable);

  for (size_t i = 0; k[i] != NULL; i++)
    {
      printf ("%s\n", k[i]);
    }
}

int
main (int argc, char const *argv[])
{
  sf_ot_init ();
  TEST (5);

  return printf ("Program ended.\n") && 0;
}