//=====[#include guards - begin]===============================================

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=======================================

#define CONFIGURATION_SIZE 2

//=====[Declaration of public data types]======================================

typedef union CONFIGURATION
{
    struct __attribute__((__packed__))
    {
        unsigned char moduleAddress;        // Direccion del modulo
        unsigned char crc;                  // Crc que se calcula previo a guardar en flash
    };
    unsigned char bytes[CONFIGURATION_SIZE];
} Configuration;

//=====[Declarations (prototypes) of public functions]=========================

void configurationInit();
void configurationSave();

//=====[#include guards - end]=================================================

#endif // _INCLINOMETER_H_
