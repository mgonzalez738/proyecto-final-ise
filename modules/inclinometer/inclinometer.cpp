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
bool setInclinometerInitial;

//=====[Declaration and initialization of private global variables]============

InclinometerData inclinometerInitial;
unsigned short transmitData[3] = { 0x0C00, 0x0E00, 0x0000 }; // Direcciones
unsigned short receiveData[3];

//=====[Declarations (prototypes) of private functions]========================

void inclinometerAcquire();
void readInclinometerData();

//=====[Implementations of public functions]===================================

void inclinometerInit() {

    // Valores iniciales
    inclinometerInitial.tiltX = 0;
    inclinometerInitial.tiltY = 0;
    inclinometerInitial.tiltZ = 0;

    // Inicializa SPI
    reset = 0;
    cs = 1;
    spi.format(16, 3);
    spi.frequency(500000);
    wait_us(1000);
    reset = 1;
    setInclinometerInitial = false;

    // Asigna la funcion de adquisición periodica
    incTicker.attach(inclinometerAcquire, std::chrono::milliseconds(INC_ACQ_PERIOD_MS));
 
}

//=====[Implementations of private functions]==================================

void inclinometerAcquire() {

    // Lee los datos de inclinacion en el event queue de main
    mainQueue.call(readInclinometerData);

}

void readInclinometerData() {

    int i;
    
    // Lee los datos de inclinacion
    for(i=0; i<3; i++) {
        cs = 0;
        wait_us(1);
        receiveData[i] = spi.write(transmitData[i]);
        wait_us(1);
        cs = 1;
        wait_us(1);
    }
    
    CriticalSectionLock::enable();

    // Calcula y asigna las inclinaciones 
    inclinometer.tiltX = ((short)(receiveData[1]<<2))*0.025/4 - inclinometerInitial.tiltX;
    inclinometer.tiltY = ((short)(receiveData[2]<<2))*0.025/4 - inclinometerInitial.tiltY;
    inclinometer.tiltZ = 0;

    // Toma los valores actuales como iniciales
    if(setInclinometerInitial) {
        inclinometerInitial.tiltX += inclinometer.tiltX;
        inclinometerInitial.tiltY += inclinometer.tiltY;
        setInclinometerInitial = false;
    }

    CriticalSectionLock::disable();
    
}