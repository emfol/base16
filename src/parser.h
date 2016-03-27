#ifndef __PARSER_H
#define __PARSER_H

/*
 * Macros
 */

#define PARSER_BUFFER_SIZE ( 1 * 4096UL )

/*
 * Type Definitions
 */

typedef unsigned long parser_reg_t;
typedef unsigned char parser_char_t;
typedef parser_char_t *parser_index_t;
typedef struct parser_context {
    parser_index_t in_base, in_limit, in_ptr;
    parser_index_t out_base, out_limit, out_ptr;
    parser_reg_t status;
} parser_context_t;
typedef parser_context_t *parser_context_ref;

/*
 * Entry Points
 */

/* Alloc parsing context structure... */
parser_context_ref parser_create_context(void);

/* Dealloc Parsing context structure... */
void parser_destroy_context(parser_context_ref context);

#endif
