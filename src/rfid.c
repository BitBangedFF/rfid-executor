/**
 * @file rfid.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <phidget22.h>

#include "rfid.h"
#include "exec.h"


#define OPEN_TIMEOUT_MS (5000)
#define UPDATE_WAIT_SEC (1)
#define UPDATE_ACCESS_ALLOWED_WAIT_SEC (3)
#define LED_CHANNEL (1)


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


int rfid_init(
        rfid_s * const rfid,
        const on_tag_data_s * const on_tag_data)
{
    int ret = 0;
    PhidgetReturnCode p_ret;

    if((rfid == NULL) || (on_tag_data == NULL))
    {
        ret = -1;
    }

    if(ret == 0)
    {
        rfid->on_tag_data = on_tag_data;
    }

    if(ret == 0)
    {
        p_ret = PhidgetRFID_create(&rfid->rfid_ch);
        if(p_ret != EPHIDGET_OK)
        {
            ret = p_err("PhidgetRFID_create", p_ret);
        }
    }

    if(ret == 0)
    {   
        p_ret = PhidgetDigitalOutput_create(&rfid->led_ch);
        if(p_ret != EPHIDGET_OK)
        {
            ret = p_err("PhidgetDigitalOutput_create", p_ret);
        }
    }

    if(ret == 0)
    {
        p_ret = Phidget_setChannel(
                (PhidgetHandle) rfid->led_ch,
                LED_CHANNEL);
        if(p_ret != EPHIDGET_OK)
        {
            ret = p_err("Phidget_setChannel", p_ret);
        }
    }

    if(ret == 0)
    {
        if(on_tag_data->serial_number > 0)
        {
            p_ret = Phidget_setDeviceSerialNumber(
                    (PhidgetHandle) rfid->rfid_ch,
                    (int32_t) on_tag_data->serial_number);
            if(p_ret != EPHIDGET_OK)
            {
                ret = p_err("Phidget_setDeviceSerialNumber", p_ret);
            }
        }
    }

    if(ret == 0)
    {
        p_ret = Phidget_openWaitForAttachment(
                (PhidgetHandle) rfid->rfid_ch,
                OPEN_TIMEOUT_MS);
        if(p_ret != EPHIDGET_OK)
        {
            ret = p_err("Phidget_openWaitForAttachment (RFID)", p_ret);
        }
    }

    if(ret == 0)
    {
        p_ret = Phidget_openWaitForAttachment(
                (PhidgetHandle) rfid->led_ch,
                OPEN_TIMEOUT_MS);
        if(p_ret != EPHIDGET_OK)
        {
            ret = p_err("Phidget_openWaitForAttachment (LED)", p_ret);
        }
    }

    if(ret == 0)
    {
        if(on_tag_data->serial_number <= 0)
        {
            int32_t sn = 0;

            p_ret = Phidget_getDeviceSerialNumber(
                    (PhidgetHandle) rfid->rfid_ch,
                    &sn);
            if(p_ret != EPHIDGET_OK)
            {
                ret = p_err("Phidget_getDeviceSerialNumber", p_ret);
            }
            else
            {
                if(on_tag_data->verbose != 0)
                {
                    (void) fprintf(
                            stdout,
                            "found device with serial number '%ld'\n",
                            (long) sn);
                }
            }
        }
    }

    if(ret == 0)
    {
        p_ret = PhidgetRFID_setAntennaEnabled(
                rfid->rfid_ch,
                1);
        if(p_ret != EPHIDGET_OK)
        {
            ret = p_err("PhidgetRFID_setAntennaEnabled", p_ret);
        }
    }

    if(ret == 0)
    {
        p_ret = PhidgetDigitalOutput_setState(
                rfid->led_ch,
                0);
        if(p_ret != EPHIDGET_OK)
        {
            ret = p_err("PhidgetDigitalOutput_setState", p_ret);
        }
    }

    return ret;
}


void rfid_fini(
        rfid_s * const rfid)
{
    PhidgetReturnCode p_ret;

    if(rfid != NULL)
    {
        (void) PhidgetRFID_setAntennaEnabled(
                rfid->rfid_ch,
                0);

        (void) PhidgetDigitalOutput_setState(
                rfid->led_ch,
                0);

        p_ret = Phidget_close((PhidgetHandle) rfid->led_ch);
        if(p_ret != EPHIDGET_OK)
        {
            (void) p_err("Phidget_close (LED)", p_ret);
        }

        p_ret = Phidget_close((PhidgetHandle) rfid->rfid_ch);
        if(p_ret != EPHIDGET_OK)
        {
            (void) p_err("Phidget_close (RFID)", p_ret);
        }

        p_ret = PhidgetDigitalOutput_delete(&rfid->led_ch);
        if(p_ret != EPHIDGET_OK)
        {
            (void) p_err("PhidgetDigitalOutput_delete", p_ret);
        }

        p_ret = PhidgetRFID_delete(&rfid->rfid_ch);
        if(p_ret != EPHIDGET_OK)
        {
            (void) p_err("PhidgetRFID_delete", p_ret);
        }
    }
}


void rfid_update(
        rfid_s * const rfid)
{
    if(rfid != NULL)
    {
        PhidgetReturnCode p_ret;
        int tag_present = 0;

        p_ret = PhidgetRFID_getTagPresent(
                rfid->rfid_ch,
                &tag_present);

        if((tag_present != 0) && (p_ret == EPHIDGET_OK))
        {
            PhidgetRFID_Protocol proto;

            p_ret = PhidgetRFID_getLastTag(
                    rfid->rfid_ch,
                    rfid->last_tag,
                    sizeof(rfid->last_tag),
                    &proto);
        }

        if((tag_present != 0) && (p_ret == EPHIDGET_OK))
        {
            const int status = exec_is_allowed(
                    rfid->last_tag,
                    rfid->on_tag_data);

            if(status == EXEC_STATUS_ALLOWED)
            {
                (void) PhidgetDigitalOutput_setState(
                        rfid->led_ch,
                        1);
            }

            exec_on_tag(rfid->last_tag, rfid->on_tag_data);

            (void) sleep(UPDATE_ACCESS_ALLOWED_WAIT_SEC);
        }

        (void) PhidgetDigitalOutput_setState(
                rfid->led_ch,
                0);
    }

    (void) sleep(UPDATE_WAIT_SEC);
}
