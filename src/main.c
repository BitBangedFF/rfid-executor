/**
 * @file main.c
 * @brief Main.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <popt.h>

#include "exec.h"
#include "rfid.h"


enum option_kind
{
    OPTION_VERBOSE = 1,
    OPTION_SERIAL_NUMBER = 2,
    OPTION_TAG = 3,
    OPTION_COMMAND = 4
};


static sig_atomic_t global_exit_signal;


static void sig_handler(int sig)
{
    if(sig == SIGINT)
    {
        global_exit_signal = 1;
    }
}


int main(int argc, char **argv)
{
    int ret = 0;
    poptContext opt_ctx;
    on_tag_data_s on_tag_data;
    rfid_s rfid;

    const struct poptOption OPTIONS_TABLE[] =
    {
        {
            "verbose",
            'v',
            POPT_ARG_NONE,
            NULL,
            OPTION_VERBOSE,
            "enable verbose output",
            NULL
        },
        {
            "serial-number",
            's',
            POPT_ARG_LONG,
            &on_tag_data.serial_number,
            OPTION_SERIAL_NUMBER | POPT_ARGFLAG_OPTIONAL,
            "serial number of RFID reader to open",
            "'phidget RFID serial number (0 means any)'"
        },
        {
            "tag",
            't',
            POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
            &on_tag_data.src_tag,
            OPTION_TAG,
            "only allow the specified tag",
            "'source tag string'"
        },
        {
            "command",
            'c',
            POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
            &on_tag_data.cmd,
            OPTION_COMMAND,
            "command to execute when a tag is detected",
            "'command string'"
        },
        POPT_AUTOHELP
        POPT_TABLEEND
    };

    (void) memset(&rfid, 0, sizeof(rfid));
    (void) memset(&on_tag_data, 0, sizeof(on_tag_data));

    global_exit_signal = 0;
    (void) signal(SIGINT, sig_handler);
    (void) siginterrupt(SIGINT, 1);

    opt_ctx = poptGetContext(
            NULL,
            argc,
            (const char**) argv,
            OPTIONS_TABLE,
            0);

    int opt_ret;
    while((opt_ret = poptGetNextOpt(opt_ctx)) >= 0)
    {
        if(opt_ret == OPTION_VERBOSE)
        {
            on_tag_data.verbose = 1;
        }
        else if(opt_ret == OPTION_SERIAL_NUMBER)
        {
            if(on_tag_data.serial_number <= 0)
            {
                (void) fprintf(
                        stderr,
                        "serial number must be greater than zero\n");
                poptPrintUsage(opt_ctx, stderr, 0);
                poptFreeContext(opt_ctx);
                exit(EXIT_FAILURE);
            }
        }
    }

    if(opt_ret < -1)
    {
        (void) fprintf(
                stderr,
                "argument error '%s': %s\n\n",
                poptBadOption(opt_ctx, POPT_BADOPTION_NOALIAS),
                poptStrerror(opt_ret));
        poptPrintUsage(opt_ctx, stderr, 0);
        poptFreeContext(opt_ctx);
        exit(EXIT_FAILURE);
    }

    poptFreeContext(opt_ctx);

    if(on_tag_data.src_tag != NULL)
    {
        on_tag_data.tag_size = strlen(on_tag_data.src_tag);
    }

    if(on_tag_data.verbose != 0)
    {
        (void) fprintf(
                stdout,
                "serial-number '%ld' -- src_tag '%s' -- cmd '%s'\n",
                on_tag_data.serial_number,
                (on_tag_data.src_tag == NULL) ? "NA" : on_tag_data.src_tag,
                (on_tag_data.cmd == NULL) ? "NA" : on_tag_data.cmd);
    }

    ret = rfid_init(&rfid, &on_tag_data);
    if(ret != 0)
    {
        global_exit_signal = 1;
    }

    while(global_exit_signal == 0)
    {
        (void) sleep(1);
    }

    rfid_fini(&rfid);

    if(on_tag_data.src_tag != NULL)
    {
        free(on_tag_data.src_tag);
    }

    if(on_tag_data.cmd != NULL)
    {
        free(on_tag_data.cmd);
    }

    if(ret == 0)
    {
        ret = EXIT_SUCCESS;
    }
    else
    {
        ret = EXIT_FAILURE;
    }

    return ret;
}
