#ifndef _COMMAND_LINE_PARSER_H_
#define _COMMAND_LINE_PARSER_H_


typedef struct {
    char short_flag;
    char *long_flag;
    void (*callback)(const char *full_flag_argument, void *context);
} Command_Line_Flag;


typedef struct {
    char *program_name;
    int number_of_flag_descriptions;
    Command_Line_Flag *flag_descriptions;
    void (*process_non_flag_arg)(char *argument, int argument_sequence_order, void *context);
} Command_Line_Schema;


Command_Line_Schema *new_command_line_schema();

void destroy_command_line_schema(Command_Line_Schema *self);

void set_flag_descriptions(Command_Line_Schema *self,
                           Command_Line_Flag flags[],
                           int number_of_flag_descriptions);


void set_non_flag_arg_callback(
        Command_Line_Schema *self,
        void (*process_positional_argument)(char *argument, int arg_index, void *context));



int parse_command_line(int argc, char **argv,
                       Command_Line_Schema* schema,
                       void *context);
#endif
