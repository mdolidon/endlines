/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   endlines : Mathias Dolidon / 2014 */


// YOU CAN'T HAVE TWO INSTANCES OF THIS IN THE SAME PROGRAM.
// THIS USES MUTABLE GLOBALS TO HANDLE BUFFERED STREAMS.


#include "endlines.h"

#define BUFFERSIZE 100000

static bool buffers_allocated = false;

static FILE* in_stream;
static BYTE* in_buffer;
static int inbuf_size;
static int inbuf_ptr;
static bool cant_pull;

static FILE* out_stream;
static BYTE* out_buffer;
static int outbuf_size;
static int outbuf_ptr;


// RESETTING THE BUFFERS

static bool
allocate_buffers() {
    in_buffer  = malloc(BUFFERSIZE* sizeof(BYTE));
    out_buffer = malloc(BUFFERSIZE* sizeof(BYTE));
    if(in_buffer==NULL || out_buffer==NULL) {
        return false;
    }
    buffers_allocated = true;
    return true;
}

static void
reset_buffers() {
    inbuf_size = BUFFERSIZE;
    outbuf_size = BUFFERSIZE;
    inbuf_ptr = BUFFERSIZE;
    outbuf_ptr = 0;
    cant_pull = false;
}
  

// MANAGING THE OUPUT BUFFER

static inline void
flush_out_buffer() {
    fwrite(out_buffer, 1, outbuf_ptr, out_stream);
    outbuf_ptr = 0;
}

static inline void
push_byte(BYTE value) {
    out_buffer[outbuf_ptr] = value;
    outbuf_ptr ++;
    if(outbuf_ptr == outbuf_size) {
        flush_out_buffer();
    }
}


static inline void
push_newline(convention_t convention) {
    switch(convention) {
        case CR: push_byte(13);
                 break;
        case LF: push_byte(10);
                 break;
        case CRLF: push_byte(13);
                   push_byte(10);
                   break;
        default: fprintf(stderr, "endlines : internal error, unknown convention\n");
                 exit(2);
    }
}



// MANAGING THE INPUT BUFFER


static inline BYTE
pull_byte() {
    if(inbuf_ptr < inbuf_size) {
        return in_buffer[inbuf_ptr++];
    }
    if(inbuf_size==0 || feof(in_stream)) {
        cant_pull = true;
        return 0;
    }
    inbuf_size = fread(in_buffer, 1, BUFFERSIZE, in_stream);
    inbuf_ptr = 0;
    return pull_byte();
}


// LOOKING OUT FOR SOMETHING WEIRD

static inline bool
is_control_char(BYTE byte) {
    return (byte <= 8 || ( byte <= 31 && byte >= 14));
}

// WHAT WE CAME HERE FOR

void
convert(FILE* instream, FILE* outstream, convention_t convention, report_t * report) {
    if(!buffers_allocated) {
        if(!allocate_buffers()) {
            fprintf(stderr, "endlines : memory error ; can't allocate.\n");
            exit(1);
        }
    }
    in_stream = instream;
    out_stream = outstream;
    reset_buffers();

    report->lines = 0;
    report->contains_control_chars = false;

    BYTE byte;
    bool last_was_13 = false;

    for(;;) {
        byte = pull_byte();
        if(cant_pull) {
            break;
        }
        if(is_control_char(byte)) {
            report->contains_control_chars = true;
        }
        if(byte == 13) {
            report->lines ++;
            push_newline(convention);
            last_was_13 = true;
        }
        else if(byte == 10) {
            if(!last_was_13) {
                report->lines ++;
                push_newline(convention);
            }
            last_was_13 = false;
        }
        else {
            push_byte(byte);
            last_was_13 = false;
        }
    }
    flush_out_buffer();
}
