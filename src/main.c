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

#include <phidget22.h>

#include "exec.h"


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

    (void) Phidget_getErrorDescription(p_ret, &err);

    (void) fprintf(
            stderr,
            "%s : %s\n", label, err);

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
    PhidgetRFIDHandle handle;

    const on_tag_data_s on_tag_data =
    {
        .src_tag = NULL,
        .cmd = "echo 'TEST'",
        .tag_size = 0
    };

    (void) signal(SIGINT, sig_handler);
    (void) siginterrupt(SIGINT, 1);

    p_ret = PhidgetRFID_create(&handle);
    if(p_ret != EPHIDGET_OK)
    {
        error_exit("PhidgetRFID_create", p_ret);
    }

    p_ret = Phidget_setDeviceSerialNumber((PhidgetHandle) handle, 332158);
    if(p_ret != EPHIDGET_OK)
    {
        (void) PhidgetRFID_delete(&handle);
        error_exit("Phidget_setDeviceSerialNumber", p_ret);
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

    p_ret = Phidget_openWaitForAttachment((PhidgetHandle) handle, 5000);
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

    return EXIT_SUCCESS;
}
