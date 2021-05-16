//=====[Libraries]=============================================================

#include "mbed.h"

#include "inclinometer.h"

//=====[Declaration of private defines]========================================

#define INC_ACQ_PERIOD_MS   1000

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

SPI spi(PE_6, PE_5, PE_2); // mosi, miso, sclk
DigitalOut cs(PE_4);
DigitalOut reset(PD_3);

Ticker incTicker;

//=====[Declaration of external public global variables]=======================

extern EventQueue mainQueue;

//=====[Declaration and initialization of public global variables]=============

InclinometerData inclinometer;

//=====[Declaration and initialization of private global variables]============

unsigned short transmitData[4] = { 0x4A00, 0x0C00, 0x0E00, 0x0000 }; // Direcciones
unsigned short receiveData[4];

//=====[Declarations (prototypes) of private functions]========================

void inclinometerAcquire();
void readInclinometerData();

//=====[Implementations of public functions]===================================

void inclinometerInit() {

    reset = 0;
    cs = 1;
    spi.format(16, 3);
    spi.frequency(500000);
    wait_us(1000);
    reset = 1;
    incTicker.attach(inclinometerAcquire, std::chrono::milliseconds(INC_ACQ_PERIOD_MS));

}

//=====[Implementations of private functions]==================================

void inclinometerAcquire() {

    mainQueue.call(readInclinometerData);

}

void readInclinometerData() {

    
    unsigned short prodId;
    int i;

    
    for(i=0; i<5; i++) {
        cs = 0;
        wait_us(1);
        receiveData[i] = spi.write(transmitData[i]);
        wait_us(1);
        cs = 1;
        wait_us(1);
    }
    

    // Calcula inclinaciones

    prodId = receiveData[1];

    inclinometer.tiltX = ((short)(receiveData[2]<<2))*0.025/4;

    inclinometer.tiltY = ((short)(receiveData[3]<<2))*0.025/4;

    inclinometer.tiltZ = 0;
    
}