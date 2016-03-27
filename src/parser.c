#include <stdlib.h>
#include "parser.h"

/*
 * Macros
 */

#define PARSER_BUFFER_SIZE 12000UL

/*
 * Implementations
 */

parser_context_ref parser_create_context(void)
{
    parser_context_ref c = (parser_context_ref)malloc(sizeof(parser_context_t) + PARSER_BUFFER_SIZE);
    if ( c != NULL ) {
        c->in_base = (parser_index_t)c + sizeof(parser_context_t);
        c->out_limit = (parser_index_t)c + sizeof(parser_context_t) + PARSER_BUFFER_SIZE - 1;
        c->out_base = (parser_index_t)c->in_base + (PARSER_BUFFER_SIZE / 2);
        c->in_limit = (parser_index_t)c->out_base - 1;
        c->in_ptr = c->in_base;
        c->out_ptr = c->out_base;
        c->status = 0UL;
    }
    return c;
}

void parser_destroy_context(parser_context_ref c)
{
    free(c);
}

parser_reg_t parser_verify_context_integrity(parser_context_ref c)
{
    parser_reg_t result = 0UL;
    if (c->in_base - (parser_index_t)c != sizeof(parser_context_t))
        result |= (1 << 0);
    if (c->out_limit + 1 - c->in_base != PARSER_BUFFER_SIZE)
        result |= (1 << 1);
    if (c->out_base != c->in_limit + 1)
        result |= (1 << 2);
    if (c->in_limit <= c->in_base)
        result |= (1 << 3);
    return result;
}
