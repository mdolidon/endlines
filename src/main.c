/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   endlines : Mathias Dolidon / 2014 */


#include "endlines.h"
#include <string.h>
#include <sys/stat.h>


const char * version = "0.1.1";

// SPECIAL NOTE :
//   Missing posix function on OSX 10.7 : utimensat 
//   Timestamp overriding capabilities not implemented yet, may
//   implement a work-around later on.

// ENUMS AND CONSTANTS ARE DEFINED IN endlines.h
const char *convention_display_names[KNOWN_CONVENTIONS_COUNT]; 

void
setup_display_names() {
    convention_display_names[CR] = "Legacy Mac (CR)";
    convention_display_names[LF] = "Unix (LF)";
    convention_display_names[CRLF] = "Windows (CR-LF)";
}

typedef struct {
    char name[10];
    convention_t convention;
}
command_line_to_convention_t;

#define CL_NAMES_COUNT 11
const command_line_to_convention_t cl_names[CL_NAMES_COUNT] = {
    {.name="lf",      .convention=LF},
    {.name="unix",    .convention=LF},
    {.name="linux",   .convention=LF},
    {.name="crlf",    .convention=CRLF},
    {.name="win",     .convention=CRLF},
    {.name="windows", .convention=CRLF},
    {.name="dos",     .convention=CRLF},
    {.name="msdos",   .convention=CRLF},
    {.name="osx",     .convention=CRLF},
    {.name="cr",      .convention=CR},
    {.name="oldmac",  .convention=CR}
};


void
display_help_and_quit() {
    fprintf(stderr, "\n ------ Convert between line ending conventions. ------\n\n");
    fprintf(stderr, " Use :\n   endlines CONVENTION [OPTIONS] [FILES]\n\n");
    fprintf(stderr, "   Input conventions are determined automatically.\n   Each input file may possibly use multiple conventions. \n");
    fprintf(stderr, "   CONVENTION expresses the wished output convention.\n");
    fprintf(stderr, "   CONVENTION can be : ");
    for(int i=0; i<CL_NAMES_COUNT; i++) {
        fprintf(stderr, "%s ", cl_names[i].name);
    }
    fprintf(stderr, "\n   If no files are specified, endlines converts from stdin to stdout.\n\n");
    fprintf(stderr, " Options :\n");
    fprintf(stderr, "   -q / --quiet : silence all but the error messages.\n");
    fprintf(stderr, "   --version : print version number.\n\n");
    fprintf(stderr, " Example :\n");
    fprintf(stderr, "   endlines unix -q `find . -name \"*.html\"`\n\n");
    exit(1);
}


convention_t
read_convention_from_string(char * name) {
    for(int i=0; i<CL_NAMES_COUNT; i++) {
        if(!strcmp(cl_names[i].name, name)) {
            return cl_names[i].convention;
        }
    }
    fprintf(stderr, "endlines : unknown line end convention : %s\n", name);
    exit(8);
}


void
parse_options(int argc, char**argv, options_t * options) {
    int i;
    options->files = 0;
    options->quiet = false;
    for(i=1; i<argc; ++i) {
        if(i>1 && argv[i][0] != '-') {
            options->files++;
            continue;
        }
        else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            display_help_and_quit();
        }
        else if(!strcmp(argv[i], "--version")) {
            fprintf(stderr, "endlines version %s\n", version);
            exit(0);
        }
        else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) {
            options->quiet = true;
        }
        else if(i==1) {
            options->convention = read_convention_from_string(argv[1]);
        }
        else {
            fprintf(stderr, "endlines : unknown option : %s\n", argv[i]);
            exit(4);
        }
    }
}



void
convert_file(char *file_name, options_t * options) {
    struct stat statinfo;
    stat(file_name, &statinfo);
    if(S_ISDIR(statinfo.st_mode)) {
        return;
    }
    FILE * in = fopen(file_name, "rb");
    if(in == NULL) {
        fprintf(stderr, "endlines : could not read %s\n", file_name);
        return;
    }
    
    FILE * out = fopen(TMP_FILE_NAME, "wb");
    if(out == NULL) {
        fprintf(stderr, "endlines : could not create temporary file %s\n", TMP_FILE_NAME);
        fclose(in);
        return;
    }

    convert(in, out, options->convention);
    
    fclose(in);
    fclose(out);
    int remove_status = remove(file_name);
    if(remove_status) {
        fprintf(stderr, "endlines : can't write over %s\n", file_name);
        remove(TMP_FILE_NAME);
        return;
    }
    rename(TMP_FILE_NAME, file_name);
}



int
main(int argc, char**argv) {
    options_t options;
    if(argc<2) {
        fprintf(stderr, "Try endlines --help\n");
        exit(1);
    }
    setup_display_names();

    parse_options(argc, argv, &options);

    if(options.files) {
        if(!options.quiet) {
            fprintf(stderr, "Going to convert %i file%s to %s\n", options.files, options.files>1?"s":"", convention_display_names[options.convention]);
        }
        for(int i=2; i<argc; i++) {
            if(argv[i][0] != '-') {
                convert_file(argv[i], &options);
            }
        }
    }
    else {
        if(!options.quiet) {
            fprintf(stderr, "Converting stdin to %s\n", convention_display_names[options.convention]);
        }
        convert(stdin, stdout, options.convention);
    }

    return 0;
}
