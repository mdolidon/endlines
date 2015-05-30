/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   endlines : Mathias Dolidon / 2014-2015 */

#ifndef _ENDLINES_H
#define _ENDLINES_H_




// If you have something to configure, it's probably
// one of these three values.


#define VERSION "0.4.1"
#define BUFFERSIZE 15000
#define TMP_FILENAME ".tmp_endlines"





////////////////////////////////////////////////////////////////////////////////

// The rest is made up of :
// - common includes
// - common type definitions
// - function definitions

#include <stdbool.h>
#include <stdio.h>     // for FILE type, if nothing else

#ifndef BYTE
#define BYTE unsigned char
#endif


// All the conventions we know about.
// Define the enum values here.
// Define display names and command-line names in main.c
// Define binary reality in engine.c
#define KNOWN_CONVENTIONS_COUNT 3
typedef enum {
    CR,
    LF,
    CRLF
} Convention;


// Reports from the conversion function to the caller
typedef struct {
    int lines;
    bool contains_control_chars;
} Report;


// This function means business.
// It is exported by engine.c
Report
convert(FILE* p_instream, FILE* p_outstream, Convention convention);

#endif
