
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// walkers library : Mathias Dolidon / 2015 


#ifndef _WALKERS_H_
#define _WALKERS_H_

#define WALKERS_MAX_PATH_LENGTH 1025

#include <stdbool.h>
#include <stddef.h>

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
// - process_file : the function pointer to the callback. It gets called for all regular file items. It gets called with two parameters :
//     1/ a char* to the relative file name
//     2/ a void* to the walk's accumulator. What the accumulator is is left up to the client. It carries data over from call to call, and can be incrementally modified.
//
// - recurse : call walk_directory automatically when a subdirectory is found.
// - skip_hidden : skip files whose name starts with a dot.
//

typedef struct {
    // options
    void (*process_file)(char*, void*);
    void* accumulator;
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
walk_filenames(
            char** filenames,      // array of strings
            int file_count,         // length of the array
            Walk_tracker* tracker
        );



void
walk_directory(
            char* directory_name,
            Walk_tracker* tracker
        );

#endif
