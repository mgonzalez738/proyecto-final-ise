//=====[#include guards - begin]===============================================

#ifndef _ACTIVITY_LEDS_H_
#define _ACTIVITY_LEDS_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=======================================

//=====[Declaration of public data types]======================================

//=====[Declarations (prototypes) of public functions]=========================

void activityLedsInit();
void blinkUsbLed();
void blinkUartLed();

//=====[#include guards - end]=================================================

#endif // _ACTIVITY_LEDS_H_
