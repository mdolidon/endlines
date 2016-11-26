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

#ifndef _ENDLINES_H_
#define _ENDLINES_H_


// If you have something to configure, it's probably
// one of these three values.


#define VERSION "1.8"
#define BUFFERSIZE 16384
#define TMP_FILENAME_BASE ".tmp_endlines_"



////////////////////////////////////////////////////////////////////////////////

// The rest is made up of :
// - common includes
// - common type definitions
// - function definitions


/* Give us access to high resolution time functions in time.h */
#define _XOPEN_SOURCE 500


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>     // for FILE type, if nothing else

#ifndef BYTE
#define BYTE unsigned char
#endif


// All the conventions we know about.
// Define the enum values here.
// Define display names and command-line names in main.c
// Define binary reality in engine.c
#define CONVENTIONS_COUNT 5
typedef enum {
    NO_CONVENTION,
    CR,
    LF,
    CRLF,
    MIXED
} Convention;



// Reports from the conversion function to the caller
typedef struct {
    bool error_during_conversion;
    bool contains_non_text_chars;
    unsigned int count_by_convention[CONVENTIONS_COUNT];
} FileReport;


typedef struct {
    FILE* instream;
    FILE* outstream;
    Convention dst_convention;
    bool interrupt_if_not_like_dst_convention;
    bool interrupt_if_non_text;
} ConversionParameters;


// This function means business.
// It is exported by engine.c
FileReport
convert_stream(ConversionParameters p);


// Functions declared in utils.c
bool has_known_binary_file_extension(char*);
Convention get_source_convention(FileReport*);

#endif
