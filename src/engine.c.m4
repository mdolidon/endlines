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
   Behviour variations to accomodate check mode vs. convert mode 
   and UTF8 vs UTF16LE vs UTF16BE alternatives were managed, at first
   with dynamic function dispatch, then with simple switches (20% speed gain).
   
   However this still required a lot of conditional checks in the middle of a tight loop.
   In particular in check mode, when acting on files that are aldready in the OS' FS cache,
   these checks have a significant performance impact.
   Rewriting them as M4 macros expanding into 6 specialized functions
   made the program run nearly twice faster in the aforementionned situation.

   Even though endline's running time is short anyway, I like the idea of keeping things tight,
   provided there's no high cost in terms of code readability and correctness.
   Let's say this is an experiment with M4. :)
*/




// Word as in "binary word", not as in "english word"

#define WORD unsigned int

typedef enum {
    WT_1BYTE,
    WT_2BYTE_LE,
    WT_2BYTE_BE
} WordType;


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
    WordType wordType;
} Buffered_stream;


// forward declarations
static void read_stream_frame(Buffered_stream*);
static WordType detect_buffer_word_type(Buffered_stream*);


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
    read_stream_frame(b);
    b->wordType = detect_buffer_word_type(b);
}

static inline void
setup_output_buffered_stream(Buffered_stream* b, FILE* stream, WordType wordType) {
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = 0;
    b->wordType = wordType;
}


// WORD TYPE DETECTION
// BOM based only for now
// Precondition : expects the buffer to have been filled up with
// the head of the stream data, and not have been read from yet.

static inline WordType
detect_buffer_word_type(Buffered_stream* b) {
    if(b->buf_size >= 2) {
        if(b->buffer[0] == 0xFF && b->buffer[1] == 0xFE) {
            return WT_2BYTE_LE;
        }
        if(b->buffer[0] == 0xFE && b->buffer[1] == 0xFF) {
            return WT_2BYTE_BE;
        }
    }
    return WT_1BYTE;
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
push_utf8_word(WORD w, Buffered_stream* b) {
    push_byte(w & 0x000000FF, b);
}

static inline void
push_utf16le_word(WORD w, Buffered_stream* b) {
    push_byte(w & 0x000000FF, b);
    push_byte((w & 0x0000FF00) >> 8, b);
}

static inline void
push_utf16be_word(WORD w, Buffered_stream* b) {
    push_byte((w & 0x0000FF00) >> 8, b);
    push_byte(w & 0x000000FF, b);
}

define(m4_expand_push_newline,
static inline void
push_$1_newline(Convention convention, Buffered_stream* b) {
    switch(convention) {
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
m4_expand_push_newline(utf8)
m4_expand_push_newline(utf16le)
m4_expand_push_newline(utf16be)



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

static inline WORD
pull_utf8_word(Buffered_stream *b) {
    return (WORD) pull_byte(b);
}

static inline WORD
pull_utf16le_word(Buffered_stream *b) {
    WORD b1, b2, w;
    b1 = (WORD) pull_byte(b);
    b2 = (WORD) pull_byte(b);
    w = b1 + (b2<<8);
    return w;
}

static inline WORD
pull_utf16be_word(Buffered_stream *b) {
    WORD b1, b2, w;
    b1 = (WORD) pull_byte(b);
    b2 = (WORD) pull_byte(b);
    w = (b1<<8) + b2;
    return w;
}


// LOOKING OUT FOR BINARY DATA IN A STREAM

static inline bool
is_non_text_char(WORD w) {
    return (w <= 8 || (w <= 31 && w >= 14));
}



// MAIN CONVERSION LOOP

void
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
define(`m4expand_processing_loop',

`ifelse($1, convert,
FileReport convert_$2(Buffered_stream* input_stream, Buffered_stream* output_stream, Convention convention) {,
FileReport check_$2(Buffered_stream* input_stream) {
)'

    FileReport report;
    init_report(&report);
    WORD word;
    bool last_was_13 = false;

    while(true) {
        word = pull_$2_word(input_stream);
        if(input_stream->eof) {
            break;
        }
        if(is_non_text_char(word)) {
            report.contains_non_text_chars = true;
        }
        if(word == 13) {
            `ifelse($1, `convert', `push_$2_newline(convention, output_stream);')'
            ++ report.count_by_convention[CR];  // may be cancelled by a LF coming up right next
            last_was_13 = true;
        } else if(word == 10) {
            if(!last_was_13) {
                `ifelse($1, `convert', `push_$2_newline(convention, output_stream);')'
                ++ report.count_by_convention[LF];
            } else {
                -- report.count_by_convention[CR];
                ++ report.count_by_convention[CRLF];
            }
            last_was_13 = false;
        } else {
                `ifelse($1, `convert', push_$2`_word(word, output_stream);')'
                        last_was_13 = false;
        }
    }
    `ifelse($1, convert, `flush_buffer(output_stream);')'

    return report;
}
)


m4expand_processing_loop(check,utf8)      // expands into a check_utf8 function
m4expand_processing_loop(check,utf16le)   // ... and so on
m4expand_processing_loop(check,utf16be)
m4expand_processing_loop(convert,utf8)
m4expand_processing_loop(convert,utf16le)
m4expand_processing_loop(convert,utf16be)


FileReport
engine_run(FILE* p_instream, FILE* p_outstream, Convention convention) {
    Buffered_stream input_stream;
    setup_input_buffered_stream(&input_stream, p_instream);

    if(p_outstream) {
        Buffered_stream output_stream;
        setup_output_buffered_stream(&output_stream, p_outstream, input_stream.wordType);
        switch(input_stream.wordType) {
            case WT_1BYTE:    return convert_utf8(&input_stream, &output_stream, convention);
            case WT_2BYTE_LE: return convert_utf16le(&input_stream, &output_stream, convention);
            case WT_2BYTE_BE: return convert_utf16be(&input_stream, &output_stream, convention);
        }
    } else {
        switch(input_stream.wordType) {
            case WT_1BYTE:    return check_utf8(&input_stream);
            case WT_2BYTE_LE: return check_utf16le(&input_stream);
            case WT_2BYTE_BE: return check_utf16be(&input_stream);
        }
    }
}
