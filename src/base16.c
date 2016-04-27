#include <stdlib.h>
#include "base16.h"

/*
 * Macros
 */

#ifndef BASE16_BUFSIZ
    #define BASE16_BUFSIZ 12000
#elif BASE16_BUFSIZ < 1024
    #error "BAD VALUE FOR BASE16_BUFSIZ"
#endif

#define CHAR_COMMENT '#'
#define IS_CHAR_COMMENT(c) \
    ((c) == CHAR_COMMENT)

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

#define ENCODE_HEXDIGIT(c) \
    ((((c) >= 'a' && (c) <= 'f') ? ((c) - 'a' + 0x0a) : ((c) - '0')) & 0x0f)

#define CTRL_MASK_MODE   (0x03 << 0)
#define CTRL_MASK_STATUS (0x03 << 2)
#define CTRL_MASK_ERROR  (0x07 << 4)

#define CTRL_MODE_NONE   (0x00 << 0)
#define CTRL_MODE_ENCODE (0x01 << 0)
#define CTRL_MODE_DECODE (0x03 << 0)

#define CTRL_STATUS_OK      (0x00 << 2)
#define CTRL_STATUS_PAIR    (0x01 << 2)
#define CTRL_STATUS_COMMENT (0x02 << 2)
#define CTRL_STATUS_ERROR   (0x03 << 2)

#define CTRL_ERROR_NONE  (0x00 << 4)
#define CTRL_ERROR_MODE  (0x01 << 4)
#define CTRL_ERROR_FRMT  (0x01 << 4)
#define CTRL_ERROR_INVAL (0x01 << 4)
#define CTRL_ERROR_ILSEQ (0x01 << 4)
#define CTRL_ERROR_NOBUF (0x01 << 4)

#define IS_MODE(m, v) \
    (((m) & CTRL_MASK_MODE) == (v))

#define SET_MODE(m, v) \
    ((m) = ((m) & ~CTRL_MASK_MODE) | ((v) & CTRL_MASK_MODE))

#define IS_STATUS(s, v) \
    (((s) & CTRL_MASK_STATUS) == (v))

#define SET_STATUS(s, v) \
    ((s) = ((s) & ~CTRL_MASK_STATUS) | ((v) & CTRL_MASK_STATUS))

#define GET_ERROR_CODE(e) \
    (((e) & CTRL_MASK_ERROR) >> 4)

#define SET_ERROR(e, v) \
    ((e) = ((e) & ~CTRL_MASK_ERROR) | ((v) & CTRL_MASK_ERROR))

/*
 * Implementations
 */

base16_context_ref base16_create_context(void)
{
    base16_reg_t i;
    base16_context_ref c = (base16_context_ref)malloc(sizeof(base16_context_t) + BASE16_BUFSIZ);
    if (c != NULL) {
        c->in_base   = (base16_index_t)c + sizeof(base16_context_t);
        c->out_limit = c->in_base + (BASE16_BUFSIZ - 1);
        c->out_base  = c->out_limit - ((BASE16_BUFSIZ / 2) - 1); /* default ratio... 1/2 + 1/2 */
        c->in_limit  = c->out_base - 1;
        c->in_first  = NULL;
        c->in_last   = NULL;
        c->out_first = NULL;
        c->out_last  = NULL;
        c->char_cnt  = 0;
        c->line_cnt  = 0;
        c->line_cur  = 0;
        c->ctrl      = CTRL_MODE_NONE | CTRL_STATUS_OK | CTRL_ERROR_NONE;
        for (i = 0; i < (base16_reg_t)sizeof(base16_cache_t); i++)
            c->cache[i] = '\0';
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
    base16_reg_t ctrl;

    ctrl = c->ctrl;
    if (!IS_MODE(ctrl, CTRL_MODE_ENCODE)) {
        SET_ERROR(ctrl, CTRL_ERROR_MODE);
        goto label_quit;
    }

    /* reset error data */
    SET_ERROR(ctrl, CTRL_ERROR_NONE);

    i_ptr = c->in_first;
    i_lim = c->in_last;
    o_ptr = c->out_first;
    o_lim = c->out_last;

    /* main loop */
    while (i_ptr <= i_lim && o_ptr <= o_lim) {
        i_chr = *i_ptr++;
        ADJUST_HEXDIGIT(i_chr);
        if (IS_STATUS(ctrl, CTRL_STATUS_PAIR)) {
            if (IS_CHAR_HEXDIGIT(i_chr)) {
                SET_STATUS(ctrl, CTRL_STATUS_OK);
                o_chr = (o_chr << 4) | ENCODE_HEXDIGIT(i_chr);
                *o_ptr++ = o_chr;
            } else {
                SET_STATUS(ctrl, CTRL_STATUS_ERROR);
                SET_ERROR(ctrl, CTRL_ERROR_ILSEQ);
                /* TODO: SET_ERROR(ctrl, CTRL_ERROR_ILSEQ)
                 * and update statistics...
                 */
                break;
            }
        } else if (IS_STATUS(ctrl, CTRL_STATUS_COMMENT)) {
            if (IS_CHAR_EOL(i_chr)) {
                SET_STATUS(ctrl, CTRL_STATUS_OK);
            }
            continue;
        }
    }

label_quit:
    c->ctrl = ctrl;

    return GET_ERROR_CODE(ctrl);

}

