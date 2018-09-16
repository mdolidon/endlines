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




// SEE command_line_parser.h FOR INTERFACE DOCUMENTATION


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command_line_parser.h"

static char *flag_name_terminators = "=";

Command_Line_Schema*
new_command_line_schema(char *program_name)
{
    Command_Line_Schema* self = malloc(sizeof(Command_Line_Schema));
    if(self == NULL) {
        fprintf(stderr, "%s : failed allocating memory : new_command_line_schema\n", program_name);
        exit(EXIT_FAILURE);
    }

    self->program_name = program_name;
    self->flag_descriptions = NULL;
    self->number_of_flag_descriptions = 0;
    self->process_non_flag_arg = NULL;

    return self;
}


void
destroy_command_line_schema(Command_Line_Schema *self)
{
    free(self);
}


void
set_flag_descriptions(Command_Line_Schema *self,
                      Command_Line_Flag fds[],
                      int number_of_flag_descriptions)
{
    self->number_of_flag_descriptions = number_of_flag_descriptions;
    self->flag_descriptions = fds;
}

void
set_non_flag_arg_callback(
        Command_Line_Schema *self,
        void (*process_non_flag_arg)(char *argument, int argument_sequence_order, void *context))
{
    self->process_non_flag_arg = process_non_flag_arg;
}


// returns NULL is s doesn't start by the prefix, and the first
// character after the prefix otherwise
static char*
strip_prefix_from(char *prefix, char *s)
{
    loop:
    if(*prefix == 0) {
        return s;
    } else if(*prefix != *s) {
        return NULL;
    } else if(*s == 0) {
        return NULL;
    } else {
        prefix ++;
        s ++;
        goto loop;
    }
}


static void
process_long_flag(char *full_flag_arg, Command_Line_Schema *schema, void *context)
{
    size_t flag_name_length = strcspn(full_flag_arg, flag_name_terminators);  // length of anything before "="

    for(int i=0; i<schema->number_of_flag_descriptions; i++) {
        if(schema->flag_descriptions[i].long_flag != NULL) {
            size_t ref_flag_name_length = strlen(schema->flag_descriptions[i].long_flag);
            if(flag_name_length == ref_flag_name_length &&
               !strncmp(full_flag_arg, schema->flag_descriptions[i].long_flag, flag_name_length)) {
                schema->flag_descriptions[i].callback(full_flag_arg, context);
                return;
            }
        }
    }
    fprintf(stderr, "%s : unknown option : --%s\n", schema->program_name, full_flag_arg);
    exit(1);
}

static void
process_short_flag(char flag, char *full_flag_arg, Command_Line_Schema *schema, void *context)
{
    for(int i=0; i<schema->number_of_flag_descriptions; i++) {
        if(schema->flag_descriptions[i].short_flag == flag) {
            schema->flag_descriptions[i].callback(full_flag_arg, context);
            return;
        }
    }
    fprintf(stderr, "%s : unknown option : -%c\n", schema->program_name, flag);
    exit(1);
}

static void
process_short_flags(char *full_bag_of_flags, Command_Line_Schema *schema, void *context)
{
    size_t flag_bag_length = strcspn(full_bag_of_flags, flag_name_terminators);  // length of anything before "="
    for(size_t flag_index=0 ; flag_index<flag_bag_length ; flag_index++) {
        process_short_flag( full_bag_of_flags[flag_index], full_bag_of_flags, schema, context);
    }
}


int
parse_command_line(int argc, char **argv,
                   Command_Line_Schema *schema,
                   void *context)
{
    int non_flag_args_count = 0;
    for(int i=1; i<argc; i++) {
        char *stripped_arg = strip_prefix_from("--", argv[i]);
        if(stripped_arg != NULL) {
            process_long_flag(stripped_arg, schema, context);
            continue;
        }

        stripped_arg = strip_prefix_from("-", argv[i]);
        if(stripped_arg != NULL) {
            process_short_flags(stripped_arg, schema, context);
            continue;
        }

        if(schema->process_non_flag_arg != NULL) {
            schema->process_non_flag_arg(argv[i], non_flag_args_count, context);
            non_flag_args_count ++;
        }
    }
    return 0;
}
