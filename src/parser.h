#ifndef __PARSER_H
#define __PARSER_H

typedef char BYTE_T;

typedef struct parser_context {
    int status;
    BYTE_T io_buf[PARSER_BUFFER_SZ];
} parser_context_t;


#endif
