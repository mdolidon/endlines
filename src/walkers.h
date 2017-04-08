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


#ifndef _WALKERS_H_
#define _WALKERS_H_

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "endlines"  // I don't know a clean X-platform way to figure out the program
#endif                           // name ; so I set it uncleanly...

#define WALKERS_MAX_PATH_LENGTH 1024 

#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>

//
// The walkers : walk_filenames and walk_directory.
// They walk a sequence of items (file names or the content of a directory) and run a callback on regular file items.
// Symbolic links get plainly ignored as of now.
//


//
// Walk_tracker class
// Contains parameters and accumulative properties.
// Parameters :
//
// - process_file : the function pointer to the callback.
//                  It gets called for all regular file items.
//                  It gets called with two parameters :
//     1/ a char* to the relative file name
//     2/ a struct stat* with the file's stat info
//     3/ a void* to the walk's accumulator. What the accumulator is is left up to the client.
//       It carries data over from call to call, and can be incrementally modified.
//
// - recurse : call walk_directory automatically when a subdirectory is found.
// - skip_hidden : skip files whose name starts with a dot.
// - verbose : self explanatory.
//

typedef struct {
    // options
    void (*process_file)(char*, struct stat*, void*);
    void *accumulator;
    bool verbose;
    bool recurse;
    bool skip_hidden;

    // counters updated by the walkers as they go
    int processed_count;
    int skipped_directories_count;
    int skipped_hidden_files_count;
    int read_errors_count;
} Walk_tracker;


// A convenience function to build a tracker object.

Walk_tracker
make_default_walk_tracker();

#define DEFAULT_WALK_TRACKER_PARAMS \
        .process_file = NULL,\
        .accumulator = NULL,\
        .verbose = false,\
        .recurse = false,\
        .skip_hidden = true,\
        .processed_count = 0,\
        .skipped_directories_count = 0,\
        .skipped_hidden_files_count = 0,\
        .read_errors_count = 0


// THE WALKERS

void
walk_filenames(char **filenames, int file_count, Walk_tracker *tracker);

void
walk_directory(char *directory_name, Walk_tracker *tracker);


#endif
