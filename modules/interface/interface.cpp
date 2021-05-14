//=====[Libraries]=============================================================

#include "mbed.h"

#include "main.h"
#include "interface.h"
#include "usbPort.h"


//=====[Declaration of private defines]========================================

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

//=====[Declaration of external public global variables]=======================

extern unsigned char moduleAddress;
extern EventQueue mainQueue;

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

//=====[Declarations (prototypes) of private functions]========================

//=====[Implementations of public functions]===================================

void interfaceInit() {

}

void interfaceParseReceivedData(unsigned char* data, int size, InterfacePort port) {

    // Verifica la cantidad de datos
    if(size < 2)
        return;
    
    // Verifica la direccion
    if((data[0] != 0x00) && (data[0] != moduleAddress))
        return;

    // Verifica la funcion
    switch(data[1])
    {
        case 2: // Get Data
          
            break;

    }
}

//=====[Implementations of private functions]==================================

