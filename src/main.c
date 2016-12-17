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
#include <sys/types.h>
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
    Convention dst_convention;
    bool quiet;
    bool verbose;
    bool binaries;
    bool keepdate;
    bool recurse;
    bool process_hidden;
    char** filenames;
    int file_count;
} Invocation;




// An accumulator that is passed around by the walkers, to the walkers_callback function
// Its main use is to keep track of what has been done
// It is complemented by the walkers' tracker object, defined in walkers.h,
// that'll hold results that are specific to the walker (e.g. skipped directories and hidden files)

typedef struct {
    int outcome_totals[FILEOP_STATUSES_COUNT];
    int convention_totals[CONVENTIONS_COUNT];
    Invocation* invocation;
} Batch_outcome_accumulator;




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
    exit(EXIT_FAILURE);
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


void setup_constants() {
    setup_conventions_display_names();
}


// =============== PARSING COMMAND LINE OPTIONS ===============
// Yes it's a huge and ugly switch

Invocation
parse_command_line(int argc, char** argv) {
    Invocation cmd_line_invocation = {.quiet=false, .binaries=false, .keepdate=false, .verbose=false,
    .recurse=false, .process_hidden=false, .filenames=NULL, .file_count=0};

    cmd_line_invocation.filenames = malloc(argc*sizeof(void*));
    if(cmd_line_invocation.filenames == NULL) {
        fprintf(stderr, "endlines : can't allocate memory\n");
        exit(EXIT_FAILURE);
    }

    for(int i=1; i<argc; ++i) {
        if(i>1 && argv[i][0] != '-') {
            cmd_line_invocation.filenames[cmd_line_invocation.file_count] = argv[i];
            ++ cmd_line_invocation.file_count;
        } else if(!strcmp(argv[i], "--help")) {
            display_help_and_quit();
        } else if(!strcmp(argv[i], "--version")) {
            display_version_and_quit();
        } else if(i==1) {
            cmd_line_invocation.dst_convention = read_convention_from_string(argv[1]);
        } else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) {
            cmd_line_invocation.quiet = true;
        } else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            cmd_line_invocation.verbose = true;
        } else if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--binaries")) {
            cmd_line_invocation.binaries = true;
        } else if(!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keepdate")) {
            cmd_line_invocation.keepdate = true;
        } else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--recurse")) {
            cmd_line_invocation.recurse = true;
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--hidden")) {
            cmd_line_invocation.process_hidden = true;
        } else {
            fprintf(stderr, "endlines : unknown option : %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    if(cmd_line_invocation.file_count == 0) {
        free(cmd_line_invocation.filenames);
    }

    return cmd_line_invocation;
}





// =============== CONVERTING OR CHECKING ONE FILE ===============


// Main conversion (resp. checking) handlers

#define TRY partial_status =
#define CATCH if(partial_status != CAN_CONTINUE) { return partial_status; }
#define CATCH_CLOSE_IN if(partial_status != CAN_CONTINUE) { fclose(in); return partial_status; }


// Make up once a file name for all tmp file creations from this process.
void
initialize_session_tmp_filename(char* session_tmp_filename) {
    struct {
        long safety_padding_a;
        pid_t pid;
        long safety_padding_b;
    } pid_holder;

    pid_holder.pid = getpid();

    int suffix = (int)(((long)pid_holder.pid) % 9999999);
    sprintf(session_tmp_filename, "%s%d", TMP_FILENAME_BASE, suffix);
}

FileOp_Status
pre_conversion_check(
        FILE* in,
        char* filename,
        Conversion_Report* file_report,
        Invocation* cmd_line_args) {

    Conversion_Parameters p = {
        .instream=in,
        .outstream=NULL,
        .dst_convention=cmd_line_args->dst_convention,
        .interrupt_if_not_like_dst_convention=true,
        .interrupt_if_non_text=!cmd_line_args->binaries
    };

    Conversion_Report preliminary_report = convert_stream(p);

    if(preliminary_report.error_during_conversion) {
        fprintf(stderr, "endlines : file access error during preliminary check of %s\n", filename);
        return FILEOP_ERROR;
    }

    if(preliminary_report.contains_non_text_chars && !cmd_line_args->binaries) {
        return SKIPPED_BINARY;
    }
    Convention src_convention = get_source_convention(&preliminary_report);
    if(src_convention == NO_CONVENTION || src_convention == cmd_line_args->dst_convention) {
        memcpy(file_report, &preliminary_report, sizeof(Conversion_Report));
        return DONE;
    }
    return CAN_CONTINUE;
}


