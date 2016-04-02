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

/* 
   About the use of macros in this file

   Initially the engine was written as regular C code.
   Behaviour variations to accomodate check mode vs. convert mode 
   and UTF8 vs UTF16LE vs UTF16BE alternatives were managed, at first
   with dynamic function dispatch, then later with simple switches (20% speed gain).
   
   However this still required a lot of conditional checks in the middle of a tight loop.
   In particular in check mode, when acting on files that are aldready in the OS' FS cache,
   these conditionals have a significant performance impact.
   Rewriting them as M4 macros expanding into specialized functions
   made the program run nearly twice faster in the aforementionned situation.

   Even though endline's running time is short anyway, I like the idea of keeping things tight,
   provided there's no high cost in terms of code readability and correctness.
*/




// Word as in "binary word", not as in "english word"

typedef unsigned int word_t; 

typedef enum {
    _1BYTE,
    _2BYTE_LE,
    _2BYTE_BE
} Word_layout;


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
    Word_layout wordLayout;
} Buffered_stream;


// forward declarations
static inline void read_stream_frame(Buffered_stream*);
static Word_layout detect_buffer_word_layout(Buffered_stream*);


static void
setup_base_buffered_stream(Buffered_stream* b, FILE* stream) {
    b->stream = stream;
    b->buf_size = BUFFERSIZE;
    b->eof = false;
}

static void
setup_input_buffered_stream(Buffered_stream* b, FILE* stream) {
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = BUFFERSIZE;
    read_stream_frame(b);
    b->wordLayout = detect_buffer_word_layout(b);
}

static void
setup_output_buffered_stream(Buffered_stream* b, FILE* stream, Word_layout wordLayout) {
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = 0;
    b->wordLayout = wordLayout;
}


// WORD LAYOUT DETECTION
// BOM based only for now
// Precondition : expects the buffer to have been filled up with
// the head of the stream data, and not have been read from yet.

