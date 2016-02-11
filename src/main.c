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

#include "endlines.h"
#include "walkers.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <utime.h>
#include <unistd.h>




//
// ALL ABOUT CONVENTION NAMES
//

typedef struct {
    char name[10];
    Convention convention;
} cmd_line_args_to_convention;

#define CL_NAMES_COUNT 10
const cmd_line_args_to_convention cl_names[CL_NAMES_COUNT] = {
    {.name="lf",      .convention=LF},
    {.name="unix",    .convention=LF},
    {.name="linux",   .convention=LF},
    {.name="osx",     .convention=LF},
    {.name="crlf",    .convention=CRLF},
    {.name="win",     .convention=CRLF},
    {.name="windows", .convention=CRLF},
    {.name="dos",     .convention=CRLF},
    {.name="cr",      .convention=CR},
    {.name="oldmac",  .convention=CR}
};

Convention
read_convention_from_string(char * name) {
    for(int i=0; i<CL_NAMES_COUNT; ++i) {
        if(!strcmp(cl_names[i].name, name)) {
            return cl_names[i].convention;
        }
    }
    fprintf(stderr, "endlines : unknown line end convention : %s\n", name);
    exit(8);
}

const char* convention_display_names[KNOWN_CONVENTIONS_COUNT];

void
setup_conventions_display_names() {
    convention_display_names[CR] = "Legacy Mac (CR)";
    convention_display_names[LF] = "Unix (LF)";
    convention_display_names[CRLF] = "Windows (CR-LF)";
}



//
// THE HELP SCREEN
//

void
display_help_and_quit() {
    fprintf(stderr, "\n ------ Convert line endings  ------\n\n"
                    " Use :\n   endlines OUT_CONVENTION [OPTIONS] [FILES]\n\n"
                    "   Input conventions are determined automatically.\n"
                    "   Each input file may possibly use multiple conventions. \n");
    fprintf(stderr, "   OUT_CONVENTION can be : ");
    for(int i=0; i<CL_NAMES_COUNT; ++i) {
        fprintf(stderr, "%s ", cl_names[i].name);
    }
    fprintf(stderr, "\n   If no files are specified, endlines converts from stdin to stdout.\n\n"
                    " General options :\n"
                    "   -q / --quiet    : silence all but the error messages.\n"
                    "   -v / --verbose  : print more about what's going on.\n"
                    "   --version       : print version number.\n\n"
                    " File options :\n"
                    "   -b / --binaries : don't skip binary files.\n"
                    "   -k / --keepdate : keep files' last modified and last access time stamps.\n"
                    "   -r / --recurse  : recurse into directories.\n"
                    "   -h / --hidden   : process hidden files (/directories) too. \n\n"
                    " Examples :\n"
                    "   endlines unix *.txt\n"
                    "   endlines win -k -r a_folder another_folder\n\n");
    exit(1);
}


void
display_version_and_quit() {
    fprintf(stderr, "\n   * endlines version %s \n", VERSION);

    fprintf(stderr, "   * Copyright 2014-2016 Mathias Dolidon\n\n"
    
                    "   Licensed under the Apache License, Version 2.0 (the \"License\"\n"
                    "   you may not use this file except in compliance with the License.\n"
                    "   You may obtain a copy of the License at\n\n"

                    "       http://www.apache.org/licenses/LICENSE-2.0\n\n"

                    "   Unless required by applicable law or agreed to in writing, software\n"
                    "   distributed under the License is distributed on an \"AS IS\" BASIS,\n"
                    "   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
                    "   See the License for the specific language governing permissions and\n"
                    "   limitations under the License.\n\n");

    exit(0);
}



//
// PARSING COMMAND LINE OPTIONS
// Yes it's a huge and ugly switch
//

typedef struct {
    Convention convention;
    bool quiet;
    bool verbose;
    bool binaries;
    bool keepdate;
    bool recurse;
    bool process_hidden;
    char** filenames;
    int file_count;
} CommandLine;

