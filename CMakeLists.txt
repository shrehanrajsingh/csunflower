cmake_minimum_required(VERSION 3.5.0)
project(sunflower VERSION 0.1.0 LANGUAGES C)

enable_testing()
add_subdirectory(tests)

add_library(sunflower
    sunflower.h sunflower.c
    sfmem.h sfmem.c
    tokenizer.h tokenizer.c
    ast.h ast.c
    sfstr.h sfstr.c
    trie.h trie.c
    sfmod.h sfmod.c
    objtable.h objtable.c
    llist.h llist.c
    parser.h parser.c
    function.h function.c
    sfarray.h sfarray.c
    arithmetic.h arithmetic.c
    tree.h tree.c
    header.h
)

