/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

endlines : Mathias Dolidon / 2014-2015 */

#include "endlines.h"
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
} command_line_to_convention;

#define CL_NAMES_COUNT 10
const command_line_to_convention cl_names[CL_NAMES_COUNT] = {
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
    fprintf(stderr, "\n ------ Convert between line ending conventions. ------\n\n");
    fprintf(stderr, " Use :\n   endlines CONVENTION [OPTIONS] [FILES]\n\n");
    fprintf(stderr, "   Input conventions are determined automatically.\n   Each input file may possibly use multiple conventions. \n");
    fprintf(stderr, "   CONVENTION expresses the wished output convention.\n");
    fprintf(stderr, "   CONVENTION can be : ");
    for(int i=0; i<CL_NAMES_COUNT; ++i) {
        fprintf(stderr, "%s ", cl_names[i].name);
    }
    fprintf(stderr, "\n   If no files are specified, endlines converts from stdin to stdout.\n\n");
    fprintf(stderr, " Options :\n");
    fprintf(stderr, "   -q / --quiet    : silence all but the error messages.\n");
    fprintf(stderr, "   -v / --verbose  : print more about what's going on.\n");
    fprintf(stderr, "   -b / --binaries : don't skip binary files.\n");
    fprintf(stderr, "   -k / --keepdate : keep files' last modified and last access time stamps.\n");
    fprintf(stderr, "   --version       : print version number.\n\n");
    fprintf(stderr, " Example :\n");
    fprintf(stderr, "   endlines unix -k `find . -name \"*.html\"`\n\n");
    exit(1);
}



//
// PARSING COMMAND LINE OPTIONS
// Yes it's a huge and ugly switch
//

typedef struct {
    Convention convention;
    int files;
    bool quiet;
    bool verbose;
    bool binaries;
    bool keepdate;
} AppOptions;

AppOptions
parse_options(int argc, char**argv) {
    AppOptions options = {.files=0, .quiet=false, .binaries=false, .keepdate=false, .verbose=false};
    int i;
    for(i=1; i<argc; ++i) {
        if(i>1 && argv[i][0] != '-') {
            ++ options.files;
            continue;
        } else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            display_help_and_quit();
        } else if(!strcmp(argv[i], "--version")) {
            fprintf(stderr, "endlines version %s\n", VERSION);
            exit(0);
        } else if(i==1) {
            options.convention = read_convention_from_string(argv[1]);
        } else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) {
            options.quiet = true;
        } else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            options.verbose = true;
        } else if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--binaries")) {
            options.binaries = true;
        } else if(!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keepdate")) {
            options.keepdate = true;
        } else {
            fprintf(stderr, "endlines : unknown option : %s\n", argv[i]);
            exit(4);
        }
    }
    return options;
}



//
// CONVERTING ONE FILE
//

#define PROCESSING_STATUSES_COUNT 5
typedef enum {
    CAN_CONTINUE,
    DONE,
    SKIPPED_BINARY,
    SKIPPED_DIRECTORY,
    SKIPPED_ERROR,
} processing_status;


processing_status
get_file_stats(char *file_name, struct stat* statinfo) {
    if(stat(file_name, statinfo)) {
        fprintf(stderr, "endlines : can not read %s\n", file_name);
        return SKIPPED_ERROR;
    } else if(S_ISDIR(statinfo->st_mode)) {
        return SKIPPED_DIRECTORY;
    } else {
        return CAN_CONTINUE;
    }
} 

struct utimbuf
get_file_times(struct stat* statinfo) {
    struct utimbuf file_times;
    file_times.actime = statinfo->st_atime;
    file_times.modtime = statinfo->st_mtime;
    return file_times;
}

processing_status
open_files(FILE** in, char* in_file_name, FILE** out, char* out_file_name) {
    *in = fopen(in_file_name, "rb");
    if(*in == NULL) {
        fprintf(stderr, "endlines : can not read %s\n", in_file_name);
        return SKIPPED_ERROR;
    }
    if(access(in_file_name, W_OK)) {
        fprintf(stderr, "endlines : can not write over %s\n", in_file_name);
        fclose(*in);
        return SKIPPED_ERROR;
    }
    *out = fopen(TMP_FILE_NAME, "wb");
    if(*out == NULL) {
        fprintf(stderr, "endlines : can not create %s\n", out_file_name);
        fclose(*in);
        return SKIPPED_ERROR;
    }
    return CAN_CONTINUE;
}

