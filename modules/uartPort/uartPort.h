//=====[#include guards - begin]===============================================

#ifndef _UART_PORT_H_
#define _UART_PORT_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=======================================

//=====[Declaration of public data types]======================================

//=====[Declarations (prototypes) of public functions]=========================

void uartPortInit();
int uartPortSendData(unsigned char* api, int size);

//=====[#include guards - end]=================================================

#endif // _UART_PORT_H_
