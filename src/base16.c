#include <stdlib.h>
#include "base16.h"

/*
 * Macros
 */

#ifndef BASE16_BUFSIZ
    #define BASE16_BUFSIZ (2 * 4096)
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

#define MASK_CTRL_STATUS       (0x01 << 0)
#define MASK_CTRL_ERROR        (0x07 << 1)
#define MASK_CTRL_MODE         (0x03 << 4)
#define MASK_CTRL_FLAGS        (0x07 << 6)

#define CTRL_STATUS_OK         (0x00 << 0)
#define CTRL_STATUS_ERROR      (0x01 << 0)

#define CTRL_ERROR_NONE        (0x00 << 1)
#define CTRL_ERROR_FAULT       (0x01 << 1)
#define CTRL_ERROR_INVAL       (0x03 << 1)
#define CTRL_ERROR_ILSEQ       (0x04 << 1)
#define CTRL_ERROR_NOBUF       (0x02 << 1)

#define CTRL_MODE_NONE         (0x00 << 4)
#define CTRL_MODE_ENCODE       (0x01 << 4)
#define CTRL_MODE_DECODE       (0x03 << 4)

#define CTRL_FLAGS_NONE        (0x00 << 6)
#define CTRL_FLAGS_PAIR        (0x01 << 6)
#define CTRL_FLAGS_COMMENT     (0x02 << 6)

#define IS_CTRL_STATUS(s, v) \
    (((s) & MASK_CTRL_STATUS) == (v))

#define SET_CTRL_STATUS(s, v) \
    ((s) = ((s) & ~MASK_CTRL_STATUS) | ((v) & MASK_CTRL_STATUS))

#define GET_CTRL_ERROR(e) \
    (((e) & MASK_CTRL_ERROR) >> 1)

#define SET_CTRL_ERROR(e, v) \
    ((e) = ((e) & ~MASK_CTRL_ERROR) | ((v) & MASK_CTRL_ERROR))

#define IS_CTRL_MODE(m, v) \
    (((m) & MASK_CTRL_MODE) == (v))

#define SET_CTRL_MODE(m, v) \
    ((m) = ((m) & ~MASK_CTRL_MODE) | ((v) & MASK_CTRL_MODE))

#define IS_CTRL_FLAGS(f, v) \
    (((f) & MASK_CTRL_FLAGS) == (v))

#define SET_CTRL_FLAGS(f, v) \
    ((f) = ((f) & ~MASK_CTRL_FLAGS) | ((v) & MASK_CTRL_FLAGS))

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
        c->in_start  = NULL;
        c->in_end    = NULL;
        c->out_start = NULL;
        c->out_end   = NULL;
        c->char_cnt  = 0;
        c->line_cnt  = 0;
        c->line_cur  = 0;
        c->ctrl      = CTRL_STATUS_OK | CTRL_ERROR_NONE | CTRL_MODE_NONE | CTRL_FLAGS_NONE;
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
    base16_reg_t ctrl, char_cnt, line_cnt, line_cur;

    ctrl = c->ctrl;

    if (!IS_CTRL_STATUS(ctrl, CTRL_STATUS_OK))
        goto label_abort;

    if (!IS_CTRL_MODE(ctrl, CTRL_MODE_ENCODE)) {
        SET_CTRL_STATUS(ctrl, CTRL_STATUS_ERROR);
        SET_CTRL_ERROR(ctrl, CTRL_ERROR_INVAL);
        goto label_abort;
    }

    /* reset error data */
    SET_CTRL_ERROR(ctrl, CTRL_ERROR_NONE);

    i_ptr = c->in_start;
    i_lim = c->in_end;
    o_ptr = c->out_start;
    o_lim = c->out_end;

    char_cnt = c->char_cnt;
    line_cnt = c->line_cnt;
    line_cur = c->line_cur;

    /* main loop */
    while (i_ptr <= i_lim && o_ptr <= o_lim) {
        i_chr = *i_ptr;
        /* update statistics... */
        char_cnt++;
        if (IS_CHAR_EOL(i_chr)) {
            line_cnt++;
            line_cur = 0;
        } else line_cur++;
        /* payload... */
        if (IS_CTRL_FLAGS(ctrl, CTRL_FLAGS_PAIR)) {
            ADJUST_HEXDIGIT(i_chr);
            if (IS_CHAR_HEXDIGIT(i_chr)) {
                o_chr = (o_chr << 4) | ENCODE_HEXDIGIT(i_chr);
                *o_ptr++ = o_chr;
                SET_CTRL_FLAGS(ctrl, CTRL_FLAGS_NONE);
            } else {
                SET_CTRL_STATUS(ctrl, CTRL_STATUS_ERROR);
                SET_CTRL_ERROR(ctrl, CTRL_ERROR_ILSEQ);
                break;
            }
        } else if (IS_CTRL_FLAGS(ctrl, CTRL_FLAGS_COMMENT)) {
            if (IS_CHAR_EOL(i_chr))
                SET_CTRL_FLAGS(ctrl, CTRL_FLAGS_NONE);
        } else if (IS_CTRL_FLAGS(ctrl, CTRL_FLAGS_NONE)) {
            if (!IS_CHAR_BLANK(i_chr)) {
                if (IS_CHAR_COMMENT(i_chr)) {
                    SET_CTRL_FLAGS(ctrl, CTRL_FLAGS_COMMENT);
                } else {
                    ADJUST_HEXDIGIT(i_chr);
                    if (IS_CHAR_HEXDIGIT(i_chr)) {
                        o_chr = ENCODE_HEXDIGIT(i_chr);
                        SET_CTRL_FLAGS(ctrl, CTRL_FLAGS_PAIR);
                    } else {
                        SET_CTRL_STATUS(ctrl, CTRL_STATUS_ERROR);
                        SET_CTRL_ERROR(ctrl, CTRL_ERROR_ILSEQ);
                        break;
                    }
                }
            }
        } else {
            SET_CTRL_STATUS(ctrl, CTRL_STATUS_ERROR);
            SET_CTRL_ERROR(ctrl, CTRL_ERROR_FAULT);
            break;
        }
        i_ptr++;
    }

    char_cnt = c->char_cnt;
    line_cnt = c->line_cnt;
    line_cur = c->line_cur;

    c->in_start = i_ptr;
    c->out_end = o_ptr - 1;

label_abort:
    c->ctrl = ctrl;
    return GET_CTRL_ERROR(ctrl);

}

