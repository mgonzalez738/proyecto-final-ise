//=====[#include guards - begin]===============================================

#ifndef _WIND_H_
#define _WIND_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=======================================

//=====[Declaration of public data types]======================================

typedef union WIND_DATA
{
    struct __attribute__((__packed__))
    {
        float speed;
        float direction;
    };
    unsigned char bytes[8];
} WindData;

//=====[Declarations (prototypes) of public functions]=========================

void windInit();

//=====[#include guards - end]=================================================

#endif // _WIND_H_
