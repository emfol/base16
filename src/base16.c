#include <stdlib.h>
#include "base16.h"

/*
 * Macros
 */

#ifndef BASE16_BUFSIZ
    #define BASE16_BUFSIZ 12000
#elif BASE16_BUFSIZ < 2
    #error "BAD VALUE FOR BASE16_BUFSIZ"
#endif

#define IS_CHAR_EOL(c) \
    ((c) == '\n')

#define IS_CHAR_BLANK(c) \
    ((c) == ' ' \
    || (c) == '\t' \
    || (c) == '\n' \
    || (c) == '\r' \
    || (c) == '\v' \
    || (c) == '\f')

#define IS_CHAR_HEXDIGIT(c) \
    (((c) >= '0' && (c) <= '9') \
    || ((c) >= 'a' && (c) <= 'f'))

#define ADJUST_HEXDIGIT(c) \
    ((c) >= 'A' && (c) <= 'F' && ((c) |= 0x20))

#define GET_HEXDIGIT(c) \
    (((c) >= 'a' && (c) <= 'f') ? ((c) - 'a' + 0x0a) : ((c) - '0'))

#define CTRL_MASK_MODE   (0x03 << 0)
#define CTRL_MASK_STATUS (0x03 << 2)
#define CTRL_MASK_ERROR  (0x03 << 4)

#define CTRL_MODE_NONE   (0x00 << 0)
#define CTRL_MODE_ENCODE (0x01 << 0)
#define CTRL_MODE_DECODE (0x03 << 0)

#define CTRL_STATUS_OK      (0x00 << 2)
#define CTRL_STATUS_PAIR    (0x01 << 2)
#define CTRL_STATUS_COMMENT (0x02 << 2)
#define CTRL_STATUS_ERROR   (0x03 << 2)

#define CTRL_ERROR_NONE  (0x00 << 4)
#define CTRL_ERROR_INVAL (0x01 << 4)
#define CTRL_ERROR_ILSEQ (0x02 << 4)

#define IS_MODE(m, v) \
    (((m) & CTRL_MASK_MODE) == (v))

#define IS_STATUS(s, v) \
    (((s) & CTRL_MASK_STATUS) == (v))

#define SET_STATUS(s, v) \
    ((s) = ((s) & ~CTRL_MASK_STATUS) | (v))

/*
 * Implementations
 */

base16_context_ref base16_create_context(void)
{
    base16_context_ref c = (base16_context_ref)malloc(sizeof(base16_context_t) + BASE16_BUFSIZ);
    if (c != NULL) {
        c->in_base = (base16_index_t)c + sizeof(base16_context_t);
        c->out_limit = c->in_base + (BASE16_BUFSIZ - 1);
        c->out_base = c->out_limit - ((BASE16_BUFSIZ / 2) - 1); /* default ratio... 1/2 + 1/2 */
        c->in_limit = c->out_base - 1;
        c->in_ptr = c->in_base;
        c->out_ptr = c->out_base;
        c->char_cnt = 0;
        c->line_cnt = 0;
        c->line_ptr = 0;
        c->ctrl = CTRL_MODE_NONE | CTRL_STATUS_OK | CTRL_ERROR_NONE;
    }
    return c;
}

void base16_destroy_context(base16_context_ref c)
{
    free(c);
}

base16_reg_t base16_verify_context_integrity(base16_context_ref c)
{
    base16_reg_t result = 0;
    if (c->in_base != (base16_index_t)c + sizeof(base16_context_t))
        result |= (1 << 0);
    if (c->out_limit != c->in_base + (BASE16_BUFSIZ - 1))
        result |= (1 << 1);
    if (c->out_base != c->in_limit + 1)
        result |= (1 << 2);
    if (c->in_limit < c->in_base)
        result |= (1 << 3);
    if (c->out_limit < c->out_base)
        result |= (1 << 4);
    return result;
}

base16_reg_t base16_encode(base16_context_ref c)
{

    base16_index_t i_ptr, i_lim, o_ptr, o_lim;
    base16_char_t i_chr, o_chr;
    base16_reg_t cr;

    cr = c->ctrl;
    if (IS_MODE(cr, CTRL_MODE_ENCODE))
        return 1;

    i_ptr = c->in_base;
    i_lim = c->in_ptr;
    o_ptr = c->out_base;
    o_lim = c->out_limit;

    while (i_ptr <= i_lim && o_ptr <= o_lim) {
        i_chr = *i_ptr++;
        ADJUST_HEXDIGIT(i_chr);
        if (IS_STATUS(cr, CTRL_STATUS_PAIR)) {
            if (IS_CHAR_HEXDIGIT(i_chr)) {
                SET_STATUS(cr, CTRL_STATUS_OK);
                o_chr = (o_chr << 4) | GET_HEXDIGIT(i_chr);
                *o_ptr++ = o_chr;
            } else {
                SET_STATUS(cr, CTRL_STATUS_ERROR);
                /* TODO: SET_ERROR(cr, CTRL_ERROR_ILSEQ)
                 * and update statistics...
                 */
                break;
            }
        } else if (IS_STATUS(cr, CTRL_STATUS_COMMENT)) {
            if (IS_CHAR_EOL(i_chr)) {
                SET_STATUS(cr, CTRL_STATUS_COMMENT);
            }
            continue;
        }
    }

    c->ctrl = cr;
    c->out_ptr = o_ptr;

    return 0;

}

