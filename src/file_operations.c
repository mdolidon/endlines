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
#include <unistd.h>


struct utimbuf
get_file_times(struct stat *statinfo)
{
    struct utimbuf file_times;
    file_times.actime = statinfo->st_atime;
    file_times.modtime = statinfo->st_mtime;
    return file_times;
}


FileOp_Status
open_input_file_for_conversion(FILE **in, char *in_filename)
{
    *in = fopen(in_filename, "rb");
    if(*in == NULL) {
        fprintf(stdout, "%s : can not read %s\n", PROGRAM_NAME, in_filename);
        return FILEOP_ERROR;
    }
    if(access(in_filename, W_OK)) {
        fprintf(stdout, "%s : can not write over %s\n", PROGRAM_NAME, in_filename);
        fclose(*in);
        return FILEOP_ERROR;
    }
    return CAN_CONTINUE;
}


FileOp_Status
open_temporary_file(FILE **out, char *tmp_filename)
{
    *out = fopen(tmp_filename, "wb");
    if(*out == NULL) {
        fprintf(stdout, "%s : can not create %s\n", PROGRAM_NAME, tmp_filename);
        return FILEOP_ERROR;
    }
    return CAN_CONTINUE;
}


FileOp_Status
open_input_file_for_dry_run(FILE **in, char *in_filename)
{
    *in = fopen(in_filename, "rb");
    if(*in == NULL) {
        fprintf(stdout, "%s : can not read %s\n", PROGRAM_NAME, in_filename);
        return FILEOP_ERROR;
    }
    return CAN_CONTINUE;
}


FileOp_Status
move_temp_file_to_destination(char *tmp_filename, char *filename, struct stat *statinfo)
{
    int err = remove(filename);
    if(err) {
        fprintf(stdout, "%s : can not write over %s\n", PROGRAM_NAME, filename);
        remove(tmp_filename);
        return FILEOP_ERROR;
    }
    err = rename(tmp_filename, filename);
    if(err) {
        fprintf(stdout, "%s : can not restore %s\n"
                        "  -- Fail safe reaction : aborting.\n"
                        "  -- You will find your data in %s\n"
                        "  -- Please rename it manually to %s\n"
                        "  -- You may report this occurence at :\n"
                        "     https://github.com/mdolidon/endlines/issues\n",
                PROGRAM_NAME, filename, tmp_filename, filename);
        exit(EXIT_FAILURE);
    }
    err = chmod(filename, statinfo->st_mode);
    if(err) {
        fprintf(stdout, "%s : could not restore permissions for %s\n", PROGRAM_NAME, filename);
    }
    err = chown(filename, statinfo->st_uid, statinfo->st_gid);
    if(err) {
        fprintf(stdout, "%s : could not restore ownership for %s\n", PROGRAM_NAME, filename);
    }

    return CAN_CONTINUE;
}


FileOp_Status
make_filename_in_same_location(char *reference_name_and_path, char *wanted_name, char *destination)
{
    int reflen = (int)strlen(reference_name_and_path);
    if(reflen>=WALKERS_MAX_PATH_LENGTH) {
        fprintf(stdout, "%s : pathname exceeding maximum length : %s\n",
                PROGRAM_NAME, reference_name_and_path);
        return FILEOP_ERROR;
    }
    int filename_start = reflen;
    while(filename_start > 0) {
        filename_start --;
        if(reference_name_and_path[filename_start]=='/') {
            filename_start ++;
            break;
        }
    }
    int wanted_length = (int)strlen(wanted_name);
    if(wanted_length + filename_start + 1 >= WALKERS_MAX_PATH_LENGTH) {
        fprintf(stdout, "%s : pathname exceeding maximum length : %s on %s\n",
                PROGRAM_NAME, wanted_name, reference_name_and_path);
        return FILEOP_ERROR;
    }
    strcpy(destination, reference_name_and_path);
    strcpy(&(destination[filename_start]), wanted_name);
    return CAN_CONTINUE;
}

