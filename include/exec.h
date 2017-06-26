/**
 * @file exec.h
 * @brief TODO.
 *
 */


#ifndef EXEC_H
#define EXEC_H


typedef struct
{
    const char * const src_tag;
    const char * const cmd;
    size_t tag_size;
} on_tag_data_s;


void exec_on_tag(
        const char * const tag,
        const on_tag_data_s * const on_tag_data);


#endif  /* EXEC_H */
