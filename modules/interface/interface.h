//=====[#include guards - begin]===============================================

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=======================================

#define API_MAX_SIZE 64

//=====[Declaration of public data types]======================================

typedef enum
{ USB = 0,  UART }
InterfacePort;

//=====[Declarations (prototypes) of public functions]=========================

void interfaceInit();
void toggleDisplayView();
void interfaceParseReceivedData(unsigned char* data, int size, InterfacePort port);

//=====[#include guards - end]=================================================

#endif // _INTERFACE_H_
