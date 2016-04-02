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
#include "known_binary_extensions.h"

#include <string.h>
#include <sys/stat.h>
#include <utime.h>
#include <unistd.h>


 

// =============== LOCAL TYPES  =============== 

// Match a convention name as given on the command line to
// the Convention enum type, defined in endlines.h

typedef struct {
    char name[10];
    Convention convention;
} cmd_line_args_to_convention;


// Holds all command line parameters

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


// Possible outcomes for each file processed 

#define OUTCOMES_COUNT 4
typedef enum {
    CAN_CONTINUE,  // intermediate state : no error yet, but processing not finished
    DONE,
    SKIPPED_BINARY,
    SKIPPED_ERROR,
} Outcome;


// An accumulator that is passed around by the walkers, to the walkers_callback function
// Its main use is to keep track of what has been done
// It is complemented by the walkers' tracker object, defined in walkers.h, 
// that'll hold results that are specific to the walker (e.g. skipped directories and hidden files)

typedef struct {
    int outcome_totals[OUTCOMES_COUNT];
    int convention_totals[CONVENTIONS_COUNT];
    CommandLine* cmd_line_args;
} Accumulator;




// =============== ALL ABOUT CONVENTION NAMES =============== 

#define CL_NAMES_COUNT 11
const cmd_line_args_to_convention cl_names[] = {
    {.name="check",   .convention=NO_CONVENTION},
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
    fprintf(stderr, "endlines : unknown action : %s\n", name);
    exit(8);
}

const char* convention_display_names[CONVENTIONS_COUNT];
const char* convention_short_display_names[CONVENTIONS_COUNT];
void
setup_conventions_display_names() {
    convention_display_names[NO_CONVENTION] = "No line ending";
    convention_short_display_names[NO_CONVENTION] = "None";

    convention_display_names[CR] = "Legacy Mac (CR)";
    convention_short_display_names[CR] = "CR";

    convention_display_names[LF] = "Unix (LF)";
    convention_short_display_names[LF] = "LF";

    convention_display_names[CRLF] = "Windows (CR-LF)";
    convention_short_display_names[CRLF] = "CRLF";

    convention_display_names[MIXED] = "Mixed endings";
    convention_short_display_names[MIXED] = "Mixed";
}





// =============== THE HELP AND VERSION SCREENS =============== 

void
display_help_and_quit() {
    fprintf(stderr, "\n"
                    "  endlines ACTION [OPTIONS] [FILES]\n\n"

                    "  ACTION can be :\n"
                    "    lf, unix, linux, osx    : convert all endings to LF.\n"
                    "    crlf, windows, win, dos : convert all endings to CR-LF.\n"
                    "    cr, oldmac              : convert all endings to CR.\n"
                    "    check                   : perform a dry run to check current conventions.\n\n"

                    "  If no files are specified, endlines converts from stdin to stdout.\n"
                    "  Supports UTF-8, UTF-16 with BOM, and all major single byte codesets.\n\n"

                    "  General   -q / --quiet    : silence all but the error messages.\n"
                    "            -v / --verbose  : print more about what's going on.\n"
                    "            --version       : print version and license.\n\n"

                    "  Files     -b / --binaries : don't skip binary files.\n"
                    "            -h / --hidden   : process hidden files (/directories) too.\n"
                    "            -k / --keepdate : keep last modified and last access times.\n"
                    "            -r / --recurse  : recurse into directories.\n\n"

                    "  Examples  endlines check *.txt\n"
                    "            endlines linux -k -r aFolder anotherFolder\n\n");
    exit(1);
}


void
display_version_and_quit() {
    fprintf(stderr, "\n   * endlines version %s \n"

                    "   * Copyright 2014-2016 Mathias Dolidon\n\n"
    
                    "   Licensed under the Apache License, Version 2.0 (the \"License\"\n"
                    "   you may not use this file except in compliance with the License.\n"
                    "   You may obtain a copy of the License at\n\n"

                    "       http://www.apache.org/licenses/LICENSE-2.0\n\n"

                    "   Unless required by applicable law or agreed to in writing, software\n"
                    "   distributed under the License is distributed on an \"AS IS\" BASIS,\n"
                    "   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
                    "   See the License for the specific language governing permissions and\n"
                    "   limitations under the License.\n\n", VERSION);

    exit(1);
}





