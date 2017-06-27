/**
 * @file main.c
 * @brief Main.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include <popt.h>
#include <phidget22.h>

#include "exec.h"


#define OPEN_TIMEOUT_MS (5000)


enum option_kind
{
    OPTION_VERBOSE = 1,
    OPTION_SERIAL_NUMBER = 2,
    OPTION_TAG = 3,
    OPTION_COMMAND = 4
};


static sig_atomic_t global_exit_signal = 0;


static void sig_handler( int sig )
{
    if( sig == SIGINT )
    {
        global_exit_signal = 1;
    }
}


static void error_exit(
        const char * const label,
        const PhidgetReturnCode p_ret)
{
    const char *err;

    if(p_ret != EPHIDGET_OK)
    {
        (void) Phidget_getErrorDescription(p_ret, &err);

        (void) fprintf(
                stderr,
                "%s : %s\n", label, err);
    }
    else if(label != NULL)
    {
        (void) fprintf(
                stderr,
                "%s\n", label);
    }

    exit(EXIT_FAILURE);
}


static void on_tag_handler(
        PhidgetRFIDHandle handle,
        void *ctx,
        const char *tag,
        PhidgetRFID_Protocol protocol)
{
    exec_on_tag(tag, (const on_tag_data_s * const) ctx);
}


int main(int argc, char **argv)
{
    PhidgetReturnCode p_ret;
    poptContext opt_ctx;
    PhidgetRFIDHandle handle;
    on_tag_data_s on_tag_data;

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

    (void) memset(&on_tag_data, 0, sizeof(on_tag_data));

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
        if(opt_ret == OPTION_SERIAL_NUMBER)
        {
            if(on_tag_data.serial_number <= 0)
            {
                (void) fprintf(
                        stderr,
                        "serial number must be greater than zero\n");
                poptPrintUsage(opt_ctx, stderr, 0);
                poptFreeContext(opt_ctx);
                error_exit(NULL, 0);
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
        error_exit(NULL, 0);
    }

    poptFreeContext(opt_ctx);

    printf(
            "serial-number '%ld' - src_tag '%s' - cmd '%s'\n",
            on_tag_data.serial_number,
            (on_tag_data.src_tag == NULL) ? "NA" : on_tag_data.src_tag,
            (on_tag_data.cmd == NULL) ? "NA" : on_tag_data.cmd);

    p_ret = PhidgetRFID_create(&handle);
    if(p_ret != EPHIDGET_OK)
    {
        error_exit("PhidgetRFID_create", p_ret);
    }

    if(on_tag_data.serial_number > 0)
    {
        p_ret = Phidget_setDeviceSerialNumber(
                (PhidgetHandle) handle,
                (int32_t) on_tag_data.serial_number);
        if(p_ret != EPHIDGET_OK)
        {
            (void) PhidgetRFID_delete(&handle);
            error_exit("Phidget_setDeviceSerialNumber", p_ret);
        }
    }

    p_ret = PhidgetRFID_setOnTagHandler(
            handle,
            on_tag_handler,
            (void*) &on_tag_data);
    if(p_ret != EPHIDGET_OK)
    {
        (void) PhidgetRFID_delete(&handle);
        error_exit("PhidgetRFID_setOnTagHandler", p_ret);
    }

    p_ret = Phidget_openWaitForAttachment((PhidgetHandle) handle, OPEN_TIMEOUT_MS);
    if(p_ret != EPHIDGET_OK)
    {
        (void) PhidgetRFID_delete(&handle);
        error_exit("Phidget_openWaitForAttachment", p_ret);
    }

    while(global_exit_signal == 0)
    {
        (void) sleep(1);
    }

    p_ret = Phidget_close((PhidgetHandle) handle);
    if(p_ret != EPHIDGET_OK)
    {
        error_exit("Phidget_close", p_ret);
    }

    p_ret = PhidgetRFID_delete(&handle);
    if(p_ret != EPHIDGET_OK)
    {
        error_exit("PhidgetRFID_delete", p_ret);
    }

    if(on_tag_data.src_tag != NULL)
    {
        free(on_tag_data.src_tag);
    }

    if(on_tag_data.cmd != NULL)
    {
        free(on_tag_data.cmd);
    }

    return EXIT_SUCCESS;
}
