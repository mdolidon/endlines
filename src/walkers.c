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



        //
        // HELPERS
        //

static void
found_a_file_that_needs_processing(char* filename, Walk_tracker* tracker) {
            ++ tracker->processed_count;
            tracker->process_file(filename, tracker->accumulator);
}

static void
found_a_hidden_file(char* filename, Walk_tracker* tracker) {
    if(tracker->skip_hidden) {
        if(tracker->verbose) {
            fprintf(stderr, "%s : skipping hidden file : %s\n", PROGNAME, filename);
        }
        ++ tracker->skipped_hidden_files_count;
    } else {
        found_a_file_that_needs_processing(filename, tracker);
    }
}

static void
found_an_unreadable_file(char* filename, Walk_tracker* tracker) {
    fprintf(stderr, "%s : can not read %s\n", PROGNAME, filename);
    ++ tracker->read_errors_count;
}

static void
found_a_directory(char* filename, Walk_tracker* tracker) {
    if(tracker->recurse) {
        walk_directory(filename, tracker);
    } else {
        if(tracker->verbose) {
            fprintf(stderr, "%s : skipping directory : %s\n", PROGNAME, filename);
        }
        ++ tracker->skipped_directories_count;
    }
}


// returns 0 on success
static int
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

static void
reset_base_path_termination(char* base_path, int base_path_length) {
    base_path[base_path_length] = 0;
}




        //
        // THE FILE NAMES WALKER
        //

typedef enum {
    YES,
    NO,
    MAYBE
} trivalent;

static inline trivalent 
is_a_hidden_filename_with_a_path_prefix(char *filename, int len) {
    for(int i=len-2; i>=0; --i) {
        if(filename[i]=='/') {
            if(filename[i+1]=='.') {
                return YES;
            } else {
                return NO;
            }
        }
    }
    return MAYBE;
}

static inline bool
is_a_hidden_file_name_witout_a_path_prefix(char *filename, int len) {
   return (filename[0]=='.' && strcmp(filename, ".") && strcmp(filename, "..") &&
                               strcmp(filename, "./") && strcmp(filename, "../"));
}

static bool is_hidden_filename(char* filename) {
    int len = strlen(filename);
    trivalent with_prefix = is_a_hidden_filename_with_a_path_prefix(filename, len);
    if(with_prefix == YES) {
        return true;
    } else if(with_prefix == NO) {
        return false;
    } else {
        return is_a_hidden_file_name_witout_a_path_prefix(filename, len);
    }
}

void 
walk_filenames(char** filenames, int file_count, Walk_tracker* tracker) {
    struct stat statinfo;
    for(int i=0; i<file_count; ++i) {
        if(is_hidden_filename(filenames[i])) {
            found_a_hidden_file(filenames[i], tracker);
        } else if(stat(filenames[i], &statinfo)) {
            found_an_unreadable_file(filenames[i], tracker);
        } else if(S_ISDIR(statinfo.st_mode)) {
            found_a_directory(filenames[i], tracker);
        } else if(S_ISREG(statinfo.st_mode)) {
            found_a_file_that_needs_processing(filenames[i], tracker);
        }
    }
}


        // 
        // THE DIRECTORY WALKER
        //


// returns 0 on success
static int
prepare_to_walk_a_directory(char* directory_name, int dirname_length, char* file_path_buffer, DIR** p_pdir) {
    if(dirname_length+1 >= WALKERS_MAX_PATH_LENGTH) {
        fprintf(stderr, "%s : pathname exceeding maximum length : %s\n", PROGNAME, directory_name);
        return -1;
    } 

    strcpy(file_path_buffer, directory_name);

    // opening the directory
    *p_pdir = opendir(directory_name);
    if(*p_pdir == NULL) {
        fprintf(stderr, "%s : can not open directory %s\n", directory_name, PROGNAME);
        return -1;
    }
    return 0;
}

void
walk_directory(char* directory_name, Walk_tracker* tracker){

    // setting up a path buffer that we'll update with the successive paths of the files we'll iterate upon
    char file_path_buffer[WALKERS_MAX_PATH_LENGTH];
    char* fake_filenames_array[1];              // this holder is a convenience
    fake_filenames_array[0] = file_path_buffer; // to avoid juggling pointer forms
    DIR* pdir;

    int dirname_length = strlen(directory_name);
    if(prepare_to_walk_a_directory(directory_name, dirname_length, file_path_buffer, &pdir)) {
        return; 
    }

    // iterating over its contents and letting walk_filenames figure out what to do with each piece of it
    struct dirent *pent;
    while((pent = readdir(pdir)) != NULL) {
        reset_base_path_termination(file_path_buffer, dirname_length);
        if(strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0) {
            continue;
        }
        if(append_filename_to_base_path(file_path_buffer, dirname_length, pent->d_name)) {
            continue;
        }
        if(pent->d_name[0] == '.') {
            found_a_hidden_file(file_path_buffer, tracker);
        } else if(pent->d_type == DT_DIR) {
            found_a_directory(file_path_buffer, tracker);
        } else if(pent->d_type == DT_REG) {
            found_a_file_that_needs_processing(file_path_buffer, tracker);
        }
    }
    closedir(pdir);
};
