/*
   This file is part of endlines' source code

   Copyright 2014-2017 Mathias Dolidon

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
#define VERSION "1.9.1"


// Prefix used for temporary files' names.
#define TMP_FILENAME_BASE ".tmp_endlines_"

// Size of buffer in bytes, for buffered file reading / writing
#define BUFFERSIZE 16384


// Basic includes for things that are used all across the source code
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <utime.h>

#ifndef BYTE
#define BYTE unsigned char
#endif



// FileOp_Status describes the result of a file operation.

#define FILEOP_STATUSES_COUNT 4
typedef enum {
    CAN_CONTINUE,   // Intermediate state : no error yet.
    DONE,           // The file has been fully processed without any error.
    SKIPPED_BINARY, // The file was a binary, and the options were telling us to skip it.
    FILEOP_ERROR    // Some error happened during the latest operation attempted on this file.
} FileOp_Status;



// The conventions that we recognize, both as we observe the current
// contents of a file, as well as what we aim for.

// The table is defined as an X-Macro : https://en.wikipedia.org/wiki/X_Macro

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






// file_operations.c : our functions for manipulating files

// Checks that we'll be allowed to write inside a file.
// Returns CAN_CONTINUE if allowed, FILEOP_ERROR if not.
FileOp_Status check_write_access(char *filename);


// Open a file in write mode.
// Returns CAN_CONTINUE upon success, FILEOP_ERROR upon failure.
FileOp_Status open_to_write(FILE **out, char *tmp_filename);


// Open a file in read mode.
// Returns CAN_CONTINUE upon success, FILEOP_ERROR upon failure.
FileOp_Status open_to_read(FILE **in,  char *in_filename);


// Deletes filename, then moves tmp_filename in the place of filename.
// The two files need to be on the same physical device.
// At the end, sets the new file's ownership and access rights as per the statinfo.
// Returns CAN_CONTINUE upon success, FILEOP_ERROR upon failure.
FileOp_Status move_temp_file_to_destination(
        char *tmp_filename, char *filename, struct stat *statinfo);


// Example :
// "somewhere/over/the" : reference_name_and_path
// "rainbow" : wanted_name
// --> "somewhere/over/rainbow"  -> result
// The result is stored in the destination buffer. The destination buffer needs
// to be allocated before the call, and large enough. It does not need to be
// larger than WALKERS_MAX_PATH_LENGTH +1 bytes.
// Returns CAN_CONTINUE upon success, FILEOP_ERROR upon failure.
FileOp_Status make_filename_in_same_location(char *reference_name_and_path, char *wanted_name,
                                             char *destination);


// Read the creation time and last modification time from a stat-info into an utimbuf.
struct utimbuf get_file_times(struct stat *statinfo);





// convert_stream.c : the central working piece that can scan file contents
//                    and produce new contents.


// convert_stream's calling signature structure :

typedef struct {
    FILE *instream;              // stream whose content will be converted
    FILE *outstream;             // stream into which to write the converted contents
                                 // (outstream can be NULL)
    Convention dst_convention;   // convention into which to convert
    bool interrupt_if_not_like_dst_convention;  // return prematurely if the input contents
                                                // use a different convention than our destination convention
    bool interrupt_if_non_text;        // return prematurely if the input contents contain
                                       // non-text characters
    bool final_char_has_to_be_eol;  // add a final end-of-line marker if there's none
} Conversion_Parameters;


// convert_stream's return value :

typedef struct {
    unsigned int count_by_convention[CONVENTIONS_COUNT]; 
        // an array telling how many line endings were encountered in the input stream,
        // by convention. 
        // The position in this array matches the position in CONVENTIONS_TABLE.

    bool error_during_conversion;  // true if an error occured during the conversion

    bool contains_non_text_chars;  // true if the input contents contained non-text characters
} Conversion_Report;


Conversion_Report convert_stream(Conversion_Parameters p);




// utils.c


// returns true if filename ends with an extension that typically belongs to binary files
// (such as "picture.png" or "payroll.xls")
bool has_known_binary_file_extension(char* filename); 
                                                            

// from a report produced by convert_stream, returns the type of convention that was used
// in the file or stream that matches this report (including NO_CONVENTION or MIXED)
Convention get_source_convention(Conversion_Report* report); 
                                                             

void display_help_and_quit();
void display_version_and_quit();


#endif
