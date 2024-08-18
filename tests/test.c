#include <sunflower.h>

#define TEST(X) test##X ()

char *
readfile (const char *fname)
{
  FILE *fptr = fopen (fname, "r");
  assert (fptr != NULL);

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

  for (size_t i = 0; r[i] != NULL; i++)
    {
      printf ("%s\n", r[i]);
      sffree (r[i]);
    }

  sffree (r);

  sf_trie_free (t);
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

llnode_t *
put_fun_rt (mod_t *mod)
{
  obj_t *arg = (obj_t *)sf_mod_getVar (mod, "arg")->val;

  char *p;
  int c = 0;
  c += printf ("%s", p = sf_parser_objRepr (mod, arg));

  sffree (p);

  obj_t *r = sf_ast_objnew (OBJ_CONST);
  r->v.o_const.type = CONST_INT;
  r->v.o_const.v.c_int.v = c;

  return sf_ot_addobj (r);
}

llnode_t *
putln_fun_rt (mod_t *mod)
{
  llnode_t *r = put_fun_rt (mod);
  putchar ('\n');

  return r;
}

llnode_t *
input_fun_rt (mod_t *mod)
{
  obj_t *msg = (obj_t *)sf_mod_getVar (mod, "msg")->val;

  char *p;
  printf ("%s", p = sf_parser_objRepr (mod, msg));

  sffree (p);

  sf_charptr inp = sf_str_new_empty ();
  char c;

  while ((c = getchar ()) != '\n')
    {
      sf_str_pushchr (&inp, c);
    }

  obj_t *r = sf_ast_objnew (OBJ_CONST);
  r->v.o_const.type = CONST_STRING;
  r->v.o_const.v.c_string.v = inp;

  return sf_ot_addobj (r);
}

llnode_t *
ctr_fun_rt (mod_t *mod)
{
  static int c = 0;

  obj_t *r = sf_ast_objnew (OBJ_CONST);
  r->v.o_const.type = CONST_INT;
  r->v.o_const.v.c_int.v = c++;

  return sf_ot_addobj (r);
}

llnode_t *
nativemethod_type_str_name_operator_plus (mod_t *mod)
{
  obj_t *self = (obj_t *)sf_mod_getVar (mod, "self")->val;
  obj_t *other = (obj_t *)sf_mod_getVar (mod, "other")->val;

  char *p1, *p2;
  p1 = sf_parser_objRepr (mod, self);
  p2 = sf_parser_objRepr (mod, other);

  sf_charptr res = sf_str_new_empty ();
  sf_str_push (&res, p1);
  sf_str_push (&res, p2);

  obj_t *o = sf_ast_objnew (OBJ_CONST);
  o->v.o_const.type = CONST_STRING;
  o->v.o_const.v.c_string.v = res;

  sffree (p1);
  sffree (p2);

  return sf_ot_addobj (o);
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

  /********************************************* */
  /* put(arg) function */

  {
    mod_t *put_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);
    obj_t *arg_obj = sf_ast_objnew (OBJ_CONST);

    arg_obj->type = OBJ_CONST;
    arg_obj->v.o_const.type = CONST_STRING;
    arg_obj->v.o_const.v.c_string.v = sf_str_new_fromStr ("\n");

    sf_mod_addVar (put_mod, "arg", sf_ot_addobj (arg_obj));

    fun_t *put_fun = sf_fun_new ("put", SF_FUN_NATIVE, put_mod, put_fun_rt);

    sf_fun_addarg (put_fun, "arg");

    fun_t *pf = sf_fun_add (put_fun);

    obj_t *ar_obj = sf_ast_objnew (OBJ_FUN);
    ar_obj->type = OBJ_FUN;
    ar_obj->v.o_fun.f = pf;

    sf_mod_addVar (m, "put", sf_ot_addobj (ar_obj));
  }

  /********************************************* */

  /********************************************* */
  /* putln(arg) function */

  {
    mod_t *putln_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);
    obj_t *arg_obj = sf_ast_objnew (OBJ_CONST);

    arg_obj->v.o_const.type = CONST_STRING;
    arg_obj->v.o_const.v.c_string.v = sf_str_new_fromStr ("\n");

    sf_mod_addVar (putln_mod, "arg", sf_ot_addobj (arg_obj));

    fun_t *putln_fun
        = sf_fun_new ("putln", SF_FUN_NATIVE, putln_mod, putln_fun_rt);

    sf_fun_addarg (putln_fun, "arg");

    fun_t *pf = sf_fun_add (putln_fun);

    obj_t *plr_obj = sf_ast_objnew (OBJ_FUN);
    plr_obj->v.o_fun.f = pf;

    sf_mod_addVar (m, "putln", sf_ot_addobj (plr_obj));
  }

  /********************************************* */

  /********************************************* */
  /* input(arg) function */

  {
    mod_t *inp_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);
    obj_t *msg_obj = sf_ast_objnew (OBJ_CONST);

    msg_obj->v.o_const.type = CONST_STRING;
    msg_obj->v.o_const.v.c_string.v = sf_str_new_fromStr ("");

    sf_mod_addVar (inp_mod, "msg", sf_ot_addobj (msg_obj));

    fun_t *inp_fun
        = sf_fun_new ("input", SF_FUN_NATIVE, inp_mod, input_fun_rt);

    sf_fun_addarg (inp_fun, "msg");

    fun_t *ifp = sf_fun_add (inp_fun);

    obj_t *ir_obj = sf_ast_objnew (OBJ_FUN);
    ir_obj->v.o_fun.f = ifp;

    sf_mod_addVar (m, "input", sf_ot_addobj (ir_obj));
  }

  /********************************************* */

  /********************************************* */
  /* ctr() function */

  {
    mod_t *ctr_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);

    fun_t *ctr_fun = sf_fun_new ("ctr", SF_FUN_NATIVE, ctr_mod, ctr_fun_rt);

    fun_t *ctrp = sf_fun_add (ctr_fun);

    obj_t *ctr_obj = sf_ast_objnew (OBJ_FUN);
    ctr_obj->type = OBJ_FUN;
    ctr_obj->v.o_fun.f = ctrp;

    sf_mod_addVar (m, "ctr", sf_ot_addobj (ctr_obj));
  }

  /********************************************* */

  /********************************************* */
  /* ''.operator+() function */

  {
    mod_t *sop_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);

    fun_t *sop_fun = sf_fun_new (SF_OP_OVRLD_PLUS, SF_FUN_NATIVE, sop_mod,
                                 nativemethod_type_str_name_operator_plus);

    sf_fun_addarg (sop_fun, "self");
    sf_fun_addarg (sop_fun, "other");

    fun_t *sopf = sf_fun_add (sop_fun);

    sf_nm_add (SF_OP_OVRLD_PLUS, CONST_STRING, sopf);
  }

  /********************************************* */

  // while (1)
  {
    sf_parser_exec (m);
  }

  char **k = sf_trie_getKeys (m->vtable);

  // for (size_t i = 0; k[i] != NULL; i++)
  //   {
  //     printf ("%s\n", k[i]);
  //   }

  // array_t **arrstack = sf_array_getStack ();

  // for (size_t i = 0; i < 2; i++)
  //   {
  //     printf ("%d\n", (*arrstack)[i].len);
  //   }
}

