#pragma once

#include "header.h"
#include "sfmem.h"
#include "sfstr.h"

enum
{
  TOK_STRING = 0,
  TOK_INT = 1,
  TOK_FLOAT = 2,
  TOK_OPERATOR = 3,
  TOK_IDENTIFIER = 4,
  TOK_COMMENT = 5,
  TOK_SPACE = 6,
  TOK_NEWLINE = 7,
  TOK_EOF
};

#define NUM_OF_TOKS TOK_EOF

struct _sf_tok_s
{
  int type;

  union
  {
    struct
    {
      sf_charptr v;
    } t_str;

    struct
    {
      sf_int v;
    } t_int;

    struct
    {
      sf_float v;
    } t_float;

    struct
    {
      sf_charptr v;
    } t_op;

    struct
    {
      sf_charptr v;
      int is_reserved;
      int is_bool;
    } t_ident; /* identifier */

    struct
    {
      sf_charptr v;
    } t_cmt; /* comment */

    struct tokenizer
    {
      int v;
    } t_space; /* only those spaces adjacent to a newline */

  } v;
};

typedef struct _sf_tok_s tok_t;

#ifdef __cplusplus
extern "C"
{
#endif

  // returned array ends with TOK_EOF node
  SF_API tok_t *sf_tokenizer_gen (const char *_Data);

  SF_API void sf_tokenizer_print (tok_t _Tok);

#ifdef __cplusplus
}
#endif
