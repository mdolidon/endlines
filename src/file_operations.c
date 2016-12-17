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

#include <stdlib.h>
#include <unistd.h>


struct utimbuf
get_file_times(struct stat* statinfo) {
    struct utimbuf file_times;
    file_times.actime = statinfo->st_atime;
    file_times.modtime = statinfo->st_mtime;
    return file_times;
}


FileOp_Status
open_input_file_for_conversion(FILE** in, char* in_filename) {
    *in = fopen(in_filename, "rb");
    if(*in == NULL) {
        fprintf(stderr, "endlines : can not read %s\n", in_filename);
        return FILEOP_ERROR;
    }
    if(access(in_filename, W_OK)) {
        fprintf(stderr, "endlines : can not write over %s\n", in_filename);
        fclose(*in);
        return FILEOP_ERROR;
    }
    return CAN_CONTINUE;
}

FileOp_Status
open_temporary_file(FILE** out, char* tmp_filename) {
    *out = fopen(tmp_filename, "wb");
    if(*out == NULL) {
        fprintf(stderr, "endlines : can not create %s\n", tmp_filename);
        return FILEOP_ERROR;
    }
    return CAN_CONTINUE;
}

FileOp_Status
open_input_file_for_dry_run(FILE** in, char* in_filename) {
    *in = fopen(in_filename, "rb");
    if(*in == NULL) {
        fprintf(stderr, "endlines : can not read %s\n", in_filename);
        return FILEOP_ERROR;
    }
    return CAN_CONTINUE;
}

FileOp_Status
move_temp_file_to_destination(char* tmp_filename, char* filename, struct stat *statinfo) {
    int err = remove(filename);
    if(err) {
        fprintf(stderr, "endlines : can not write over %s\n", filename);
        remove(tmp_filename);
        return FILEOP_ERROR;
    }
    err = rename(tmp_filename, filename);
    if(err) {
        fprintf(stderr, "endlines : can not restore %s\n"
                        "  -- Fail safe reaction : aborting.\n"
                        "  -- You will find your data in %s\n"
                        "  -- Please rename it manually to %s\n"
                        "  -- You may report this occurence at :\n"
                        "     https://github.com/mdolidon/endlines/issues\n",
                filename, tmp_filename, filename);
        exit(EXIT_FAILURE);
    }
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
