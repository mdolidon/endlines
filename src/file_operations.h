#ifndef _FILE_OPERATIONS_H_
#define _FILE_OPERATIONS_H_

#include <sys/stat.h>
#include <utime.h>
#include <unistd.h>



// Possible outcomes for each file processed

#define FILEOP_STATUSES_COUNT 4
typedef enum {
    CAN_CONTINUE,  // intermediate state : no error yet, but processing not finished
    DONE,
    SKIPPED_BINARY,
    FILEOP_ERROR,
} FileOp_Status;



struct utimbuf get_file_times(struct stat* statinfo);

FileOp_Status open_input_file_for_conversion(FILE** in, char* in_filename);
FileOp_Status open_temporary_file(FILE** out, char* tmp_filename);
FileOp_Status open_input_file_for_dry_run(FILE** in, char* in_filename);
FileOp_Status move_temp_file_to_destination(char* tmp_filename, char* filename, struct stat *statinfo);

#endif
