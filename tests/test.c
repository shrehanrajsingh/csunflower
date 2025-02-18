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
ord_fun_rt (mod_t *mod)
{
  obj_t *s = (obj_t *)sf_mod_getVar (mod, "str")->val;

  assert (s->type == OBJ_CONST && s->v.o_const.type == CONST_STRING);
  const char *p = SFCPTR_TOSTR (s->v.o_const.v.c_string.v);

  assert (strlen (p) == 1);
  char c = *p;

  obj_t *r = sf_ast_objnew (OBJ_CONST);
  r->v.o_const.type = CONST_INT;
  r->v.o_const.v.c_int.v = (int)c;

  return sf_ot_addobj (r);
}

llnode_t *
super_fun_rt (mod_t *mod)
{
  obj_t *a1 = (obj_t *)sf_mod_getVar (mod, "inst")->val;
  obj_t *a2 = (obj_t *)sf_mod_getVar (mod, "cl")->val;

  assert (a1->type == OBJ_CLASSOBJ
          && "argument 1 of super () is not a class instance.");

  assert (a2->type == OBJ_CLASS
          && "argument 2 of super () is not a class object");

  class_t *a1_c = a1->v.o_cobj.val, *a2_c = a2->v.o_class.val;

  for (size_t i = 0; i < a1_c->il_c; i++)
    {
      obj_t *c = (obj_t *)a1_c->inh_list[i]->val;

      if (c->v.o_class.val->meta.clref == a2_c)
        {
          return a1_c->inh_list[i];
        }
    }

  obj_t *r = sf_ast_objnew (OBJ_CONST);
  r->v.o_const.type = CONST_NONE;

  return sf_ot_addobj (r);
}