processing_status
move_temp_file_to_destination(char* file_name) {
    int err = remove(file_name);
    if(err) {
        fprintf(stderr, "endlines : can not write over %s\n", file_name);
        remove(TMP_FILE_NAME);
        return SKIPPED_ERROR;
    }
    rename(TMP_FILE_NAME, file_name);
    return CAN_CONTINUE;
}


#define TRY partial_status =
#define CATCH if(partial_status != CAN_CONTINUE) { return partial_status; }

processing_status
convert_one_file(char *file_name, AppOptions * options) {
    struct stat statinfo;
    processing_status partial_status;
    FILE *in  = NULL;
    FILE *out = NULL;

    TRY get_file_stats(file_name, &statinfo); CATCH
    struct utimbuf original_file_times = get_file_times(&statinfo);
    TRY open_files(&in, file_name, &out, TMP_FILE_NAME); CATCH

    Report report = convert(in, out, options->convention);

    fclose(in);
    fclose(out);

    if(report.contains_control_chars && !options->binaries) {
        remove(TMP_FILE_NAME);
        return SKIPPED_BINARY;
    }

    TRY move_temp_file_to_destination(file_name); CATCH

    if(options->keepdate) {
        utime(file_name, &original_file_times);
    }
    return DONE;
}

#undef TRY
#undef CATCH



//
// HANDLING A CONVERSION BATCH
//

void
print_verbose_file_outcome(char * file_name, processing_status outcome) {
    switch(outcome) {
        case DONE: fprintf(stderr, "endlines : converted %s\n", file_name);
            break;
        case SKIPPED_DIRECTORY: fprintf(stderr, "endlines : skipped directory %s\n", file_name);
            break;
        case SKIPPED_BINARY: fprintf(stderr, "endlines : skipped probable binary %s\n", file_name);
            break;
        default: break;
    }
}

void
print_totals(int done, int directories, int binaries, int errors) {
        fprintf(stderr,     "endlines : %i file%s converted\n", done, done>1?"s":"");
        if(directories) {
            fprintf(stderr, "           %i director%s skipped\n", directories, directories>1?"ies":"y");
        }
        if(binaries) {
            fprintf(stderr, "           %i binar%s skipped\n", binaries, binaries>1?"ies":"y");
        }
        if(errors) {
            fprintf(stderr, "           %i error%s\n", errors, errors>1?"s":"");
        }
}


void
convert_files(int argc, char ** argv, AppOptions* options)  {
    int totals[PROCESSING_STATUSES_COUNT] = {0,0,0,0,0};
    processing_status outcome;
    if(!options->quiet) {
        fprintf(stderr, "endlines : converting files to %s\n", convention_display_names[options->convention]);
    }
    for(int i=2; i<argc; ++i) {
        if(argv[i][0] != '-') {
            outcome = convert_one_file(argv[i], options);
            ++ totals[outcome];
            if(options->verbose) {
                print_verbose_file_outcome(argv[i], outcome);
            } 
        }
    }
    if(!options->quiet) {
        print_totals(totals[DONE],
                     totals[SKIPPED_DIRECTORY],
                     totals[SKIPPED_BINARY],
                     totals[SKIPPED_ERROR]);
    }
}



//
// ENTRY POINT
//

int
main(int argc, char**argv) {
    if(argc<2) {
        display_help_and_quit();
    }

    setup_conventions_display_names();
    AppOptions options = parse_options(argc, argv);

    if(options.files) {
        convert_files(argc, argv, &options);
    }
    else {
        if(!options.quiet) {
            fprintf(stderr, "Converting stdin to %s\n", convention_display_names[options.convention]);
        }
        convert(stdin, stdout, options.convention);
    }

    return 0;
}