void
t6btr (void *arg)
{
  printf ("%s\n", (char *)arg);
}

void
test6 ()
{
  tree_t *t = sf_tree_new ("Root", NULL, NULL);

  sf_tree_addleft_leaf (t, "Level 1(l)");
  sf_tree_addleft_leaf (t, "Level 2(ll)");
  sf_tree_addleft_leaf (t, "Level 3(lll)");
  sf_tree_addleft_leaf (t, "Level 4(llll)");
  sf_tree_addleft_leaf (t, "Level 5(lllll)");

  sf_tree_addright_leaf (t, "Level 1(r)");
  sf_tree_addright_leaf (t, "Level 2(rr)");
  sf_tree_addright_leaf (t, "Level 3(rrr)");
  sf_tree_addright_leaf (t, "Level 4(rrrr)");

  sf_tree_addright_leaf (t->left, "Level 2(lr)");
  sf_tree_addleft_leaf (t->left->right, "Level 3(lrl)");
  sf_tree_addleft_leaf (t->right, "Level 2(rl)");

  printf ("PREORDER: \n");
  sf_tree_traverse_pre (t, t6btr);

  printf ("INORDER: \n");
  sf_tree_traverse_in (t, t6btr);

  printf ("POSTORDER: \n");
  sf_tree_traverse_post (t, t6btr);

  printf ("LEVELORDER: \n");
  sf_tree_traverse_levelord (t, t6btr);

  sf_tree_free (t);
}

void
test7 ()
{
  sf_charptr p = sf_str_new_fromSize (10);

  for (size_t i = 0; i < 10; i++)
    {
      p[i] = i + '0';
    }

  p[19] = '\0';

  printf ("%s\n", p);

  sf_str_insert (&p, 3, 'P');
  printf ("%s\n", p);
}

int
main (int argc, char const *argv[])
{
  // SF_DEBUG_DUMP = fopen ("../../tests/dgb.sf", "w");

  sf_ot_init ();
  sf_fun_init ();
  sf_array_init ();
  sf_dbg_fledump_init ();
  sf_class_init ();
  sf_parser_init ();
  sf_nm_init ();

  // while (1)

  // Code 2: Token print
  // Code 3: dump ast tree
  // Code 4: linked list test
  // Code 5: parser
  // Code 6: tree test
  TEST (5);

  // fclose (SF_DEBUG_DUMP);
  sf_dbg_dumpclose ();

  return !printf ("Program ended.\n");
}