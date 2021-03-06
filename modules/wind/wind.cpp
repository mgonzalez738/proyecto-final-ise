//=====[Libraries]=============================================================

#include "mbed.h"

#include "wind.h"

//=====[Declaration of private defines]========================================

#define WIND_ACQ_PERIOD_MS  1000
#define VANE_GAIN           424.2103
#define VANE_OFFSET         -28.7651
#define SPEED_GAIN          0.5

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

AnalogIn vane(A0);
InterruptIn speed(PE_12);

Ticker windTicker;

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

WindData wind;

//=====[Declaration and initialization of private global variables]============

int pulseCounter;

//=====[Declarations (prototypes) of private functions]========================

void pulseIncrement();
void windAcquire();

//=====[Implementations of public functions]===================================

void windInit() {

    // Inicializa el contador de pulsos
    pulseCounter = 0;

    // Asigna la adquisicion analogica y calculo de frecuencia periodicos
    windTicker.attach(windAcquire, std::chrono::milliseconds(WIND_ACQ_PERIOD_MS));

    // Asigna la interrupción de flaco ascendente para los pulsos del anemometro
    speed.rise(pulseIncrement);

}

//=====[Implementations of private functions]==================================

void pulseIncrement() {

    // Incrementa e contador de pulsos
    pulseCounter++;

}

void windAcquire() {

    // Adquiere la veleta
    float vaneReading = vane.read();
    
    CriticalSectionLock::enable();

    // Calcula el angulo de la veleta
    wind.direction = VANE_GAIN * vaneReading + VANE_OFFSET;
    if(wind.direction  >= 360.0) {
        wind.direction  = 0;
    }
    if(wind.direction  < 0.0) {
        wind.direction  = 0;
    }

    // Calcula velocidad
    wind.speed = (float)pulseCounter / WIND_ACQ_PERIOD_MS * 1000 * SPEED_GAIN;
    pulseCounter = 0;
    
    CriticalSectionLock::disable();
}