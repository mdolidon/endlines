/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   endlines : Mathias Dolidon / 2014 */


#include "endlines.h"

#define BUFFERSIZE 100000

struct Buffered_stream {
    FILE* stream;
    BYTE buffer[BUFFERSIZE];
    int buf_size;
    int buf_ptr;
    bool blocked;
};

static inline void
setup_base_buffered_stream(struct Buffered_stream* b, FILE* stream) {
    b->stream = stream;
    b->buf_size = BUFFERSIZE;
    b->blocked = false;
}

static inline void
setup_input_buffered_stream(struct Buffered_stream* b, FILE* stream) {
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = BUFFERSIZE;
}

static inline void
setup_output_buffered_stream(struct Buffered_stream* b, FILE* stream) {
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = 0;
}
  

// MANAGING AN OUPUT BUFFER

static inline void
flush_out_buffer(struct Buffered_stream* b) {
    fwrite(b->buffer, 1, b->buf_ptr, b->stream);
    b->buf_ptr = 0;
}

static inline void
push_byte(BYTE value, struct Buffered_stream* b) {
    b->buffer[b->buf_ptr] = value;
    ++ b->buf_ptr;
    if(b->buf_ptr == b->buf_size) {
        flush_out_buffer(b);
    }
}

static inline void
push_newline(convention_t convention, struct Buffered_stream* b) {
    switch(convention) {
        case CR: push_byte(13, b);
                 break;
        case LF: push_byte(10, b);
                 break;
        case CRLF: push_byte(13, b);
                   push_byte(10, b);
                   break;
        default: fprintf(stderr, "endlines : internal error, unknown convention\n");
                 exit(2);
    }
}



// MANAGING AN INPUT BUFFER

static inline BYTE
pull_byte(struct Buffered_stream* b) {
    if(b->buf_ptr < b->buf_size) {
        return b->buffer[(b->buf_ptr) ++];
    }
    if(b->buf_size==0 || feof(b->stream)) {
        b->blocked = true;
        return 0;
    }
    b->buf_size = fread(b->buffer, 1, BUFFERSIZE, b->stream);
    b->buf_ptr = 0;
    return pull_byte(b);
}


// LOOKING OUT FOR SOMETHING WEIRD

static inline bool
is_control_char(BYTE byte) {
    return (byte <= 8 || (byte <= 31 && byte >= 14));
}

// BUSINESS


void
convert(FILE* p_instream, FILE* p_outstream, convention_t convention, report_t * report) {
    struct Buffered_stream input_stream; 
    struct Buffered_stream output_stream;
    setup_input_buffered_stream(&input_stream, p_instream);
    setup_output_buffered_stream(&output_stream, p_outstream);

    report->lines = 0;
    report->contains_control_chars = false;

    BYTE byte;
    bool last_was_13 = false;

    for(;;) {
        byte = pull_byte(&input_stream);
        if(input_stream.blocked) {
            break;
        }
        if(is_control_char(byte)) {
            report->contains_control_chars = true;
        }
        if(byte == 13) {
            report->lines ++;
            push_newline(convention, &output_stream);
            last_was_13 = true;
        } else if(byte == 10) {
            if(!last_was_13) {
                report->lines ++;
                push_newline(convention, &output_stream);
            }
            last_was_13 = false;
        } else {
            push_byte(byte, &output_stream);
            last_was_13 = false;
        }
    }
    flush_out_buffer(&output_stream);
}
