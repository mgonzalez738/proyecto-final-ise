//=====[#include guards - begin]===============================================

#ifndef _USB_PORT_H_
#define _USB_PORT_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=======================================

//=====[Declaration of public data types]======================================

//=====[Declarations (prototypes) of public functions]=========================

void usbPortInit();
int usbPortSendData(unsigned char* api, int size);

//=====[#include guards - end]=================================================

#endif // _USB_PORT_H_
