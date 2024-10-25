#include "tokenizer.h"

char *_sf_getnum (const char *_Data, int *_IsFlPtr);
char *_sf_getstring (const char *_Data);
char *_sf_getidentifier (const char *_Data);
char *_sf_getoperator (const char *_Data);

int _sf_identifierisreserved (const char *_Id);

SF_API tok_t *
sf_tokenizer_gen (const char *data)
{
  size_t dl = strlen (data);
  size_t i = 0;

  tok_t *res = NULL;
  size_t rc = 0;

  while (i < dl)
    {
      char c = data[i];

      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
        {
          char *r = _sf_getidentifier (data + i);

          tok_t n;
          n.type = TOK_IDENTIFIER;
          n.v.t_ident.v = sf_str_new_fromStr (r);
          n.v.t_ident.is_reserved = _sf_identifierisreserved (r);
          n.v.t_ident.is_bool = !strcmp (r, SF_BOOL_TRUE_REPR)
                                || !strcmp (r, SF_BOOL_FALSE_REPR);

          res = sfrealloc (res, (rc + 1) * sizeof (*res));
          res[rc++] = n;

          i += strlen (r) - 1;
          sffree (r);

          goto l_end;
        }

      if (strstr ("~!%^&*()-+=[]{}|:;<>,./", (char *)(char[]){ c, '\0' })
          != NULL)
        {
          char *r = _sf_getoperator (data + i);

          tok_t n;
          n.type = TOK_OPERATOR;
          n.v.t_op.v = sf_str_new_fromStr (r);

          res = sfrealloc (res, (rc + 1) * sizeof (*res));
          res[rc++] = n;

          i += strlen (r) - 1;
          sffree (r);

          goto l_end;
        }

      switch (c)
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          {
            int isfl = 0;
            char *r = _sf_getnum (data + i, &isfl);
            tok_t n;
            // printf ("%s\n", r);

            if (isfl)
              {
                n.type = TOK_FLOAT;
                n.v.t_float.v = atof (r);
              }
            else
              {
                n.type = TOK_INT;
                n.v.t_int.v = atoi (r);
              }

            res = sfrealloc (res, (rc + 1) * sizeof (*res));
            res[rc++] = n;

            i += strlen (r) - 1;
            sffree (r);
          }
          break;
        case '\n':
          {
            tok_t n = (tok_t){
              .type = TOK_NEWLINE,
            };

            res = sfrealloc (res, (rc + 1) * sizeof (*res));
            res[rc++] = n;
          }
          break;
        case '\t':
          {
            if (!rc)
              {
              l1:;
                tok_t n = (tok_t){
                  .type = TOK_SPACE,
                  .v.t_space.v = 4,
                };

                res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = n;

                goto l_end;
              }

            if (res[rc - 1].type == TOK_NEWLINE)
              {
                goto l1;
              }
            else if (res[rc - 1].type == TOK_SPACE)
              {
                res[rc - 1].v.t_space.v += 4;
              }
          }
          break;
        case ' ':
          {
            if (!rc)
              {
              l2:;
                tok_t n = (tok_t){
                  .type = TOK_SPACE,
                  .v.t_space.v = 1,
                };

                res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = n;

                goto l_end;
              }

            if (res[rc - 1].type == TOK_NEWLINE)
              {
                goto l2;
              }
            else if (res[rc - 1].type == TOK_SPACE)
              {
                res[rc - 1].v.t_space.v += 1;
              }
          }
          break;
        case '\'':
        case '\"':
          {
            char *r = _sf_getstring (data + i);

            if (rc && res[rc - 1].type == TOK_STRING)
              {
                sf_str_push (&res[rc - 1].v.t_str.v, r);
              }
            else
              {
                tok_t n = (tok_t){ .type = TOK_STRING,
                                   .v.t_str.v = sf_str_new_fromStr (r) };

                sf_str_unescape (&n.v.t_str.v);

                res = sfrealloc (res, (rc + 1) * sizeof (*res));
                res[rc++] = n;
              }
            // printf ("%s\n", r);

            i += strlen (r) + 1;
            sffree (r);
          }
          break;

        default:
          break;
        }

    l_end:
      i++;
    }

end:
  res = sfrealloc (res, (rc + 1) * sizeof (*res));
  res[rc] = (tok_t){ .type = TOK_EOF };

  return res;
}

char *
_sf_getnum (const char *d, int *isfl)
{
  char *res = NULL;

  size_t nend_idx = 0;
  size_t dl = strlen (d);

  size_t i;
  for (i = 0; i < dl; i++)
    {
      if (d[i] == '.')
        {
          *isfl = 1;
          continue;
        }

      if (!(d[i] >= '0' && d[i] <= '9'))
        {
          nend_idx = i;
          break;
        }
    }

  if (!nend_idx && i == dl)
    {
      res = sfstrdup ((char *)d);
    }
  else
    {
      res = sfmalloc ((nend_idx + 1) * sizeof (*res));
      strncpy (res, d, nend_idx);

      res[nend_idx] = '\0';
    }

  return res;
}

