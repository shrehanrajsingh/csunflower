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
main (int argc, char const *argv[])
{
  TEST (3);

  return printf ("Program ended.\n") && 0;
}