FileOp_Status
convert_one_file(
        char* filename,
        struct stat* statinfo,
        Invocation* cmd_line_args,
        Conversion_Report* file_report) {


    FileOp_Status partial_status;
    FILE *in  = NULL;
    FILE *out = NULL;

    static char session_tmp_filename[40] = "";
    char local_tmp_file_name[WALKERS_MAX_PATH_LENGTH];
    struct utimbuf original_file_times = get_file_times(statinfo);

    if(session_tmp_filename[0]==0) {
        initialize_session_tmp_filename(session_tmp_filename);
    }

    TRY open_input_file_for_conversion(&in, filename); CATCH
    TRY pre_conversion_check(in, filename, file_report, cmd_line_args); CATCH_CLOSE_IN
    rewind(in);
    int tmp_path_err = make_filename_in_same_location(filename, session_tmp_filename, local_tmp_file_name);
    if(tmp_path_err) {
        fclose(in);
        return FILEOP_ERROR;
    }
    TRY open_temporary_file(&out, local_tmp_file_name); CATCH_CLOSE_IN

    Conversion_Parameters p = {
        .instream=in,
        .outstream=out,
        .dst_convention=cmd_line_args->dst_convention,
        .interrupt_if_not_like_dst_convention=false,
        .interrupt_if_non_text=!cmd_line_args->binaries
    };
    Conversion_Report report = convert_stream(p);

    fclose(in);
    fclose(out);

    if(report.error_during_conversion) {
        fprintf(stderr, "endlines : file access error during conversion of %s\n", filename);
        return FILEOP_ERROR;
    }
    if(report.contains_non_text_chars && !cmd_line_args->binaries) {
        remove(local_tmp_file_name);
        return SKIPPED_BINARY;
    }

    TRY move_temp_file_to_destination(local_tmp_file_name, filename, statinfo); CATCH

    if(cmd_line_args->keepdate) {
        utime(filename, &original_file_times);
    }

    memcpy(file_report, &report, sizeof(Conversion_Report));
    return DONE;
}


FileOp_Status
check_one_file(char* filename, Invocation* cmd_line_args, Conversion_Report* file_report) {
    FileOp_Status partial_status;
    FILE *in  = NULL;
    TRY open_input_file_for_dry_run(&in, filename); CATCH

    Conversion_Parameters p = {
        .instream=in,
        .outstream=NULL,
        .dst_convention=NO_CONVENTION,
        .interrupt_if_not_like_dst_convention=false,
        .interrupt_if_non_text=!cmd_line_args->binaries
    };
    Conversion_Report report = convert_stream(p);

    fclose(in);

    if(report.error_during_conversion) {
        fprintf(stderr, "endlines : file access error during check of %s\n", filename);
        return FILEOP_ERROR;
    }
    if(report.contains_non_text_chars && !cmd_line_args->binaries) {
        return SKIPPED_BINARY;
    }

    memcpy(file_report, &report, sizeof(Conversion_Report));
    return DONE;
}

#undef TRY
#undef CATCH





// =============== HANDLING A CONVERSION BATCH ===============


typedef struct {
    bool dry_run;
    int *count_by_convention;  // array
    int done;
    int directories;
    int binaries;
    int hidden;
    int errors;
} Outcome_totals_for_display;

