//=====[Libraries]=============================================================

#include "mbed.h"

#include "main.h"
#include "configuration.h"
#include "interface.h"
#include "usbPort.h"
#include "uartPort.h"
#include "activityLeds.h"
#include "inclinometer.h"
#include "imu.h"
#include "wind.h"

//=====[Declaration of private defines]=======================================

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

EventQueue mainQueue(128*EVENTS_EVENT_SIZE);

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

unsigned char moduleAddress = 1;

//=====[Declaration and initialization of private global variables]============

//=====[Declarations (prototypes) of private functions]========================

//=====[Main function, the program entry point after power on or reset]========

int main() {

	// Inicializa modulos
    configurationInit();
    interfaceInit();
    usbPortInit();
    uartPortInit();
    activityLedsInit();
    inclinometerInit();
    imuInit();
    windInit();
     
    // Espera y ejecuta eventos
    mainQueue.dispatch_forever();
}

//=====[Implementations of public functions]===================================

//=====[Implementations of private functions]==================================
