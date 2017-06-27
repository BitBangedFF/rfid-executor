/**
 * @file exec.h
 * @brief TODO.
 *
 */


#ifndef EXEC_H
#define EXEC_H


#define EXEC_STATUS_ALLOWED (0)
#define EXEC_STATUS_DENIED (1)


typedef struct
{
    unsigned int verbose;
    long serial_number;
    char *src_tag;
    char *cmd;
    size_t tag_size;
} on_tag_data_s;


int exec_is_allowed(
        const char * const tag,
        const on_tag_data_s * const on_tag_data);


void exec_on_tag(
        const char * const tag,
        const on_tag_data_s * const on_tag_data);


#endif  /* EXEC_H */
