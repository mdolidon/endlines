/*
   This file is part of endlines' source code

   Copyright 2014-2016 Mathias Dolidon

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

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
    int eof;
} Buffered_stream;

static inline void
setup_base_buffered_stream(Buffered_stream* b, FILE* stream) {
    b->stream = stream;
    b->buf_size = BUFFERSIZE;
    b->eof = false;
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

static inline BYTE
pull_byte(Buffered_stream* b) {
    if(b->buf_ptr < b->buf_size) {
        return b->buffer[(b->buf_ptr) ++];
    } else {
        b->buf_size = fread(b->buffer, 1, BUFFERSIZE, b->stream);
        if(b->buf_size == 0) {
            b->eof = true;
            return 0;
        }
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

    while(true) {
        byte = pull_byte(&input_stream);
        if(input_stream.eof) {
            break;
        }
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