char *
_sf_getstring (const char *d)
{
  char quote = *d;
  size_t dl = strlen (d);
  size_t qend_idx = 0;

  for (size_t i = 1; i < dl; i++)
    {
      char c = d[i];

      if (c == quote)
        {
          if (d[i - 1] != '\\')
            {
              qend_idx = i;
              break;
            }
        }
    }

  assert (qend_idx && "String quotes not ended.");

  char *res = sfmalloc ((qend_idx) * sizeof (*res));
  strncpy (res, d + 1, qend_idx - 1);

  res[qend_idx - 1] = '\0';

  return res;
}

char *
_sf_getidentifier (const char *d)
{
  char *res = NULL;

  size_t nend_idx = 0;
  size_t dl = strlen (d);

  size_t i;
  for (i = 0; i < dl; i++)
    {
      if ((d[i] >= 'a' && d[i] <= 'z') || (d[i] >= 'A' && d[i] <= 'Z')
          || (d[i] >= '0' && d[i] <= '9') || d[i] == '_')
        ;
      else
        {
          nend_idx = i;
          break;
        }
    }

  if (!nend_idx && i == dl)
    {
      res = sfstrdup ((char *)d);
    }
  else
    {
      res = sfmalloc ((nend_idx + 1) * sizeof (*res));
      strncpy (res, d, nend_idx);

      res[nend_idx] = '\0';
    }

  return res;
}

char *
_sf_getoperator (const char *d)
{
  char op[4] = { 0, 0, 0, 0 };
  char op1 = *d;
  size_t dl = strlen (d);

  switch (op1)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '!':
    case '|':
    case '&':
    case '=':
    case '^':
    case '%':
      {
        op[0] = op1;

        if (dl - 1)
          {
            char op2 = d[1];

            if (op2 == '=')
              {
                op[1] = op2;
              }

            if (op2 == '*' && op2 == op1) // **
              {
                op[1] = op2;

                if (dl - 2)
                  {
                    char op3 = d[2];

                    if (op3 == '=') // **=
                      {
                        op[2] = op3;
                      }
                  }
              }
          }
      }
      break;
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case '~':
    case ':':
    case ';':
    case '.':
    case ',':
      {
        op[0] = op1;
      }
      break;
    case '<':
    case '>':
      {
        op[0] = op1;

        if (dl - 1)
          {
            char op2 = d[1];

            if (op2 == op1) // << or >>
              {
                op[1] = op2;

                if (dl - 2)
                  {
                    char op3 = d[2];

                    if (op3 == '=') // <<= or >>=
                      {
                        op[2] = op3;
                      }
                  }
              }

            if (op2 == '=') // <= or >=
              {
                op[1] = op2;
              }
          }
      }
      break;

    default:
      break;
    }

  return sfstrdup (op);
}

SF_API void
sf_tokenizer_print (tok_t tok)
{
  switch (tok.type)
    {
    case TOK_COMMENT:
      {
        printf ("(comment) %s\n", SFCPTR_TOSTR (tok.v.t_cmt.v));
      }
      break;
    case TOK_EOF:
      {
        printf ("(eof)\n");
      }
      break;
    case TOK_FLOAT:
      {
        printf ("(float) %f\n", tok.v.t_float.v);
      }
      break;
    case TOK_INT:
      {
        printf ("(int) %d\n", tok.v.t_int.v);
      }
      break;
    case TOK_IDENTIFIER:
      {
        printf ("(identifier) %s\n", tok.v.t_ident.v);
      }
      break;
    case TOK_NEWLINE:
      {
        printf ("(newline)\n");
      }
      break;
    case TOK_OPERATOR:
      {
        printf ("(operator) %s\n", tok.v.t_op.v);
      }
      break;
    case TOK_SPACE:
      {
        printf ("(space) %d\n", tok.v.t_space.v);
      }
      break;
    case TOK_STRING:
      {
        printf ("(string) %s\n", tok.v.t_str.v);
      }
      break;

    default:
      printf ("(unknown token) type: %d\n", tok.type);
      break;
    }
}

int
_sf_identifierisreserved (const char *id)
{
  const char *residfs[] = {
    "for",  "if",  "else",  "while",    "import", "to",     "step", "in",
    "type", "fun", "class", "inherits", "as",     "return", "with",
#if !defined(SF_DISABLE_THIS)
    "this",
#endif

    NULL,
  };

  size_t i = 0;

  while (residfs[i] != NULL)
    if (!strcmp (residfs[i], id))
      return 1;
    else
      i++;

  return 0;
}