static Word_layout
detect_buffer_word_layout(Buffered_stream* b) {
    if(b->buf_size >= 2) {
        if(b->buffer[0] == 0xFF && b->buffer[1] == 0xFE) {
            return _2BYTE_LE;
        }
        if(b->buffer[0] == 0xFE && b->buffer[1] == 0xFF) {
            return _2BYTE_BE;
        }
    }
    return _1BYTE;
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
push_utf8_word(word_t w, Buffered_stream* b) {
    push_byte(w & 0x000000FF, b);
}

static inline void
push_utf16le_word(word_t w, Buffered_stream* b) {
    push_byte(w & 0x000000FF, b);
    push_byte((w & 0x0000FF00) >> 8, b);
}

static inline void
push_utf16be_word(word_t w, Buffered_stream* b) {
    push_byte((w & 0x0000FF00) >> 8, b);
    push_byte(w & 0x000000FF, b);
}

m4_define(expand_push_newline,
static inline void
push_$1_newline(Convention dst_convention, Buffered_stream* b) {
    switch(dst_convention) {
        case CR: push_$1_word(13, b);
                 break;
        case LF: push_$1_word(10, b);
                 break;
        case CRLF: push_$1_word(13, b);
                   push_$1_word(10, b);
                   break;
        default: break;
    }
}
)
expand_push_newline(utf8)
expand_push_newline(utf16le)
expand_push_newline(utf16be)



// MANAGING AN INPUT BUFFER

static inline void
read_stream_frame(Buffered_stream* b) {
    b->buf_size = fread(b->buffer, 1, BUFFERSIZE, b->stream);
    if(b->buf_size == 0) {
        b->eof = true;
        return;
    }
    b->buf_ptr = 0;
}

static inline BYTE
pull_byte(Buffered_stream* b) {
    if(b->buf_ptr < b->buf_size) {
        return b->buffer[(b->buf_ptr) ++];
    } else {
        read_stream_frame(b);
        if( b-> eof) {
            return 0;
        }
        return pull_byte(b);
    }
}

static inline word_t
pull_utf8_word(Buffered_stream *b) {
    return (word_t) pull_byte(b);
}

static inline word_t
pull_utf16le_word(Buffered_stream *b) {
    word_t b1, b2, w;
    b1 = (word_t) pull_byte(b);
    b2 = (word_t) pull_byte(b);
    w = b1 + (b2<<8);
    return w;
}

static inline word_t
pull_utf16be_word(Buffered_stream *b) {
    word_t b1, b2, w;
    b1 = (word_t) pull_byte(b);
    b2 = (word_t) pull_byte(b);
    w = (b1<<8) + b2;
    return w;
}


// LOOKING OUT FOR BINARY DATA IN A STREAM

static inline bool
is_non_text_char(word_t w) {
    return (w <= 8 || (w <= 31 && w >= 14));
}



// MAIN CONVERSION LOOP

static void
init_report(FileReport* report) {
    report->contains_non_text_chars=false;
    for(int i=0; i<CONVENTIONS_COUNT; i++) {
        report->count_by_convention[i] = 0;
    }
}


// Main loop generator macro
// Parameters :
// $1 : check or convert
// $2 : utf8, utf16le or utf16be
m4_define(`expand_processing_loop',

`m4_ifelse($1, convert,
`static FileReport convert_$2( Buffered_stream* input_stream, 
                        Buffered_stream* output_stream,
                        Convention dst_convention,
                        bool interrupt_if_non_text) {',

`static FileReport check_$2( Buffered_stream* input_stream,
                      bool interrupt_if_non_text) {'
)'

    FileReport report;
    init_report(&report);
    word_t word;
    bool last_was_13 = false;

    while(true) {
        word = pull_$2_word(input_stream);
        if(input_stream->eof) {
            break;
        }
        if(is_non_text_char(word)) {
            report.contains_non_text_chars = true;
            if(interrupt_if_non_text) {
                break;
            }
        }
        if(word == 13) {
            `m4_ifelse($1, `convert', `push_$2_newline(dst_convention, output_stream);')'
            ++ report.count_by_convention[CR];  // may be cancelled by a LF coming up right next
            last_was_13 = true;
        } else if(word == 10) {
            if(!last_was_13) {
                `m4_ifelse($1, `convert', `push_$2_newline(dst_convention, output_stream);')'
                ++ report.count_by_convention[LF];
            } else {
                -- report.count_by_convention[CR];
                ++ report.count_by_convention[CRLF];
            }
            last_was_13 = false;
        } else {
            `m4_ifelse($1, `convert', push_$2`_word(word, output_stream);')'
            last_was_13 = false;
        }
    }
    `m4_ifelse($1, convert, `flush_buffer(output_stream);')'

    return report;
}
) // end of M4's define


expand_processing_loop(check,utf8)      // expands into a check_utf8 function
expand_processing_loop(check,utf16le)   // ... and so on
expand_processing_loop(check,utf16be)
expand_processing_loop(convert,utf8)
expand_processing_loop(convert,utf16le)
expand_processing_loop(convert,utf16be)


FileReport
engine_run( FILE* p_instream,
            FILE* p_outstream,
            Convention dst_convention,
            bool interrupt_if_non_text ) {

    Buffered_stream input_stream;
    setup_input_buffered_stream(&input_stream, p_instream);

    if(p_outstream) {
        Buffered_stream output_stream;
        setup_output_buffered_stream(&output_stream, p_outstream, input_stream.wordLayout);
        switch(input_stream.wordLayout) {
            case _1BYTE:
                return convert_utf8(&input_stream, &output_stream, dst_convention, interrupt_if_non_text);
            case _2BYTE_LE:
                return convert_utf16le(&input_stream, &output_stream, dst_convention, interrupt_if_non_text);
            case _2BYTE_BE:
                return convert_utf16be(&input_stream, &output_stream, dst_convention, interrupt_if_non_text);
        }
    } else {
        switch(input_stream.wordLayout) {
            case _1BYTE:    return check_utf8(&input_stream, interrupt_if_non_text);
            case _2BYTE_LE: return check_utf16le(&input_stream, interrupt_if_non_text);
            case _2BYTE_BE: return check_utf16be(&input_stream, interrupt_if_non_text);
        }
    }
}
