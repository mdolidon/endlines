/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   endlines : Mathias Dolidon / 2014-2015 */

#include "endlines.h"


// BUFFERED STREAM STRUCTURE DEFINITION AND INSTANCIATION

// Note : I don't subtype Buffered_stream into an input
// version and an output version to enforce their limitations.
//
// Just be reasonable : don't try to pull from an output stream,
// and don't try to push into an input stream.

typedef struct {
    FILE* stream;
    BYTE buffer[BUFFERSIZE];
    int buf_size;
    int buf_ptr;
} Buffered_stream;

static inline void
setup_base_buffered_stream(Buffered_stream* b, FILE* stream) {
    b->stream = stream;
    b->buf_size = BUFFERSIZE;
}

static inline void
setup_input_buffered_stream(Buffered_stream* b, FILE* stream) {
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = BUFFERSIZE;
}

static inline void
setup_output_buffered_stream(Buffered_stream* b, FILE* stream) {
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = 0;
}
  


// MANAGING AN OUTPUT BUFFER

static inline void
flush_buffer(Buffered_stream* b) {
    fwrite(b->buffer, 1, b->buf_ptr, b->stream);
    b->buf_ptr = 0;
}

static inline void
push_byte(BYTE value, Buffered_stream* b) {
    b->buffer[b->buf_ptr] = value;
    ++ b->buf_ptr;
    if(b->buf_ptr == b->buf_size) {
        flush_buffer(b);
    }
}

static inline void
push_newline(Convention convention, Buffered_stream* b) {
    switch(convention) {
        case CR: push_byte(13, b);
                 break;
        case LF: push_byte(10, b);
                 break;
        case CRLF: push_byte(13, b);
                   push_byte(10, b);
                   break;
    }
}



// MANAGING AN INPUT BUFFER

static inline bool
has_data(Buffered_stream* b) {
    return b->buf_ptr < b->buf_size ||
           !feof(b->stream);
}

static inline BYTE
pull_byte(Buffered_stream* b) {
    if(b->buf_ptr < b->buf_size) {
        return b->buffer[(b->buf_ptr) ++];
    } else {
        b->buf_size = fread(b->buffer, 1, BUFFERSIZE, b->stream);
        b->buf_ptr = 0;
        return pull_byte(b);
    }
}



// LOOKING OUT FOR BINARY DATA IN A STREAM

static inline bool
is_control_char(BYTE byte) {
    return (byte <= 8 || (byte <= 31 && byte >= 14));
}



// MAIN CONVERSION LOOP

Report
convert(FILE* p_instream, FILE* p_outstream, Convention convention) {
    Buffered_stream input_stream; 
    Buffered_stream output_stream;
    setup_input_buffered_stream(&input_stream, p_instream);
    setup_output_buffered_stream(&output_stream, p_outstream);

    Report report = {.lines=0, .contains_control_chars=false};

    BYTE byte;
    bool last_was_13 = false;

    while(has_data(&input_stream)) {
        byte = pull_byte(&input_stream);
        if(is_control_char(byte)) {
            report.contains_control_chars = true;
        }
        if(byte == 13) {
            report.lines ++;
            push_newline(convention, &output_stream);
            last_was_13 = true;
        } else if(byte == 10) {
            if(!last_was_13) {
                report.lines ++;
                push_newline(convention, &output_stream);
            }
            last_was_13 = false;
        } else {
            push_byte(byte, &output_stream);
            last_was_13 = false;
        }
    }
    flush_buffer(&output_stream);
    return report;
}
