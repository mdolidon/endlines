#include "walkers.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#define PROGNAME "endlines"


Walk_tracker
make_default_walk_tracker() {
    Walk_tracker p = {
        DEFAULT_WALK_TRACKER_PARAMS
    };
    return p;
}


void 
walk_filenames(
        char** filenames,
        int file_count, 
        Walk_tracker* tracker
        ) {

    struct stat statinfo;
    char* filename;

    for(int i=0; i<file_count; ++i) {
        filename = filenames[i];

        if(filename[0] == '.' && tracker -> skip_hidden) {
            if(tracker->verbose) {
                fprintf(stderr, "%s : skipping hidden file : %s\n", PROGNAME, filename);
            }
            ++ tracker->skipped_hidden_files_count;
            continue;
        }

        if(stat(filename, &statinfo)) {
            fprintf(stderr, "%s : can not read %s\n", PROGNAME, filename);
            ++ tracker->read_errors_count;
            continue;
        }

        if(S_ISDIR(statinfo.st_mode)) {
            if(tracker->recurse) {
                walk_directory(filename, tracker);
            } else {
                if(tracker->verbose) {
                    fprintf(stderr, "%s : skipping directory : %s\n", PROGNAME, filename);
                }
                ++ tracker->skipped_directories_count;
                continue;
            }
        } else if(S_ISREG(statinfo.st_mode)) {
            ++ tracker->processed_count;
            tracker->process_file(filename, tracker->accumulator);
        }
    }
}



// returns 0 on success
static inline int
append_filename_to_base_path(char* base_path, int base_path_length, char* filename) {
    int total_length = base_path_length + strlen(filename) + 1; // +1 for a slash
    if(total_length+1 > WALKERS_MAX_PATH_LENGTH) {              // +1 for terminating 0
        fprintf(stderr, "%s : pathname exceeding maximum length : %s/%s\n", PROGNAME, base_path, filename);
        return 1;
    }
    base_path[base_path_length] = '/';
    strcpy(&(base_path[base_path_length+1]), filename);
    return 0;
}

static inline void
reset_base_path_termination(char* base_path, int base_path_length) {
    base_path[base_path_length] = 0;
}



void
walk_directory(
            char* directory_name,
            Walk_tracker* tracker
        ){

    // setting up a path buffer that we'll update with the successive paths of the files we'll iterate upon
    char file_path_buffer[WALKERS_MAX_PATH_LENGTH];
    char* fake_filenames_array[1];              // this holder is a convenience
    fake_filenames_array[0] = file_path_buffer; //  to avoid juggling pointer notations

    int dirname_length = strlen(directory_name);

    if(dirname_length+1 >= WALKERS_MAX_PATH_LENGTH) {
        fprintf(stderr, "%s : pathname exceeding maximum length : %s\n", PROGNAME, directory_name);
        return;
    } 

    strcpy(file_path_buffer, directory_name);

    // opening the directory
    DIR * pdir;
    pdir = opendir(directory_name);
    if(pdir == NULL) {
        fprintf(stderr, "%s : can not open directory %s\n", directory_name, PROGNAME);
        return;
    }

    // iterating over its contents and letting walk_filenames figure out what to do with each piece of it
    struct dirent *pent;
    while((pent = readdir(pdir)) != NULL) {
        reset_base_path_termination(file_path_buffer, dirname_length);
        if(strcmp(pent->d_name, ".") == 0 ||
           strcmp(pent->d_name, "..") == 0) {
            continue;
        }
        if(pent->d_name[0] == '.' && tracker -> skip_hidden) {
            if(tracker->verbose) {
                fprintf(stderr, "%s : skipping hidden file : %s\n", PROGNAME, pent->d_name);
            }
            ++ tracker->skipped_hidden_files_count;
            continue;
        }
        if(append_filename_to_base_path(file_path_buffer, dirname_length, pent->d_name)) {
            continue;
        }
        walk_filenames(fake_filenames_array, 1, tracker);
    }
    closedir(pdir);
};
