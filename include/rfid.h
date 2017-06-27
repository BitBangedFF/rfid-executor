/**
 * @file rfid.h
 * @brief TODO.
 *
 */


#ifndef RFID_H
#define RFID_H


#include "exec.h"


int rfid_init(
        PhidgetRFIDHandle * const handle,
        const on_tag_data_s * const on_tag_data);

void rfid_fini(
        PhidgetRFIDHandle *  const handle);


#endif  /* RFID_H */