CommandLine
parse_cmd_line_args(int argc, char** argv) {
    CommandLine cmd_line_args = {.quiet=false, .binaries=false, .keepdate=false, .verbose=false,
    .recurse=false, .process_hidden=false, .filenames=NULL, .file_count=0};

    cmd_line_args.filenames = malloc(argc*sizeof(char*)); // will be marginally too big, we can live with that
    if(cmd_line_args.filenames == NULL) {
        fprintf(stderr, "Can't allocate memory\n");
        exit(1);
    }

    int i;
    for(i=1; i<argc; ++i) {
        if(i>1 && argv[i][0] != '-') {
            cmd_line_args.filenames[cmd_line_args.file_count] = argv[i];
            ++ cmd_line_args.file_count;
            continue;
        } else if(!strcmp(argv[i], "--help")) {
            display_help_and_quit();
        } else if(!strcmp(argv[i], "--version")) {
            display_version_and_quit();
        } else if(i==1) {
            cmd_line_args.convention = read_convention_from_string(argv[1]);
        } else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) {
            cmd_line_args.quiet = true;
        } else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            cmd_line_args.verbose = true;
        } else if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--binaries")) {
            cmd_line_args.binaries = true;
        } else if(!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keepdate")) {
            cmd_line_args.keepdate = true;
        } else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--recurse")) {
            cmd_line_args.recurse = true;
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--hidden")) {
            cmd_line_args.process_hidden = true;
        } else {
            fprintf(stderr, "endlines : unknown option : %s\n", argv[i]);
            exit(4);
        }
    }
    return cmd_line_args;
}



//
// CONVERTING ONE FILE
//

#define PROCESSING_STATUSES_COUNT 4
typedef enum {
    CAN_CONTINUE,
    DONE,
    SKIPPED_BINARY,
    SKIPPED_ERROR,
} processing_status;


processing_status
get_file_stats(char* filename, struct stat* statinfo) {
    if(stat(filename, statinfo)) {
        fprintf(stderr, "endlines : can not read %s\n", filename);
        return SKIPPED_ERROR;
    }
    return CAN_CONTINUE;
}

struct utimbuf
get_file_times(struct stat* statinfo) {
    struct utimbuf file_times;
    file_times.actime = statinfo->st_atime;
    file_times.modtime = statinfo->st_mtime;
    return file_times;
}

processing_status
open_files(FILE** in, char* in_filename, FILE** out, char* out_filename) {
    *in = fopen(in_filename, "rb");
    if(*in == NULL) {
        fprintf(stderr, "endlines : can not read %s\n", in_filename);
        return SKIPPED_ERROR;
    }
    if(access(in_filename, W_OK)) {
        fprintf(stderr, "endlines : can not write over %s\n", in_filename);
        fclose(*in);
        return SKIPPED_ERROR;
    }
    *out = fopen(TMP_FILENAME, "wb");
    if(*out == NULL) {
        fprintf(stderr, "endlines : can not create %s\n", out_filename);
        fclose(*in);
        return SKIPPED_ERROR;
    }
    return CAN_CONTINUE;
}

processing_status
move_temp_file_to_destination(char* filename, struct stat *statinfo) {
    int err = remove(filename);
    if(err) {
        fprintf(stderr, "endlines : can not write over %s\n", filename);
        remove(TMP_FILENAME);
        return SKIPPED_ERROR;
    }
    rename(TMP_FILENAME, filename);
    err = chmod(filename, statinfo->st_mode);
    if(err) {
        fprintf(stderr, "endlines : could not restore permissions for %s\n", filename);
    }
    err = chown(filename, statinfo->st_uid, statinfo->st_gid);
    if(err) {
        fprintf(stderr, "endlines : could not restore ownership for %s\n", filename);
    }

    return CAN_CONTINUE;
}


#define TRY partial_status =
#define CATCH if(partial_status != CAN_CONTINUE) { return partial_status; }

