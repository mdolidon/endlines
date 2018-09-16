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


#ifndef _COMMAND_LINE_PARSER_H_
#define _COMMAND_LINE_PARSER_H_


// A simple generic command-line parser.
// -------------------------------------


// First create a Command_Line_Schema structure, with new_command_line_schema.
// 
// Then call set_flag_descriptions to describe the command line flags that 
// you want the parser to recognize, and set_non_flag_arg_callback to provide
// a function that the parser will call each time it encounters something else than a flag.
//
// Once this is set up, you can call parse_command_line, and then clean up with destroy_command_line_schema. 



// parse_command_line will analyze the command line arguments. Each time it encounters a known flag, it will
// call a callback function that you've given for that flag. The callback will be passed a persistent
// context object, in which you can do whatever you want to keep track of the options that have been passed.
// If parse_command_line encounters an unknown flag, it will display an error message and call exit().





// Each flag is defined by its short form, long form, and callback function.
// Short flag can be \0 and long flag can be NULL.
// For example :
//    short_flag = 'v'
//    long_flag = "verbose"
//    callback = a function that enables verbosity mode

// The callback function will be provided with :
//   - first arg : a string, the whole argv item that triggered that call ; for example : "--verbose" or "-vlx"
//   - second arg : the user-defined context object that you have passed to parse_command_line in the first place.

typedef struct {
    char short_flag;
    char *long_flag;
    void (*callback)(const char *full_flag_argument, void *context);
} Command_Line_Flag;




// This structure is not really part of the module interface. Use accessor functions.

typedef struct {
    char *program_name;
    int number_of_flag_descriptions;
    Command_Line_Flag *flag_descriptions;
    void (*process_non_flag_arg)(char *argument, int argument_sequence_order, void *context);
} Command_Line_Schema;



// Create a new schema description. This allocates memory.

Command_Line_Schema *new_command_line_schema(char *program_name);




// Once a Command_Line_Schema object has been created, set all its flag
// descriptions at once.

void set_flag_descriptions(Command_Line_Schema *self,
                           Command_Line_Flag flags[],
                           int number_of_flag_descriptions);


// Set up a callback function that will be called for each argv item that is not
// a flag or a group of flags.

void set_non_flag_arg_callback(
        Command_Line_Schema *self,
        void (*process_positional_argument)(char *argument, int arg_index, void *context));



// Run the actual parsing after you've set it all up.

int parse_command_line(int argc, char **argv,
                       Command_Line_Schema* schema,
                       void *context);



// Destroy a schema object after command line parsing.

void destroy_command_line_schema(Command_Line_Schema *self);



#endif
