#include <stdlib.h>
#include "parser.h"

/*
 * Macros
 */

#ifndef PARSER_BUFSIZ
    #define PARSER_BUFSIZ 12000
#elif PARSER_BUFSIZ < 2
    #error INVALID SIZE FOR PARSER BUFFER
#endif

/*
 * Implementations
 */

parser_context_ref parser_create_context(void)
{
    parser_context_ref c = (parser_context_ref)malloc(sizeof(parser_context_t) + PARSER_BUFSIZ);
    if (c != NULL) {
        c->in_base = (parser_index_t)c + sizeof(parser_context_t);
        c->out_limit = c->in_base + (PARSER_BUFSIZ - 1);
        c->out_base = c->out_limit - ((PARSER_BUFSIZ / 2) - 1); /* default ratio... 1/2 + 1/2 */
        c->in_limit = c->out_base - 1;
        c->in_ptr = c->in_base;
        c->out_ptr = c->out_base;
        c->status = 0;
    }
    return c;
}

void parser_destroy_context(parser_context_ref c)
{
    free(c);
}

parser_reg_t parser_verify_context_integrity(parser_context_ref c)
{
    parser_reg_t result = 0;
    if (c->in_base != (parser_index_t)c + sizeof(parser_context_t))
        result |= (1 << 0);
    if (c->out_limit != c->in_base + (PARSER_BUFSIZ - 1))
        result |= (1 << 1);
    if (c->out_base != c->in_limit + 1)
        result |= (1 << 2);
    if (c->in_limit < c->in_base)
        result |= (1 << 3);
    if (c->out_limit < c->out_base)
        result |= (1 << 4);
    return result;
}
