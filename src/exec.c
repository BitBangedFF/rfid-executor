/**
 * @file exec.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <phidget22.h>

#include "exec.h"


static void show_tag(
        const char * const tag)
{
    printf("tag '%s'\n", tag);
}


static void do_command(
        const on_tag_data_s * const on_tag_data)
{
    if(on_tag_data->cmd != NULL)
    {
        (void) system(on_tag_data->cmd);
    }
}


void exec_on_tag(
        const char * const tag,
        const on_tag_data_s * const on_tag_data)
{
    if((tag != NULL) && (on_tag_data != NULL))
    {
        if(on_tag_data->src_tag != NULL)
        {
            const int diff = strncmp(
                    tag,
                    on_tag_data->src_tag,
                    on_tag_data->tag_size);
            if(diff == 0)
            {
                do_command(on_tag_data);
            }
        }
        else
        {
            do_command(on_tag_data);
        }
    }
    else
    {
        show_tag(tag);
    }
}