llnode_t *
int_fun_rt (mod_t *mod)
{
  obj_t *o1 = (obj_t *)sf_mod_getVar (mod, "str")->val;

  assert (o1->type == OBJ_CONST && o1->v.o_const.type == CONST_STRING
          && "int() expects a string.");

  obj_t *rt = sf_ast_objnew (OBJ_CONST);
  rt->v.o_const.type = CONST_INT;
  rt->v.o_const.v.c_int.v = atoi (SFCPTR_TOSTR (o1->v.o_const.v.c_string.v));

  return sf_ot_addobj (rt);
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

llnode_t *
nativemethod_type_str_name_replace (mod_t *mod)
{
  obj_t *self = (obj_t *)sf_mod_getVar (mod, "self")->val,
        *arg1 = (obj_t *)sf_mod_getVar (mod, "a1")->val,
        *arg2 = (obj_t *)sf_mod_getVar (mod, "a2")->val;

  char *s = SFCPTR_TOSTR (self->v.o_const.v.c_string.v),
       *a1 = SFCPTR_TOSTR (arg1->v.o_const.v.c_string.v),
       *a2 = SFCPTR_TOSTR (arg2->v.o_const.v.c_string.v);

  size_t sl = strlen (s), a1l = strlen (a1), a2l = strlen (a2);
  size_t p = 0;

  sf_charptr r = sf_str_new_empty ();

  for (size_t i = 0; i < sl; i++)
    {
      if (!strncmp (s + i, a1, a1l))
        {
          sf_str_push (&r, a2);
          i += a1l - 1;
        }
      else
        sf_str_pushchr (&r, s[i]);
    }

  sf_str_free (&self->v.o_const.v.c_string.v);
  self->v.o_const.v.c_string.v = r;

  obj_t *res = sf_ast_objnew (OBJ_CONST);
  res->v.o_const.type = CONST_NONE;

  return sf_ot_addobj (res);
}

llnode_t *
nativemethod_type_int_name_speak (mod_t *mod)
{
  obj_t *self = (obj_t *)sf_mod_getVar (mod, "self")->val;

  char *p = sf_parser_objRepr (mod, self);
  printf ("%s\n", p);

  sffree (p);

  obj_t *o = sf_ast_objnew (OBJ_CONST);
  o->v.o_const.type = CONST_NONE;

  return sf_ot_addobj (o);
}

llnode_t *
nativemethods_type_int_name_bin (mod_t *mod)
{
  obj_t *self = (obj_t *)sf_mod_getVar (mod, "self")->val;

  sf_int i = self->v.o_const.v.c_int.v;
  sf_charptr rs = sf_str_new_empty ();

  while (i != 0)
    {
      sf_str_pushchr (&rs, i % 2 + '0');
      i /= 2;
    }

  sf_str_push (&rs, "b0");
  sf_str_reverse (&rs);

  obj_t *res = sf_ast_objnew (OBJ_CONST);
  res->v.o_const.type = CONST_STRING;
  res->v.o_const.v.c_string.v = rs;

  return sf_ot_addobj (res);
}

llnode_t *
nativemethods_type_int_name_oct (mod_t *mod)
{
  obj_t *self = (obj_t *)sf_mod_getVar (mod, "self")->val;

  sf_int i = self->v.o_const.v.c_int.v;
  sf_charptr rs = sf_str_new_empty ();

  while (i != 0)
    {
      sf_str_pushchr (&rs, i % 8 + '0');
      i /= 8;
    }

  sf_str_push (&rs, "o0");
  sf_str_reverse (&rs);

  obj_t *res = sf_ast_objnew (OBJ_CONST);
  res->v.o_const.type = CONST_STRING;
  res->v.o_const.v.c_string.v = rs;

  return sf_ot_addobj (res);
}

llnode_t *
nativemethods_type_int_name_hex (mod_t *mod)
{
  obj_t *self = (obj_t *)sf_mod_getVar (mod, "self")->val;

  sf_int i = self->v.o_const.v.c_int.v;
  sf_charptr rs = sf_str_new_empty ();

  char q;
  while (i != 0)
    {
      q = i % 16;
      if (q > 9)
        q = 'a' + (q - 10);
      else
        q += '0';

      sf_str_pushchr (&rs, q);
      i /= 16;
    }

  sf_str_push (&rs, "x0");
  sf_str_reverse (&rs);

  obj_t *res = sf_ast_objnew (OBJ_CONST);
  res->v.o_const.type = CONST_STRING;
  res->v.o_const.v.c_string.v = rs;

  return sf_ot_addobj (res);
}

llnode_t *
len_fun_rt (mod_t *mod)
{
  obj_t *o = (obj_t *)sf_mod_getVar (mod, "arg")->val;

  int l = 0;

  switch (o->type)
    {
    case OBJ_CONST:
      {
        switch (o->v.o_const.type)
          {
          case CONST_STRING:
            {
              l = strlen (SFCPTR_TOSTR (o->v.o_const.v.c_string.v));
            }
            break;

          default:
            {
              e_printf ("object does not have len property.\n");
            }
            break;
          }
      }
      break;

    case OBJ_ARRAY:
      {
        array_t *t = o->v.o_array.v;
        l = t->len;
      }
      break;

    default:
      {
        e_printf ("object does not have len property.\n");
      }
      break;
    }

  obj_t *n = sf_ast_objnew (OBJ_CONST);
  n->v.o_const.type = CONST_INT;
  n->v.o_const.v.c_int.v = l;

  return sf_ot_addobj (n);
}

llnode_t *
dbg_prntrc_rt (mod_t *mod)
{
  llnode_t *l = sf_mod_getVar (mod, "obj");
  printf (
      "ref_count: %d, pa_size: %d (without accounting for arg in prc(arg))\n",
      l->meta.ref_count - 1, ((obj_t *)l->val)->meta.pa_size);

  obj_t *n = sf_ast_objnew (OBJ_CONST);
  n->v.o_const.type = CONST_NONE;

  return sf_ot_addobj (n);
}

void
test5 ()
{
  sf_module_addlookuppath ("../../tests/");

  char *cont = readfile ("../../tests/test.sf");

  tok_t *t = sf_tokenizer_gen (cont);
  size_t sptr = 0;
  stmt_t *st = sf_ast_stmtgen (t, &sptr);

  mod_t *m = sf_mod_new (MOD_TYPE_FILE, NULL);

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
  /* super() function */

  {
    mod_t *super_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);
    obj_t *inst_obj = sf_ast_objnew (OBJ_CONST);
    obj_t *cl_obj = sf_ast_objnew (OBJ_CONST);

    inst_obj->v.o_const.type = CONST_NONE;
    cl_obj->v.o_const.type = CONST_NONE;

    sf_mod_addVar (super_mod, "inst", sf_ot_addobj (inst_obj));
    sf_mod_addVar (super_mod, "cl", sf_ot_addobj (cl_obj));

    fun_t *super_fun
        = sf_fun_new ("super", SF_FUN_NATIVE, super_mod, super_fun_rt);

    sf_fun_addarg (super_fun, "inst");
    sf_fun_addarg (super_fun, "cl");

    fun_t *sfp = sf_fun_add (super_fun);

    obj_t *sp_obj = sf_ast_objnew (OBJ_FUN);
    sp_obj->v.o_fun.f = sfp;

    sf_mod_addVar (m, "super", sf_ot_addobj (sp_obj));
  }

  /********************************************* */
  /********************************************* */
  /* int(str) function */

  {
    mod_t *int_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);
    obj_t *str_obj = sf_ast_objnew (OBJ_CONST);

    str_obj->v.o_const.type = CONST_STRING;
    str_obj->v.o_const.v.c_string.v = sf_str_new_fromStr ("");

    sf_mod_addVar (int_mod, "str", sf_ot_addobj (str_obj));

    fun_t *int_fun = sf_fun_new ("int", SF_FUN_NATIVE, int_mod, int_fun_rt);

    sf_fun_addarg (int_fun, "str");

    fun_t *ifp = sf_fun_add (int_fun);

    obj_t *ir_obj = sf_ast_objnew (OBJ_FUN);
    ir_obj->v.o_fun.f = ifp;

    sf_mod_addVar (m, "int", sf_ot_addobj (ir_obj));
  }

  /********************************************* */

  /********************************************* */
  /* STRING METHODS */
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

  /* ''.replace() */
  {
    mod_t *rp_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);

    fun_t *rp_fun = sf_fun_new ("replace", SF_FUN_NATIVE, rp_mod,
                                nativemethod_type_str_name_replace);

    sf_fun_addarg (rp_fun, "self");
    sf_fun_addarg (rp_fun, "a1");
    sf_fun_addarg (rp_fun, "a2");

    fun_t *sopf = sf_fun_add (rp_fun);

    sf_nm_add ("replace", CONST_STRING, sopf);
  }

  /********************************************* */

  /********************************************* */
  /* ord(str) function */

  {
    mod_t *ord_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);
    obj_t *str_obj = sf_ast_objnew (OBJ_CONST);

    str_obj->v.o_const.type = CONST_STRING;
    str_obj->v.o_const.v.c_string.v = sf_str_new_fromStr ("");

    sf_mod_addVar (ord_mod, "str", sf_ot_addobj (str_obj));

    fun_t *inp_fun = sf_fun_new ("ord", SF_FUN_NATIVE, ord_mod, ord_fun_rt);

    sf_fun_addarg (inp_fun, "str");

    fun_t *ofp = sf_fun_add (inp_fun);

    obj_t *or_obj = sf_ast_objnew (OBJ_FUN);
    or_obj->v.o_fun.f = ofp;

    sf_mod_addVar (m, "ord", sf_ot_addobj (or_obj));
  }

  /********************************************* */

  /********************************************* */
  /* INT METHODS */

  {
    mod_t *ispeak_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);

    fun_t *isk_fun = sf_fun_new ("speak", SF_FUN_NATIVE, ispeak_mod,
                                 nativemethod_type_int_name_speak);

    sf_fun_addarg (isk_fun, "self");

    sf_nm_add ("speak", CONST_INT, isk_fun);
  }

  {
    mod_t *itb_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);

    fun_t *itb_fun = sf_fun_new ("bin", SF_FUN_NATIVE, itb_mod,
                                 nativemethods_type_int_name_bin);

    sf_fun_addarg (itb_fun, "self");

    sf_nm_add ("bin", CONST_INT, itb_fun);
  }

  {
    mod_t *ito_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);

    fun_t *ito_fun = sf_fun_new ("oct", SF_FUN_NATIVE, ito_mod,
                                 nativemethods_type_int_name_oct);

    sf_fun_addarg (ito_fun, "self");

    sf_nm_add ("oct", CONST_INT, ito_fun);
  }

  {
    mod_t *ith_mod = sf_mod_new (MOD_TYPE_FUNC, NULL);

    fun_t *ith_fun = sf_fun_new ("hex", SF_FUN_NATIVE, ith_mod,
                                 nativemethods_type_int_name_hex);

    sf_fun_addarg (ith_fun, "self");

    sf_nm_add ("hex", CONST_INT, ith_fun);
  }

  /********************************************* */

  {
    mod_t *lenfmod = sf_mod_new (MOD_TYPE_FUNC, NULL);

    obj_t *al = sf_ast_objnew (OBJ_CONST);
    al->v.o_const.type = CONST_STRING;
    al->v.o_const.v.c_string.v = sf_str_new_empty ();

    sf_mod_addVar (lenfmod, "arg", sf_ot_addobj (al));

    fun_t *f = sf_fun_new ("len", SF_FUN_NATIVE, lenfmod, len_fun_rt);
    sf_fun_addarg (f, "arg");

    fun_t *rf = sf_fun_add (f);

    obj_t *fo = sf_ast_objnew (OBJ_FUN);
    fo->v.o_fun.f = rf;

    sf_mod_addVar (m, "len", sf_ot_addobj (fo));
  }

  {
    // dbg_prntrc_rt
    mod_t *dbg1 = sf_mod_new (MOD_TYPE_FUNC, NULL);

    obj_t *al = sf_ast_objnew (OBJ_CONST);
    al->v.o_const.type = CONST_STRING;
    al->v.o_const.v.c_string.v = sf_str_new_empty ();

    sf_mod_addVar (dbg1, "obj", sf_ot_addobj (al));

    fun_t *f = sf_fun_new ("prc", SF_FUN_NATIVE, dbg1, dbg_prntrc_rt);
    sf_fun_addarg (f, "obj");

    fun_t *rf = sf_fun_add (f);

    obj_t *fo = sf_ast_objnew (OBJ_FUN);
    fo->v.o_fun.f = rf;

    sf_mod_addVar (m, "prc", sf_ot_addobj (fo));
  }

  sf_module_setparent (m);

  mod_t *mainmod = sf_mod_new (MOD_TYPE_FILE, m);
  mainmod->body = st;
  mainmod->body_len = sptr;

  // while (1)
  {
    sf_parser_exec (mainmod);
    // mod_t *ml = sf_mod_new (1, NULL);

    // obj_t *o = sf_ast_objnew (OBJ_CONST);
    // o->v.o_const.type = CONST_NONE;

    // llnode_t *ol = sf_ot_addobj (o);

    // sf_mod_addVar (ml, "test", ol);
    // sf_mod_addVar (ml, "test1", ol);
    // sf_mod_addVar (ml, "test2", ol);
    // sf_mod_addVar (ml, "test3", ol);

    // sf_mod_free (ml);
    // printf ("%d\n", ol->meta.ref_count);
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

  for (size_t i = 0; k[i] != NULL; i++)
    {
      sffree (k[i]);
    }
  sffree (k);

  sf_mod_free (mainmod);
  sf_mod_free (m);
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

  sf_tree_free (t, NULL);
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
  sf_module_init ();

#if !defined(SF_NODEBUG)
  sf_dbg_fledump_init ();
#endif

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
#if !defined(SF_NODEBUG)
  sf_dbg_dumpclose ();
#endif
  sf_module_dest ();

  return !printf ("Program ended.\n");
}