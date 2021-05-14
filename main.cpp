//=====[Libraries]=============================================================

#include "mbed.h"

#include "main.h"
#include "interface.h"
#include "usbPort.h"
#include "activityLeds.h"

//=====[Declaration of private defines]=======================================

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

EventQueue mainQueue;

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

unsigned char moduleAddress = 0;

//=====[Declaration and initialization of private global variables]============

//=====[Declarations (prototypes) of private functions]========================

//=====[Main function, the program entry point after power on or reset]========

int main() {

	// Inicializa modulos
	usbPortInit();
    interfaceInit();
    activityLedsInit();
     
    // Espera y ejecuta eventos
    mainQueue.dispatch_forever();
}

//=====[Implementations of public functions]===================================

//=====[Implementations of private functions]==================================
