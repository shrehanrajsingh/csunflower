#include "sfstr.h"

SF_API sf_charptr
sf_str_new_empty (void)
{
  sf_charptr r = sfmalloc (sizeof (char));
  *r = '\0';

  return r;
}

SF_API void
sf_str_push (sf_charptr *t, const char *s)
{
  *t = sfrealloc (*t, (strlen (*t) + strlen (s) + 1) * sizeof (**t));
  strcat (*t, s);
}

SF_API void
sf_str_pushchr (sf_charptr *t, const char s)
{
  size_t tl = strlen (*t);
  *t = sfrealloc (*t, (tl + 2) * sizeof (char));
  (*t)[tl] = s;
  (*t)[tl + 1] = '\0';
}

SF_API int
sf_str_eq (sf_charptr _L1, sf_charptr _L2)
{
  return !strcmp (_L1, _L2);
}

SF_API int
sf_str_eq_rCp (sf_charptr _L1, const char *_L2)
{
  return !strcmp (_L1, _L2);
}

SF_API int
sf_str_inStr (const char *_L1, sf_charptr _L2)
{
  return strstr (_L2, _L1) != NULL;
}

SF_API sf_charptr
sf_str_copy (sf_charptr s)
{
  return sfstrdup (s);
}

SF_API sf_charptr
sf_str_new_fromStr (const char *src)
{
  sf_charptr t = sf_str_new_empty ();
  sf_str_push (&t, src);

  return t;
}

SF_API void
sf_str_unescape (sf_charptr *t)
{
  sf_charptr p = *t;
  size_t j = 0;
  size_t pl = strlen (p);

  for (size_t i = 0; i < pl; i++)
    {
      char c = p[i];

      if (c == '\\')
        {
          assert (i + 1 != pl);
          char d = p[i + 1];

          switch (d)
            {
            case 'n':
              p[j] = '\n';
              break;
            case 't':
              p[j] = '\t';
              break;
            case '\\':
              p[j] = '\\';
              break;
            case '\'':
              p[j] = '\'';
              break;
            case '"':
              p[j] = '\"';
              break;

            default:
              e_printf ("Invalid unescape '%c' in sf_str_unescape()\n", d);
              break;
            }
          j++;
          i++;
        }
      else
        {
          p[j] = p[i];
          j++;
        }
    }

  p[j++] = '\0';
  *t = sfrealloc (*t, j * sizeof (**t));
}

SF_API int
sf_str_startswith (sf_charptr s, const char *ss)
{
  size_t sl = strlen (s);
  size_t ssl = strlen (ss);

  if (sl < ssl)
    return 0;

  return !strncmp (s, ss, ssl);
}

SF_API int
sf_str_endswith (sf_charptr s, const char *ss)
{
  size_t sl = strlen (s);
  size_t ssl = strlen (ss);

  if (sl < ssl)
    return 0;

  return !strcmp (s + sl - ssl, ss);
}