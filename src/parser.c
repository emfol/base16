#include <stdlib.h>
#include "parser.h"

parser_context_ref parser_create_context(void)
{
    parser_context_ref ctx = (parser_context_ref)malloc(sizeof(parser_context_t) + PARSER_BUFFER_SIZE);
    if ( ctx != NULL ) {
        ctx->in_base = (parser_index_t)ctx + sizeof(parser_context_t);
        ctx->out_limit = (parser_index_t)ctx + sizeof(parser_context_t) + PARSER_BUFFER_SIZE - 1;
        ctx->out_base = (parser_index_t)ctx->in_base + (PARSER_BUFFER_SIZE / 2);
        ctx->in_limit = (parser_index_t)ctx->out_base - 1;
        ctx->in_ptr = ctx->in_base;
        ctx->out_ptr = ctx->out_base;
        ctx->status = 0L;
    }
    return ctx;
}

void parser_destroy_context(parser_context_ref context)
{
    free(context);
}

