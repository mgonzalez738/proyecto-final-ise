//=====[Libraries]=============================================================

#include "mbed.h"

#include "imu.h"
#include "BNO055.h"

//=====[Declaration of private defines]========================================

#define IMU_ACQ_PERIOD_MS   1000

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

BNO055 sensor(PB_11, PB_10);
Ticker imuTicker;

//=====[Declaration of external public global variables]=======================

extern EventQueue mainQueue;

//=====[Declaration and initialization of public global variables]=============

ImuData imu;
bool setImuInitial;

//=====[Declaration and initialization of private global variables]============

ImuData imuInitial;

//=====[Declarations (prototypes) of private functions]========================

void imuAcquire();
void readImuData();

//=====[Implementations of public functions]===================================

void imuInit() {

    // Valores iniciales
    imuInitial.tiltX = 0;
    imuInitial.tiltY = 0;
    imuInitial.tiltZ = 0;
    

    // Inicializa sensor y adquisicion
    sensor.reset();
    //sensor.check();
    readImuData();          // Requerido antes de tomar los valores iniciales (primer lectura devuelve 0, revisar)
    setImuInitial = true;
    imuTicker.attach(imuAcquire, std::chrono::milliseconds(IMU_ACQ_PERIOD_MS));

}

//=====[Implementations of private functions]==================================

void imuAcquire() {

    mainQueue.call(readImuData);

}

void readImuData() {

    sensor.setmode(OPERATION_MODE_NDOF);
    sensor.get_calib();
    sensor.get_angles();

    if(setImuInitial) {
        imuInitial.tiltX = sensor.euler.roll;
        imuInitial.tiltY = sensor.euler.pitch;
        imuInitial.tiltZ = sensor.euler.yaw;
        setImuInitial = false;
    }

    CriticalSectionLock::enable();
    imu.tiltX = sensor.euler.roll - imuInitial.tiltX;
    imu.tiltY = sensor.euler.pitch - imuInitial.tiltY;
    imu.tiltZ = sensor.euler.yaw - imuInitial.tiltZ;
    CriticalSectionLock::disable();
    
}