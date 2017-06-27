/**
 * @file rfid.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <phidget22.h>

#include "rfid.h"
#include "exec.h"


#define OPEN_TIMEOUT_MS (5000)


static int p_err(
        const char * const label,
        const PhidgetReturnCode p_ret)
{
    const char *err = NULL;

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

    return -1;
}


static void on_tag_handler(
        PhidgetRFIDHandle handle,
        void *ctx,
        const char *tag,
        PhidgetRFID_Protocol protocol)
{
    exec_on_tag(tag, (const on_tag_data_s * const) ctx);
}


int rfid_init(
        PhidgetRFIDHandle * const handle,
        const on_tag_data_s * const on_tag_data)
{
    int ret = 0;
    PhidgetReturnCode p_ret;

    p_ret = PhidgetRFID_create(handle);
    if(p_ret != EPHIDGET_OK)
    {
        ret = p_err("PhidgetRFID_create", p_ret);
    }

    if(ret == 0)
    {
        if(on_tag_data->serial_number > 0)
        {
            p_ret = Phidget_setDeviceSerialNumber(
                    (PhidgetHandle) *handle,
                    (int32_t) on_tag_data->serial_number);
            if(p_ret != EPHIDGET_OK)
            {
                (void) PhidgetRFID_delete(handle);
                ret = p_err("Phidget_setDeviceSerialNumber", p_ret);
            }
        }
    }

    if(ret == 0)
    {
        p_ret = PhidgetRFID_setOnTagHandler(
                *handle,
                on_tag_handler,
                (void*) on_tag_data);
        if(p_ret != EPHIDGET_OK)
        {
            (void) PhidgetRFID_delete(handle);
            ret = p_err("PhidgetRFID_setOnTagHandler", p_ret);
        }
    }

    if(ret == 0)
    {
        p_ret = Phidget_openWaitForAttachment(
                (PhidgetHandle) *handle,
                OPEN_TIMEOUT_MS);
        if(p_ret != EPHIDGET_OK)
        {
            (void) PhidgetRFID_delete(handle);
            ret = p_err("Phidget_openWaitForAttachment", p_ret);
        }
    }

    return ret;
}


void rfid_fini(
        PhidgetRFIDHandle *  const handle)
{
    PhidgetReturnCode p_ret;

    if(handle != NULL)
    {
        p_ret = Phidget_close((PhidgetHandle) *handle);
        if(p_ret != EPHIDGET_OK)
        {
            (void) p_err("Phidget_close", p_ret);
        }

        p_ret = PhidgetRFID_delete(handle);
        if(p_ret != EPHIDGET_OK)
        {
            (void) p_err("PhidgetRFID_delete", p_ret);
        }
    }
}