void
print_verbose_file_outcome(char * filename, FileOp_Status outcome, Convention source_convention) {
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
print_outcome_totals(Outcome_totals_for_display t) {
    fprintf(stderr,  "\nendlines : %i file%s %s", t.done,
            t.done>1?"s":"", t.dry_run?"checked":"converted");

    if(t.done) {
        fprintf(stderr, " %s :\n", t.dry_run?"; found":"from");
        for(int i=0; i<CONVENTIONS_COUNT; ++i) {
            if(t.count_by_convention[i]) {
                fprintf(stderr, "              - %i %s\n",
                        t.count_by_convention[i], convention_display_names[i]);
            }
        }
    } else {
        fprintf(stderr, "\n");
    }

    if(t.directories) {
        fprintf(stderr, "           %i director%s skipped\n",
                t.directories, t.directories>1?"ies":"y");
    }
    if(t.binaries) {
        fprintf(stderr, "           %i binar%s skipped\n",
                t.binaries, t.binaries>1?"ies":"y");
    }
    if(t.hidden) {
        fprintf(stderr, "           %i hidden file%s skipped\n",
                t.hidden, t.hidden>1?"s":"");
    }
    if(t.errors) {
        fprintf(stderr, "           %i error%s\n",
                t.errors, t.errors>1?"s":"");
    }
    fprintf(stderr, "\n");
}

void
walkers_callback(char* filename, struct stat* statinfo, void* p_accumulator) {
    FileOp_Status outcome;
    Conversion_Report file_report;
    Convention source_convention;
    Batch_outcome_accumulator* accumulator = (Batch_outcome_accumulator*) p_accumulator;

    if(!accumulator->invocation->binaries &&
            has_known_binary_file_extension(filename)) {
        outcome = SKIPPED_BINARY;
    } else if(accumulator->invocation->dst_convention == NO_CONVENTION) {
        outcome = check_one_file(filename, accumulator->invocation, &file_report);
    } else {
        outcome = convert_one_file(filename, statinfo, accumulator->invocation, &file_report);
    }

    source_convention = get_source_convention(&file_report);
    ++ accumulator->outcome_totals[outcome];
    if(outcome == DONE) {
        ++ accumulator->convention_totals[source_convention];
    }
    if(accumulator->invocation->verbose) {
        print_verbose_file_outcome(filename, outcome, source_convention);
    }
}

Batch_outcome_accumulator
make_accumulator(Invocation* cmd_line_args) {
    Batch_outcome_accumulator a;
    for(int i=0; i<FILEOP_STATUSES_COUNT; ++i) {
        a.outcome_totals[i] = 0;
    }
    for(int i=0; i<CONVENTIONS_COUNT; ++i) {
        a.convention_totals[i] = 0;
    }
    a.invocation = cmd_line_args;
    return a;
}

Walk_tracker
make_tracker(Invocation* cmd_line_args, Batch_outcome_accumulator* accumulator) {
    Walk_tracker t = make_default_walk_tracker();

    t.process_file = &walkers_callback;
    t.accumulator = accumulator;
    t.verbose = cmd_line_args->verbose;
    t.recurse = cmd_line_args->recurse;
    t.skip_hidden = !cmd_line_args->process_hidden;
    return t;
}

void
convert_files(Invocation* invocation)  {
    Batch_outcome_accumulator accumulator = make_accumulator(invocation);
    Walk_tracker tracker = make_tracker(invocation, &accumulator);

    if(!invocation->quiet) {
        if(invocation->dst_convention == NO_CONVENTION) {
            fprintf(stderr, "endlines : dry run, scanning files\n");
        } else {
            fprintf(stderr, "endlines : converting files to %s\n",
                    convention_display_names[invocation->dst_convention]);
        }
    }

    walk_filenames(invocation->filenames, invocation->file_count, &tracker);

    if(!invocation->quiet) {
        Outcome_totals_for_display totals = {
            .dry_run     = (invocation->dst_convention == NO_CONVENTION),
            .count_by_convention = accumulator.convention_totals,
            .done        = accumulator.outcome_totals[DONE],
            .directories = tracker.skipped_directories_count,
            .binaries    = accumulator.outcome_totals[SKIPPED_BINARY],
            .hidden      = tracker.skipped_hidden_files_count,
            .errors      = accumulator.outcome_totals[FILEOP_ERROR] + tracker.read_errors_count
        };
        print_outcome_totals(totals);
    }
}

// ============== HANDLING THE CONVERSION OF STANDARD STREAMS ===============

void print_stream_conversion_outcome(Conversion_Parameters *parameters, Conversion_Report *report) {
    Convention source_convention = get_source_convention(report);
    if(parameters->dst_convention == NO_CONVENTION) {
        char *binary_comment = report->contains_non_text_chars ? "looked like a binary stream and " : "";
        fprintf(stderr, "endlines : stdin %shad line endings in %s\n",
                binary_comment,
                convention_display_names[source_convention]);
    } else {
        char *binary_comment = report->contains_non_text_chars ? "(looked like a binary stream) " : "";
        fprintf(stderr, "endlines : converted from %s in stdin %sto %s in stdout\n",
                convention_display_names[source_convention],
                binary_comment,
                convention_display_names[parameters->dst_convention]);
    }
}

void convert_stdin_to_stdout(Invocation *invocation) {
    if(!invocation->quiet) {
        if(invocation->dst_convention == NO_CONVENTION) {
            fprintf(stderr, "endlines : dry run, scanning standard input\n");
        } else {
            fprintf(stderr, "endlines : converting standard input to %s\n",
                    convention_display_names[invocation->dst_convention]);
        }
    }
    Conversion_Parameters p = {
        .instream=stdin,
        .outstream= invocation->dst_convention==NO_CONVENTION ? NULL : stdout,
        .dst_convention=invocation->dst_convention,
        .interrupt_if_non_text=false
    };
    Conversion_Report report = convert_stream(p);
    if(!invocation->quiet) {
        print_stream_conversion_outcome(&p, &report);
    }
}


// =============== ENTRY POINT ===============

int
main(int argc, char**argv) {
    if(argc <= 1) {
        display_help_and_quit();
    }

    setup_constants();

    Invocation cmd_line_invocation = parse_command_line(argc, argv);

    if(cmd_line_invocation.file_count > 0) {
        convert_files(&cmd_line_invocation);
    } else {
        convert_stdin_to_stdout(&cmd_line_invocation);
    }
    return 0;
}

