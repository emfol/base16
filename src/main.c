#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

int main(void)
{

    /* alloc memory for context... */
    parser_context_ref ctx = parser_create_context();
    if (ctx == NULL)
        return EXIT_FAILURE;

    printf(
        "context address: %lx\n"
        "[ context, buffer, context + buffer ] -> [ %lu (0x%lx), %lu (0x%lx), %lu (0x%lx) ]\n"
        "[ in_base, in_limit, in_ptr ] -> [ 0x%lx, 0x%lx, 0x%lx ]\n"
        "[ out_base, out_limit, out_ptr ] -> [ 0x%lx, 0x%lx, 0x%lx ]\n",
        (unsigned long)ctx,
        sizeof(parser_context_t), sizeof(parser_context_t),
        PARSER_BUFFER_SIZE, PARSER_BUFFER_SIZE,
        sizeof(parser_context_t) + PARSER_BUFFER_SIZE, sizeof(parser_context_t) + PARSER_BUFFER_SIZE,
        (unsigned long)ctx->in_base, (unsigned long)ctx->in_limit, (unsigned long)ctx->in_ptr,
        (unsigned long)ctx->out_base, (unsigned long)ctx->out_limit, (unsigned long)ctx->out_ptr
    );

    /* release memory... */
    parser_destroy_context(ctx);

    return EXIT_SUCCESS;

}

