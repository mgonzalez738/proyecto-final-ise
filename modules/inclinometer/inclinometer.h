//=====[#include guards - begin]===============================================

#ifndef _INCLINOMETER_H_
#define _INCLINOMETER_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=======================================

//=====[Declaration of public data types]======================================

typedef union INCLINOMETER_DATA
{
    struct __attribute__((__packed__))
    {
        float tiltX;
        float tiltY;
        float tiltZ;  // Siempre en cero, compatibilidad con IMU para raspberry
    };
    unsigned char bytes[12];
} InclinometerData;

//=====[Declarations (prototypes) of public functions]=========================

void inclinometerInit();

//=====[#include guards - end]=================================================

#endif // _INCLINOMETER_H_
