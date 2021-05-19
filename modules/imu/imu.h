//=====[#include guards - begin]===============================================

#ifndef _IMU_H_
#define _IMU_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=======================================

//=====[Declaration of public data types]======================================

typedef union IMU_DATA
{
    struct __attribute__((__packed__))
    {
        float tiltX;
        float tiltY;
        float tiltZ;  
    };
    unsigned char bytes[12];
} ImuData;

//=====[Declarations (prototypes) of public functions]=========================

void imuInit();

//=====[#include guards - end]=================================================

#endif // _IMU_H_
