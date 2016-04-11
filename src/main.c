#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

int main(void)
{

    /* alloc memory for context... */
    parser_reg_t result;
    parser_context_ref ctx = parser_create_context();
    if (ctx == NULL)
        return EXIT_FAILURE;

    result = parser_verify_context_integrity(ctx);
    if (result != 0)
        printf("!! Bad integrity result: 0x%lx\n", (unsigned long) result);

    printf(
        "[ context address, context size, integrity ] -> [ 0x%lx, %lu (0x%lx), (0x%lx) ]\n"
        "[ in_base, in_limit, in_ptr ] -> [ 0x%lx, 0x%lx, 0x%lx ]\n"
        "[ out_base, out_limit, out_ptr ] -> [ 0x%lx, 0x%lx, 0x%lx ]\n",
        (unsigned long)ctx, sizeof(parser_context_t), sizeof(parser_context_t), (unsigned long)result,
        (unsigned long)ctx->in_base, (unsigned long)ctx->in_limit, (unsigned long)ctx->in_ptr,
        (unsigned long)ctx->out_base, (unsigned long)ctx->out_limit, (unsigned long)ctx->out_ptr
    );

    /* release memory... */
    parser_destroy_context(ctx);

    return EXIT_SUCCESS;

}

