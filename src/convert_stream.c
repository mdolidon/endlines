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

#include <stdlib.h>


// Word as in "binary word", not as in "english word"

typedef unsigned int word_t; 

typedef enum {
    WT_1BYTE,
    WT_2BYTE_LE,
    WT_2BYTE_BE
} Word_layout;


// BUFFERED STREAM STRUCTURE DEFINITION AND INSTANCIATION

// Note : I don't subtype Buffered_stream into an input
// version and an output version to enforce their limitations.
//
// Just be reasonable : don't try to pull from an output stream,
// and don't try to push into an input stream.


typedef struct {
    FILE *stream;
    BYTE buffer[BUFFERSIZE];
    int buf_size;
    int buf_ptr;
    bool eof;
    Word_layout wordLayout;
} Buffered_stream;


// forward declarations
static inline void read_stream_frame(Buffered_stream*);
static Word_layout detect_buffer_word_layout(Buffered_stream*);


static inline void
setup_base_buffered_stream(Buffered_stream *b, FILE *stream)
{
    b->stream = stream;
    b->buf_size = BUFFERSIZE;
    b->eof = false;
}

static inline void
setup_input_buffered_stream(Buffered_stream *b, FILE *stream)
{
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = BUFFERSIZE;
    read_stream_frame(b);
    b->wordLayout = detect_buffer_word_layout(b);
}

static inline void
setup_output_buffered_stream(Buffered_stream *b, FILE *stream, Word_layout wordLayout)
{
    setup_base_buffered_stream(b, stream);
    b->buf_ptr = 0;
    b->wordLayout = wordLayout;
}


// WORD LAYOUT DETECTION
// BOM based only for now
// Precondition : expects the buffer to have been filled up with
// the head of the stream data, and not have been read from yet.

