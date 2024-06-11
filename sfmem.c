#include "sfmem.h"

SF_API void *
sfmalloc (size_t _Size)
{
  if (!_Size)
    return NULL;
  return malloc (_Size);
}

SF_API void *
sfrealloc (void *_Old_ptr, size_t _New_size)
{
  if (!_New_size)
    {
      sffree (_Old_ptr);
      return NULL;
    }

  return realloc (_Old_ptr, _New_size);
}

SF_API void
sffree (void *_Ptr)
{
  if (_Ptr == NULL)
    return;
  free (_Ptr);
}

SF_API char *
sfstrdup (char *str)
{
  // assert (sizeof (sf_charptr) == sizeof (char *));

  if (str == NULL)
    return NULL;

  return strdup (str);
}