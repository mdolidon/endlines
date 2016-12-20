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

#define PROGRAM_NAME "endlines"
#define VERSION "pre-1.9"
#define BUFFERSIZE 16384
#define TMP_FILENAME_BASE ".tmp_endlines_"

#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <utime.h>

#ifndef BYTE
#define BYTE unsigned char
#endif


#define FILEOP_STATUSES_COUNT 4
typedef enum {
    CAN_CONTINUE,  // intermediate state : no error yet
    DONE,
    SKIPPED_BINARY,
    FILEOP_ERROR
} FileOp_Status;

#define CONVENTIONS_COUNT 5
#define CONVENTIONS_TABLE \
    X(NO_CONVENTION, "No line ending",  "None") \
    X(CR,            "Legacy Mac (CR)", "CR") \
    X(LF,            "Unix (LF)",       "LF") \
    X(CRLF,          "Windows (CR-LF)", "CRLF") \
    X(MIXED,         "Mixed endings",   "Mixed")

#define X(a,b,c) a,
typedef enum {
    CONVENTIONS_TABLE
} Convention;
#undef X

typedef struct {
    FILE *instream;
    FILE *outstream;
    Convention dst_convention;
    bool interrupt_if_not_like_dst_convention;
    bool interrupt_if_non_text;
} Conversion_Parameters;

typedef struct {
    unsigned int count_by_convention[CONVENTIONS_COUNT];
    bool error_during_conversion;
    bool contains_non_text_chars;
} Conversion_Report;


// file_operations.c
struct utimbuf get_file_times(struct stat *statinfo);

FileOp_Status open_input_file_for_conversion(FILE **in,  char *in_filename);
FileOp_Status open_temporary_file(FILE **out, char *tmp_filename);
FileOp_Status open_input_file_for_dry_run(FILE **in,  char *in_filename);

FileOp_Status move_temp_file_to_destination(
        char *tmp_filename, char *filename, struct stat *statinfo);

FileOp_Status make_filename_in_same_location(char *reference_name_and_path, char *wanted_name,
                                             char *destination);

// convert_stream.c
Conversion_Report convert_stream(Conversion_Parameters p);


// utils.c
bool       has_known_binary_file_extension(char*);
Convention get_source_convention(Conversion_Report*);
void       display_help_and_quit();
void       display_version_and_quit();


#endif
