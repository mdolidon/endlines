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


#include "command_line_parser.h"
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
    bool dst_convention_specified;
    Convention dst_convention;
    bool quiet;
    bool verbose;
    bool binaries;
    bool keepdate;
    bool recurse;
    bool process_hidden;
    bool final_char_has_to_be_eol;
    char **filenames;
    int file_count;
} Invocation;




// An accumulator that is passed around by the walkers, to the walkers_callback function
// Its main use is to keep track of what has been done
// It is complemented by the walkers' tracker object, defined in walkers.h,
// that'll hold results that are specific to the walker (e.g. skipped directories and hidden files)

typedef struct {
    int outcome_totals[FILEOP_STATUSES_COUNT];
    int convention_totals[CONVENTIONS_COUNT];
    Invocation *invocation;
} Batch_outcome_accumulator;




// =============== ALL ABOUT CONVENTION NAMES ===============

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
const int cl_names_count =
        (int)(sizeof(cl_names)/sizeof(cl_names[0]));

Convention
read_convention_from_string(const char *name)
{
    for(int i=0; i<cl_names_count; ++i) {
        if(!strcmp(cl_names[i].name, name)) {
            return cl_names[i].convention;
        }
    }
    fprintf(stderr, "%s : unknown action : %s\n", PROGRAM_NAME, name);
    exit(EXIT_FAILURE);
}

#define X(a,b,c) b,
const char *convention_display_names[CONVENTIONS_COUNT] = {CONVENTIONS_TABLE};
#undef X
#define X(a,b,c) c,
const char *convention_short_display_names[CONVENTIONS_COUNT] = {CONVENTIONS_TABLE};
#undef X


// =============== PARSING COMMAND LINE OPTIONS ===============

void
got_help_flag(const char *arg, void *context)
{
    display_help_and_quit();
}

void
got_version_flag(const char *arg, void *context)
{
    display_version_and_quit();
}

void
got_quiet_flag(const char *arg, void *context)
{
    ((Invocation *)context)->quiet = true;
}

void
got_verbose_flag(const char *arg, void *context)
{
    ((Invocation *)context)->verbose = true;
}

void
got_process_binaries_flag(const char *arg, void *context)
{
    ((Invocation *)context)->binaries = true;
}

void
got_keepdate_flag(const char *arg, void *context)
{
    ((Invocation *)context)->keepdate = true;
}

void
got_recurse_flag(const char *arg, void *context)
{
    ((Invocation *)context)->recurse = true;
}

void
got_process_hidden_flag(const char *arg, void *context)
{
    ((Invocation *)context)->process_hidden = true;
}

void
got_final_char_has_to_be_eol_flag(const char *arg, void *context)
{
    ((Invocation *)context)->final_char_has_to_be_eol = true;
}

void
got_non_flag_arg(char *argument, int arg_index, void *context)
{
    if(!((Invocation *)context)->dst_convention_specified) {
        ((Invocation *)context)->dst_convention = read_convention_from_string(argument);
        ((Invocation *)context)->dst_convention_specified = true;
    } else {
        ((Invocation *)context)->filenames[ ((Invocation *)context)->file_count ] = argument;
        ((Invocation *)context)->file_count ++;
    }
}

