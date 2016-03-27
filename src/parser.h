#ifndef __PARSER_H
#define __PARSER_H

/*
 * Type Definitions
 */

typedef unsigned long parser_reg_t;
typedef unsigned char parser_char_t;
typedef parser_char_t *parser_index_t;
typedef struct parser_context {
    parser_reg_t status;
    /* input */
    parser_index_t in_base;   /* first input address */
    parser_index_t in_limit;  /* last input address */
    parser_index_t in_ptr;    /* next available position */
    /* output */
    parser_index_t out_base;  /* first output address */
    parser_index_t out_limit; /* last output address */
    parser_index_t out_ptr;   /* next available address */
} parser_context_t;
typedef parser_context_t *parser_context_ref;

/*
 * Entry Points
 */

/* Alloc parsing context structure... */
parser_context_ref parser_create_context(void);

/* Dealloc Parsing context structure... */
void parser_destroy_context(parser_context_ref context);

/* Verify context integrity... */
parser_reg_t parser_verify_context_integrity(parser_context_ref context);

#endif