processing_status
convert_one_file(char* filename, CommandLine* cmd_line_args) {
    struct stat statinfo;
    processing_status partial_status;
    FILE *in  = NULL;
    FILE *out = NULL;

    TRY get_file_stats(filename, &statinfo); CATCH
    struct utimbuf original_file_times = get_file_times(&statinfo);
    TRY open_files(&in, filename, &out, TMP_FILENAME); CATCH

    Report report = convert(in, out, cmd_line_args->convention);

    fclose(in);
    fclose(out);

    if(report.contains_control_chars && !cmd_line_args->binaries) {
        remove(TMP_FILENAME);
        return SKIPPED_BINARY;
    }

    TRY move_temp_file_to_destination(filename, &statinfo); CATCH

    if(cmd_line_args->keepdate) {
        utime(filename, &original_file_times);
    }
    return DONE;
}

#undef TRY
#undef CATCH



//
// HANDLING A CONVERSION BATCH
//
// First the helpers...
//

typedef struct {
    int totals[PROCESSING_STATUSES_COUNT];
    CommandLine* cmd_line_args;
} Accumulator;


void
print_verbose_file_outcome(char * filename, processing_status outcome) {
    switch(outcome) {
        case DONE: fprintf(stderr, "endlines : converted %s\n", filename);
            break;
        case SKIPPED_BINARY: fprintf(stderr, "endlines : skipped probable binary %s\n", filename);
            break;
        default: break;
    }
}

void
print_totals(int done, int directories, int binaries, int hidden, int errors) {
        fprintf(stderr,     "endlines : %i file%s converted\n", done, done>1?"s":"");
        if(directories) {
            fprintf(stderr, "           %i director%s skipped\n", directories, directories>1?"ies":"y");
        }
        if(binaries) {
            fprintf(stderr, "           %i binar%s skipped\n", binaries, binaries>1?"ies":"y");
        }
        if(hidden) {
            fprintf(stderr, "           %i hidden file%s skipped\n", hidden, hidden>1?"s":"");
        }
        if(errors) {
            fprintf(stderr, "           %i error%s\n", errors, errors>1?"s":"");
        }
}

//
// ...and now the business end of batch runs
//

void
walkers_callback(char* filename, void* p_accumulator) {
    Accumulator* accumulator = (Accumulator*) p_accumulator;
    processing_status outcome = convert_one_file(filename, accumulator->cmd_line_args);
    ++ accumulator->totals[outcome];
    if(accumulator->cmd_line_args->verbose) {
        print_verbose_file_outcome(filename, outcome);
    }
}


void
convert_files(int argc, char ** argv, CommandLine* cmd_line_args)  {
    Accumulator accumulator;
    for(int i=0; i<PROCESSING_STATUSES_COUNT; ++i) {
        accumulator.totals[i] = 0;
    }
    accumulator.cmd_line_args = cmd_line_args;

    Walk_tracker tracker = make_default_walk_tracker();
    tracker.process_file = &walkers_callback;
    tracker.accumulator = &accumulator;
    tracker.verbose = cmd_line_args->verbose;
    tracker.recurse = cmd_line_args->recurse;
    tracker.skip_hidden = !cmd_line_args->process_hidden;

    if(!cmd_line_args->quiet) {
        fprintf(stderr, "endlines : converting files to %s\n", convention_display_names[cmd_line_args->convention]);
    }

    walk_filenames(cmd_line_args->filenames, cmd_line_args->file_count, &tracker);

    if(!cmd_line_args->quiet) {
        print_totals(accumulator.totals[DONE],
                     tracker.skipped_directories_count,
                     accumulator.totals[SKIPPED_BINARY],
                     tracker.skipped_hidden_files_count,
                     accumulator.totals[SKIPPED_ERROR] + tracker.read_errors_count
        );
    }
}



//
// ENTRY POINT
//

int
main(int argc, char**argv) {
    if(argc <= 1) {
        display_help_and_quit();
    }

    setup_conventions_display_names();
    CommandLine cmd_line_args = parse_cmd_line_args(argc, argv);

    if(cmd_line_args.file_count > 0) {
        convert_files(argc, argv, &cmd_line_args);
    } else {
        if(!cmd_line_args.quiet) {
            fprintf(stderr, "Converting stdin to %s\n", convention_display_names[cmd_line_args.convention]);
        }
        convert(stdin, stdout, cmd_line_args.convention);
    }
    return 0;
}