Invocation
parse_endlines_command_line(int argc, char **argv)
{
    Command_Line_Schema* command_line_schema = new_command_line_schema(PROGRAM_NAME);

    Command_Line_Flag flags[] = {
      {.short_flag=0,   .long_flag="help",     .callback=got_help_flag},
      {.short_flag=0,   .long_flag="version",  .callback=got_version_flag},
      {.short_flag='f', .long_flag="final",    .callback=got_final_char_has_to_be_eol_flag},
      {.short_flag='q', .long_flag="quiet",    .callback=got_quiet_flag},
      {.short_flag='v', .long_flag="verbose",  .callback=got_verbose_flag},
      {.short_flag='k', .long_flag="keepdate", .callback=got_keepdate_flag},
      {.short_flag='b', .long_flag="binaries", .callback=got_process_binaries_flag},
      {.short_flag='r', .long_flag="recurse",  .callback=got_recurse_flag},
      {.short_flag='h', .long_flag="hidden",   .callback=got_process_hidden_flag}
    };
    const int flags_count = (int)(sizeof(flags)/sizeof(flags[0]));
    set_flag_descriptions(command_line_schema, flags, flags_count);
    set_non_flag_arg_callback(command_line_schema, got_non_flag_arg);

    Invocation cmd_line_invocation = {
        .dst_convention=NO_CONVENTION,
        .dst_convention_specified=false,
        .quiet=false, .binaries=false,
        .keepdate=false, .verbose=false,
        .recurse=false, .process_hidden=false,
        .final_char_has_to_be_eol=false,
        .filenames=NULL, .file_count=0
    };

    cmd_line_invocation.filenames = malloc(argc*sizeof(void*));
    if(cmd_line_invocation.filenames == NULL) {
        fprintf(stderr, "%s : can't allocate memory\n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    parse_command_line(argc, argv, command_line_schema, &cmd_line_invocation);
    destroy_command_line_schema(command_line_schema);
    if(! cmd_line_invocation.dst_convention_specified) {
        fprintf(stderr, "%s : you need to specify an action. See %s --help\n", PROGRAM_NAME, PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }
    return cmd_line_invocation;
}

void destroy_invocation_on_stack(Invocation *i)
{
    free(i->filenames);
}




// =============== CONVERTING OR CHECKING ONE FILE ===============


// Main conversion (resp. checking) handlers

#define TRY partial_status =
#define CATCH if(partial_status != CAN_CONTINUE) { return partial_status; }
#define CATCH_CLOSE_IN if(partial_status != CAN_CONTINUE) { fclose(in); return partial_status; }


// Make up once a file name for all tmp file creations from this process.
void
initialize_session_tmp_filename(char *session_tmp_filename)
{
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
pre_conversion_check(FILE *in, char *filename,
                     Conversion_Report *file_report,
                     Invocation *invocation)
{
    Conversion_Parameters p = {
        .instream=in,
        .outstream=NULL,
        .dst_convention=invocation->dst_convention,
        .interrupt_if_not_like_dst_convention=true,
        .interrupt_if_non_text=!invocation->binaries,
        .final_char_has_to_be_eol=false
    };
    Conversion_Report preliminary_report = convert_stream(p);

    if(preliminary_report.error_during_conversion) {
        fprintf(stdout, "%s : file access error during preliminary check of %s\n",
                PROGRAM_NAME, filename);
        return FILEOP_ERROR;
    }
    if(preliminary_report.contains_non_text_chars && !invocation->binaries) {
        return SKIPPED_BINARY;
    }
    Convention src_convention = get_source_convention(&preliminary_report);
    if((src_convention == NO_CONVENTION && !invocation->final_char_has_to_be_eol) ||
        src_convention == invocation->dst_convention) {
        memcpy(file_report, &preliminary_report, sizeof(Conversion_Report));
        return DONE;
    }
    return CAN_CONTINUE;
}


FileOp_Status
convert_one_file(char *filename, struct stat *statinfo,
        Invocation *invocation,
        Conversion_Report *file_report)
{
    FileOp_Status partial_status;
    FILE *in  = NULL;
    FILE *out = NULL;
    static char session_tmp_filename[40] = "";
    if(session_tmp_filename[0]==0) {
        initialize_session_tmp_filename(session_tmp_filename);
    }
    char local_tmp_file_name[WALKERS_MAX_PATH_LENGTH];
    struct utimbuf original_file_times = get_file_times(statinfo);

    TRY open_input_file_for_conversion(&in, filename); CATCH
    TRY pre_conversion_check(in, filename, file_report, invocation); CATCH_CLOSE_IN
    rewind(in);
    TRY make_filename_in_same_location(filename, session_tmp_filename, local_tmp_file_name); CATCH_CLOSE_IN
    TRY open_temporary_file(&out, local_tmp_file_name); CATCH_CLOSE_IN

    Conversion_Parameters p = {
        .instream=in,
        .outstream=out,
        .dst_convention=invocation->dst_convention,
        .interrupt_if_not_like_dst_convention=false,
        .interrupt_if_non_text=!invocation->binaries,
        .final_char_has_to_be_eol=invocation->final_char_has_to_be_eol
    };
    Conversion_Report report = convert_stream(p);

    fclose(in);
    fclose(out);

    if(report.error_during_conversion) {
        remove(local_tmp_file_name);
        fprintf(stdout, "%s : file access error during conversion of %s\n", PROGRAM_NAME, filename);
        return FILEOP_ERROR;
    }
    if(report.contains_non_text_chars && !invocation->binaries) {
        remove(local_tmp_file_name);
        return SKIPPED_BINARY;
    }

    TRY move_temp_file_to_destination(local_tmp_file_name, filename, statinfo); CATCH

    if(invocation->keepdate) {
        utime(filename, &original_file_times);
    }
    memcpy(file_report, &report, sizeof(Conversion_Report));
    return DONE;
}


FileOp_Status
check_one_file(char *filename, Invocation *invocation, Conversion_Report *file_report)
{
    FileOp_Status partial_status;
    FILE *in  = NULL;
    TRY open_input_file_for_dry_run(&in, filename); CATCH

    Conversion_Parameters p = {
        .instream=in,
        .outstream=NULL,
        .dst_convention=NO_CONVENTION,
        .interrupt_if_not_like_dst_convention=false,
        .interrupt_if_non_text=!invocation->binaries,
        .final_char_has_to_be_eol=false
    };
    Conversion_Report report = convert_stream(p);

    fclose(in);

    if(report.error_during_conversion) {
        fprintf(stdout, "%s : file access error during check of %s\n", PROGRAM_NAME, filename);
        return FILEOP_ERROR;
    }
    if(report.contains_non_text_chars && !invocation->binaries) {
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
print_verbose_file_outcome(char *filename, FileOp_Status outcome, Convention source_convention)
{
    switch(outcome) {
    case DONE:
        fprintf(stdout, "%s : %s -- %s\n",
                PROGRAM_NAME,
                convention_short_display_names[source_convention], filename);
        break;
    case SKIPPED_BINARY:
        fprintf(stdout, "%s : skipped probable binary %s\n", PROGRAM_NAME, filename);
        break;
    default: break;
    }
}


void
print_outcome_totals(Outcome_totals_for_display t)
{
    fprintf(stdout,  "\n%s : %i file%s %s", PROGRAM_NAME, t.done,
            t.done>1?"s":"", t.dry_run?"checked":"converted");

    if(t.done) {
        fprintf(stdout, " %s :\n", t.dry_run?"; found":"from");
        for(int i=0; i<CONVENTIONS_COUNT; ++i) {
            if(t.count_by_convention[i]) {
                fprintf(stdout, "              - %i %s\n",
                        t.count_by_convention[i], convention_display_names[i]);
            }
        }
    } else {
        fprintf(stdout, "\n");
    }
    if(t.directories) {
        fprintf(stdout, "           %i director%s skipped\n",
                t.directories, t.directories>1?"ies":"y");
    }
    if(t.binaries) {
        fprintf(stdout, "           %i binar%s skipped\n",
                t.binaries, t.binaries>1?"ies":"y");
    }
    if(t.hidden) {
        fprintf(stdout, "           %i hidden file%s skipped\n",
                t.hidden, t.hidden>1?"s":"");
    }
    if(t.errors) {
        fprintf(stdout, "           %i error%s\n",
                t.errors, t.errors>1?"s":"");
    }
    fprintf(stdout, "\n");
}


void
walkers_callback(char *filename, struct stat *statinfo, void *p_accumulator)
{
    FileOp_Status outcome;
    Conversion_Report file_report;
    Convention source_convention = NO_CONVENTION;
    Batch_outcome_accumulator *accumulator = (Batch_outcome_accumulator*) p_accumulator;

    if(!accumulator->invocation->binaries &&
            has_known_binary_file_extension(filename)) {
        outcome = SKIPPED_BINARY;
    } else if(accumulator->invocation->dst_convention == NO_CONVENTION) {
        outcome = check_one_file(filename, accumulator->invocation, &file_report);
    } else {
        outcome = convert_one_file(filename, statinfo, accumulator->invocation, &file_report);
    }
    if(outcome == DONE) {
        source_convention = get_source_convention(&file_report);
        ++ accumulator->convention_totals[source_convention];
    }
    ++ accumulator->outcome_totals[outcome];
    if(accumulator->invocation->verbose) {
        print_verbose_file_outcome(filename, outcome, source_convention);
    }
}


Batch_outcome_accumulator
make_accumulator(Invocation *invocation)
{
    Batch_outcome_accumulator a;
    for(int i=0; i<FILEOP_STATUSES_COUNT; ++i) {
        a.outcome_totals[i] = 0;
    }
    for(int i=0; i<CONVENTIONS_COUNT; ++i) {
        a.convention_totals[i] = 0;
    }
    a.invocation = invocation;
    return a;
}


Walk_tracker
make_tracker(Invocation *invocation, Batch_outcome_accumulator *accumulator)
{
    Walk_tracker t = make_default_walk_tracker();

    t.process_file = &walkers_callback;
    t.accumulator = accumulator;
    t.verbose = invocation->verbose;
    t.recurse = invocation->recurse;
    t.skip_hidden = !invocation->process_hidden;
    return t;
}


void
convert_files(Invocation *invocation)
{
    Batch_outcome_accumulator accumulator = make_accumulator(invocation);
    Walk_tracker tracker = make_tracker(invocation, &accumulator);

    if(!invocation->quiet) {
        if(invocation->dst_convention == NO_CONVENTION) {
            fprintf(stdout, "%s : dry run, scanning files\n", PROGRAM_NAME);
        } else {
            fprintf(stdout, "%s : converting files to %s\n",
                    PROGRAM_NAME,
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

void print_stream_conversion_outcome(Conversion_Parameters *parameters, Conversion_Report *report)
{
    Convention source_convention = get_source_convention(report);
    if(parameters->dst_convention == NO_CONVENTION) {
        char *binary_comment = report->contains_non_text_chars ? "looked like a binary stream and " : "";
        fprintf(stderr, "%s : stdin %shad line endings in %s\n",
                PROGRAM_NAME, binary_comment,
                convention_display_names[source_convention]);
    } else {
        char *binary_comment = report->contains_non_text_chars ? "(looked like a binary stream) " : "";
        fprintf(stderr, "%s : converted from %s in stdin %sto %s in stdout\n",
                PROGRAM_NAME,
                convention_display_names[source_convention],
                binary_comment,
                convention_display_names[parameters->dst_convention]);
    }
}

void convert_stdin_to_stdout(Invocation *invocation)
{
    if(!invocation->quiet) {
        if(invocation->dst_convention == NO_CONVENTION) {
            fprintf(stderr, "%s : dry run, scanning standard input\n", PROGRAM_NAME);
        } else {
            fprintf(stderr, "%s : converting standard input to %s\n",
                    PROGRAM_NAME,
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
main(int argc, char**argv)
{
    if(argc <= 1) {
        display_help_and_quit();
    }
    Invocation cmd_line_invocation = parse_endlines_command_line(argc, argv);
    if(cmd_line_invocation.file_count > 0) {
        convert_files(&cmd_line_invocation);
    } else {
        convert_stdin_to_stdout(&cmd_line_invocation);
    }
    destroy_invocation_on_stack(&cmd_line_invocation);
    return EXIT_SUCCESS;
}

