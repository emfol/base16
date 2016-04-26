#ifndef _BASE16_H
#define _BASE16_H

#include <limits.h>

#if CHAR_BIT != 8
    #error "CHAR SIZE NOT SUPPORTED"
#endif

/*
 * Type Definitions
 */

typedef unsigned long base16_reg_t;
typedef unsigned char base16_char_t;
typedef base16_char_t *base16_index_t;
typedef base16_char_t base16_buffer_t[8];
typedef struct base16_context {
    base16_reg_t ctrl;
    base16_reg_t frmt;
    base16_reg_t char_cnt;
    base16_reg_t line_cnt;
    base16_reg_t line_ptr;
    /* input */
    base16_index_t in_base;   /* address of first available char */
    base16_index_t in_limit;  /* address of last available char */
    base16_index_t in_last;   /* address of last valid char */
    base16_index_t in_ptr;    /* auxiliary pointer */
    /* output */
    base16_index_t out_base;  /* address of first available char */
    base16_index_t out_limit; /* address of last available char */
    base16_index_t out_last;  /* address of last valid char */
    base16_index_t out_ptr;   /* auxiliary pointer */
    base16_buffer_t buf;
} base16_context_t;
typedef base16_context_t *base16_context_ref;

/*
 * Entry Points
 */

/* Alloc parsing context structure... */
base16_context_ref base16_create_context(void);

/* Dealloc Parsing context structure... */
void base16_destroy_context(base16_context_ref context);

/* Verify context integrity... */
base16_reg_t base16_verify_context_integrity(base16_context_ref context);

#endif
