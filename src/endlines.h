/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   endlines : Mathias Dolidon / 2014 */

#ifndef _ENDLINES_H
#define _ENDLINES_H_

#ifndef BYTE
#define BYTE unsigned char
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// All the conventions we know about.
// Define the enum values here.
// Define display names and command-line names in main.c
// Define binary reality in engine.c
#define KNOWN_CONVENTIONS_COUNT 3
typedef enum {
    CR,
    LF,
    CRLF
} convention_t;


// Command line options
typedef struct {
    convention_t convention;
    int files;
    bool quiet;
} options_t;


// The name for the temporary file that's written to in file mode.
#define TMP_FILE_NAME ".tmp_endlines"


// This function means business.
// It is exported by engine.c
void
convert(FILE* instream, FILE* outstream, convention_t convention);

#endif
