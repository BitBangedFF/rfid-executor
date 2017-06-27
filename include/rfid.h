/**
 * @file rfid.h
 * @brief TODO.
 *
 */


#ifndef RFID_H
#define RFID_H


#include <phidget22.h>

#include "exec.h"


typedef struct
{
    PhidgetRFIDHandle rfid_ch;
    PhidgetDigitalOutputHandle led_ch;
    const on_tag_data_s *on_tag_data;
} rfid_s;


int rfid_init(
        rfid_s * const rfid,
        const on_tag_data_s * const on_tag_data);


void rfid_fini(
        rfid_s * const rfid);


#endif  /* RFID_H */
