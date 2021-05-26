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
ImuCalibration calib;
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
    
    // Inicializa sensor
    sensor.reset();
    //sensor.check();
    sensor.setmode(OPERATION_MODE_NDOF);
    setImuInitial = false;

    // Asigna la funcion de adquisición periodica
    imuTicker.attach(imuAcquire, std::chrono::milliseconds(IMU_ACQ_PERIOD_MS));

}

//=====[Implementations of private functions]==================================

void imuAcquire() {

    // Lee los datos de calibracion y orientacion en el event queue de main
    mainQueue.call(readImuData);

}

void readImuData() {

    // Lee los datos de calibracion
    sensor.get_calib();
    // Lee los datos de orientacion
    sensor.get_angles();

    CriticalSectionLock::enable();

    // Asigna los datos de calibración
    calib.system = (sensor.calib & 0xC0) >> 6;
    calib.gyroscope = (sensor.calib & 0x30) >> 4;
    calib.accelerometer = (sensor.calib & 0x0C) >> 2;
    calib.magnetometer = (sensor.calib & 0x03);

    // Asigna los datos de orientacion
    imu.tiltX = sensor.euler.roll - imuInitial.tiltX;
    imu.tiltY = -sensor.euler.pitch - imuInitial.tiltY;
    imu.tiltZ = sensor.euler.yaw - imuInitial.tiltZ;

    // Toma los valores actuales como iniciales
    if(setImuInitial) {
        imuInitial.tiltX += imu.tiltX;
        imuInitial.tiltY += imu.tiltY;
        imuInitial.tiltZ += imu.tiltZ;
        setImuInitial = false;
    }

    CriticalSectionLock::disable();
    
}