static Word_layout
detect_buffer_word_layout(Buffered_stream *b)
{
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
// Pushing and flushing check the presence of an actual stream
// b->stream is allowed to contain a null-pointer ; this is used when
// checking files, and lets the loop be written only once.

// writing operations return true if an error occured
static inline bool
flush_buffer(Buffered_stream *b)
{
    if(b->stream) {
        size_t nb_bytes_to_write = (size_t)b->buf_ptr;
        size_t nb_bytes_written = fwrite(b->buffer, 1, nb_bytes_to_write, b->stream);
        if(nb_bytes_to_write != nb_bytes_written) {
            return true;
        }
        b->buf_ptr = 0;
    }
    return false;
}

static inline bool
push_byte(BYTE value, Buffered_stream *b)
{
    bool err = false;
    b->buffer[b->buf_ptr] = value;
    ++ b->buf_ptr;
    if(b->buf_ptr == b->buf_size) {
        err = flush_buffer(b);
    }
    return err;
}


// function pointer was 20% slower than a big switch
static inline bool
push_word(word_t w, Buffered_stream *b)
{
    bool err = false;
    if(b->stream) {
        switch(b->wordLayout) {
        case WT_1BYTE:
            err = err || push_byte( (w & 0x000000FF), b);
            break;
        case WT_2BYTE_LE:
            err = err || push_byte( (w & 0x000000FF), b);
            err = err || push_byte( ((w & 0x0000FF00) >> 8), b);
            break;
        case WT_2BYTE_BE:
            err = err || push_byte( ((w & 0x0000FF00) >> 8), b);
            err = err || push_byte( (w & 0x000000FF), b);
            break;
        default:
            fprintf(stderr, "endlines : convert_stream.push_word called on a stream with"
                            "an unknown word layout ; aborting !\n");
            exit(EXIT_FAILURE);
        }
    }
    return err;
}

static inline bool
push_newline(Convention convention, Buffered_stream *b)
{
    bool err = false;
    switch(convention) {
    case NO_CONVENTION:
        break;
    case CR:
        err = err || push_word(13, b);
        break;
    case LF:
        err = err || push_word(10, b);
        break;
    case CRLF:
        err = err || push_word(13, b);
        err = err || push_word(10, b);
        break;
    default:
        fprintf(stderr, "endlines : convert_stream.push_newline called with an unknown convention ; aborting !\n");
        exit(EXIT_FAILURE);
    }
    return err;
}



// MANAGING AN INPUT BUFFER

static inline void
read_stream_frame(Buffered_stream *b)
{
    b->buf_size = fread(b->buffer, 1, BUFFERSIZE, b->stream);
    if(b->buf_size == 0) {
        b->eof = true;
        return;
    }
    b->buf_ptr = 0;
}

static inline BYTE
pull_byte(Buffered_stream *b)
{
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

// function pointer was 20% slower than a big switch
static inline word_t
pull_word(Buffered_stream *b)
{
    word_t b1, b2, w;
    switch(b->wordLayout) {
    case WT_1BYTE:
        return (word_t) pull_byte(b);
    case WT_2BYTE_LE:
        b1 = (word_t) pull_byte(b);
        b2 = (word_t) pull_byte(b);
        w = b1 + (b2<<8);
        return w;
    case WT_2BYTE_BE:
        b1 = (word_t) pull_byte(b);
        b2 = (word_t) pull_byte(b);
        w = (b1<<8) + b2;
        return w;
    default:
        fprintf(stderr, "endlines : convert_stream.pull_word called on a stream with "
                        "an unknown word layout ; aborting !\n");
        exit(EXIT_FAILURE);
    }
}

// LOOKING OUT FOR BINARY DATA IN A STREAM

static inline bool
is_non_text_char(word_t w)
{
    return (w <= 8 || (w <= 31 && w >= 14));
}





// MAIN CONVERSION LOOP

static inline void
init_report(Conversion_Report *report)
{
    report->error_during_conversion = false;
    report->contains_non_text_chars = false;
    for(int i=0; i<CONVENTIONS_COUNT; i++) {
        report->count_by_convention[i] = 0;
    }
}

Conversion_Report
convert_stream(Conversion_Parameters p)
{
    bool err = false;

    Buffered_stream input_stream;
    setup_input_buffered_stream(&input_stream, p.instream);

    Buffered_stream output_stream;
    setup_output_buffered_stream(&output_stream, p.outstream, input_stream.wordLayout);

    Conversion_Report report;
    init_report(&report);

    word_t word;
    bool last_was_13 = false;
    bool last_was_newline = false;

    while(true) {
        word = pull_word(&input_stream);
        if(input_stream.eof) {
            break;
        }
        if(is_non_text_char(word)) {
            last_was_newline = false;
            report.contains_non_text_chars = true;
            if(p.interrupt_if_non_text) {
                break;
            }
        }
        if(word == 13) {
            err = push_newline(p.dst_convention, &output_stream);
            ++ report.count_by_convention[CR];  // may be cancelled by a LF coming up right next
            last_was_13 = true;
            last_was_newline = true;
        } else if(word == 10) {
            if(!last_was_13) {
                err = push_newline(p.dst_convention, &output_stream);
                last_was_newline = true;
                ++ report.count_by_convention[LF];
                if(p.interrupt_if_not_like_dst_convention && p.dst_convention != LF) {
                    break;
                }
            } else {
                -- report.count_by_convention[CR];
                ++ report.count_by_convention[CRLF];
                last_was_newline = true;
                if(p.interrupt_if_not_like_dst_convention && p.dst_convention != CRLF) {
                    break;
                }
            }
            last_was_13 = false;
        } else {
            err = push_word(word, &output_stream);
            last_was_13 = false;
            last_was_newline = false;
        }
        if(err) {
            break;
        }
    }

    if(p.final_char_has_to_be_eol && !last_was_newline) {
        err = push_newline(p.dst_convention, &output_stream);
    }
    err = err || flush_buffer(&output_stream);

    if(ferror(p.instream)) {
        err = true;
    }
    report.error_during_conversion = err;
    return report;
}
