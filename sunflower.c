#include "sunflower.h"

FILE *SF_DEBUG_DUMP;

SF_API void
sf_dbg_fledump_init ()
{
  SF_DEBUG_DUMP = fopen ("../../tests/dgb.sf", "w");
}

SF_API FILE *
sf_dbg_get_filedump ()
{
  return SF_DEBUG_DUMP;
}

SF_API void
sf_dbg_dumpclose ()
{
  fclose (SF_DEBUG_DUMP);
}