// =============== PARSING COMMAND LINE OPTIONS =============== 
// Yes it's a huge and ugly switch

CommandLine
parse_cmd_line_args(int argc, char** argv) {
    CommandLine cmd_line_args = {.quiet=false, .binaries=false, .keepdate=false, .verbose=false,
    .recurse=false, .process_hidden=false, .filenames=NULL, .file_count=0};

    cmd_line_args.filenames = malloc(argc*sizeof(char*)); // will be marginally too big, we can live with that
    if(cmd_line_args.filenames == NULL) {
        fprintf(stderr, "Can't allocate memory\n");
        exit(1);
    }

    for(int i=1; i<argc; ++i) {
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





// =============== CONVERTING OR CHECKING ONE FILE ===============

Outcome
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

Outcome
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

Outcome
open_input_file_for_dry_run(FILE** in, char* in_filename) {
    *in = fopen(in_filename, "rb");
    if(*in == NULL) {
        fprintf(stderr, "endlines : can not read %s\n", in_filename);
        return SKIPPED_ERROR;
    }
    return CAN_CONTINUE;
}

Outcome
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

char*
get_file_extension(char* name) {
    char* extension = name + strlen(name);
    while(*extension != '.' && *extension != '/' && extension != name) {
        -- extension;
    }
    if(*extension == '/' || extension == name) {
        return "";
    } else {
        return extension+1;
    }
}

bool
has_known_binary_file_extension(char* filename) {
    char* ext = get_file_extension(filename);
    for(int i=0; i<KNOWN_BINARY_EXTENSIONS_COUNT; i++) {
        if( !strcmp(ext, known_binary_file_extensions[i]) ) {
            return true;
        }
    }
    return false;
}



// Main conversion (resp. checking) handlers

#define TRY partial_status =
#define CATCH if(partial_status != CAN_CONTINUE) { return partial_status; }

Outcome
convert_one_file(char* filename, CommandLine* cmd_line_args, FileReport* file_report) {

    if(!cmd_line_args->binaries && has_known_binary_file_extension(filename)) {
        return SKIPPED_BINARY;
    }

    struct stat statinfo;
    Outcome partial_status;
    FILE *in  = NULL;
    FILE *out = NULL;

    TRY get_file_stats(filename, &statinfo); CATCH
    struct utimbuf original_file_times = get_file_times(&statinfo);
    TRY open_files(&in, filename, &out, TMP_FILENAME); CATCH

    FileReport report = engine_run(in, out, cmd_line_args->convention, !cmd_line_args->binaries);
    memcpy(file_report, &report, sizeof(FileReport));

    fclose(in);
    fclose(out);

    if(report.contains_non_text_chars && !cmd_line_args->binaries) {
        remove(TMP_FILENAME);
        return SKIPPED_BINARY;
    }

    TRY move_temp_file_to_destination(filename, &statinfo); CATCH

    if(cmd_line_args->keepdate) {
        utime(filename, &original_file_times);
    }
    return DONE;
}


Outcome
check_one_file(char* filename, CommandLine* cmd_line_args, FileReport* file_report) {
    if(!cmd_line_args->binaries && has_known_binary_file_extension(filename)) {
        return SKIPPED_BINARY;
    }

    Outcome partial_status;
    FILE *in  = NULL;
    TRY open_input_file_for_dry_run(&in, filename); CATCH

    FileReport report = engine_run(in, NULL, NO_CONVENTION, !cmd_line_args->binaries);
    memcpy(file_report, &report, sizeof(FileReport));

    fclose(in);
    if(report.contains_non_text_chars && !cmd_line_args->binaries) {
        return SKIPPED_BINARY;
    }

    return DONE;
}

#undef TRY
#undef CATCH





// =============== HANDLING A CONVERSION BATCH ===============

Convention
get_source_convention(FileReport* file_report) {
    Convention c = NO_CONVENTION;
    for(int i=0; i<CONVENTIONS_COUNT; i++) {
        if(file_report->count_by_convention[i] > 0) {
            if(c == NO_CONVENTION) {
                c = (Convention)i;
            } else {
                c = MIXED;
            }
        }
    }
    return c;
}

void
print_verbose_file_outcome(char * filename, Outcome outcome, Convention source_convention) {
    switch(outcome) {
        case DONE:
            fprintf(stderr, "endlines : %s -- %s\n",
                    convention_short_display_names[source_convention], filename);
            break;
        case SKIPPED_BINARY: 
            fprintf(stderr, "endlines : skipped probable binary %s\n", filename);
            break;
        default: break;
    }
}

void
print_outcome_totals(bool dry_run,
                     int* count_by_convention, int done, int directories,
                     int binaries, int hidden, int errors) {
    fprintf(stderr,  "\nendlines : %i file%s %s", done, done>1?"s":"", dry_run?"checked":"converted");
    if(done) {
        fprintf(stderr, " %s :\n", dry_run?"; found":"from");
        for(int i=0; i<CONVENTIONS_COUNT; ++i) {
            if(count_by_convention[i]) {
                fprintf(stderr, "              - %i %s\n",  count_by_convention[i], convention_display_names[i]);
            }
        }
    } else {
        fprintf(stderr, "\n");
    }

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
    fprintf(stderr, "\n");
}

void
walkers_callback(char* filename, void* p_accumulator) {
    Outcome outcome;
    FileReport file_report;
    Convention source_convention;
    Accumulator* accumulator = (Accumulator*) p_accumulator;

    if(accumulator->cmd_line_args->convention == NO_CONVENTION) {
        outcome = check_one_file(filename, accumulator->cmd_line_args, &file_report);
    } else {
        outcome = convert_one_file(filename, accumulator->cmd_line_args, &file_report);
    }

    source_convention = get_source_convention(&file_report);
    ++ accumulator->outcome_totals[outcome];
    if(outcome == DONE) {
        ++ accumulator->convention_totals[source_convention];
    }
    if(accumulator->cmd_line_args->verbose) {
        print_verbose_file_outcome(filename, outcome, source_convention);
    }
}

Accumulator
make_accumulator(CommandLine* cmd_line_args) {
    Accumulator a;
    for(int i=0; i<OUTCOMES_COUNT; ++i) {
        a.outcome_totals[i] = 0;
    }
    for(int i=0; i<CONVENTIONS_COUNT; ++i) {
        a.convention_totals[i] = 0;
    }
    a.cmd_line_args = cmd_line_args;
    return a;
}

Walk_tracker 
make_tracker(CommandLine* cmd_line_args, Accumulator* accumulator) {
    Walk_tracker t = make_default_walk_tracker(); // from walkers.h
    t.process_file = &walkers_callback;
    t.accumulator = accumulator;
    t.verbose = cmd_line_args->verbose;
    t.recurse = cmd_line_args->recurse;
    t.skip_hidden = !cmd_line_args->process_hidden;
    return t;
}

void
convert_files(int argc, char ** argv, CommandLine* cmd_line_args)  {
    Accumulator accumulator = make_accumulator(cmd_line_args);
    Walk_tracker tracker = make_tracker(cmd_line_args, &accumulator);

    if(!cmd_line_args->quiet) {
        if(cmd_line_args->convention == NO_CONVENTION) {
            fprintf(stderr, "endlines : dry run, scanning files\n");
        } else {
            fprintf(stderr, "endlines : converting files to %s\n", convention_display_names[cmd_line_args->convention]);
        }
    }

    walk_filenames(cmd_line_args->filenames, cmd_line_args->file_count, &tracker);

    if(!cmd_line_args->quiet) {
        print_outcome_totals(cmd_line_args->convention == NO_CONVENTION,
                             accumulator.convention_totals,
                             accumulator.outcome_totals[DONE],
                             tracker.skipped_directories_count,
                             accumulator.outcome_totals[SKIPPED_BINARY],
                             tracker.skipped_hidden_files_count,
                             accumulator.outcome_totals[SKIPPED_ERROR] + tracker.read_errors_count
        );
    }
}





// =============== ENTRY POINT ===============

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
            if(cmd_line_args.convention == NO_CONVENTION) {
                fprintf(stderr, "endlines : dry run, scanning standard input\n");
            } else {
                fprintf(stderr, "Converting standard input to %s\n", convention_display_names[cmd_line_args.convention]);
            }
        }
        engine_run(stdin, stdout, cmd_line_args.convention, false);
    }
    return 0;
}
