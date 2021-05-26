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

typedef union IMU_CALIBRATION
{
    struct __attribute__((__packed__))
    {
        unsigned char system;
        unsigned char gyroscope;
        unsigned char accelerometer;  
        unsigned char magnetometer;
    };
    unsigned char bytes[4];
} ImuCalibration;

//=====[Declarations (prototypes) of public functions]=========================

void imuInit();

//=====[#include guards - end]=================================================

#endif // _IMU_H_
