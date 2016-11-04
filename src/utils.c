
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
#include "known_binary_extensions.h"
#include <string.h>


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


static char*
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

