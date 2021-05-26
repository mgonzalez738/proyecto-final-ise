//=====[Libraries]=============================================================

#include "mbed.h"

#include "interface.h"
#include "configuration.h"
#include "usbPort.h"
#include "uartPort.h"
#include "display.h"
#include "wind.h"
#include "inclinometer.h"
#include "imu.h"


//=====[Declaration of private defines]========================================

#define DISPLAY_INTRO_TIME      3000ms
#define DISPLAY_REFRESH_PERIOD  500ms
#define DEBOUNCE_TIME           50ms

//=====[Declaration of private data types]=====================================

typedef enum
{ SHOW_INC = 0,  SHOW_IMU,  SHOW_WIND }
DisplayShow;

//=====[Declaration and initialization of public global objects]===============

Timeout displayIntroTimeout;
Timeout debounceDisplaySwitchButtonTimeout;
Timeout debounceSetInitButtonTimeout;
Ticker displayRefreshTicker;

InterruptIn displaySwitchButton(BUTTON1);
InterruptIn setInitButton(PE_15);

//=====[Declaration of external public global variables]=======================

extern unsigned char moduleAddress;
extern EventQueue mainQueue;
extern WindData wind;
extern InclinometerData inclinometer;
extern ImuData imu;
extern ImuCalibration calib;
extern bool setImuInitial;
extern bool setInclinometerInitial;
extern Configuration configuration;

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

DisplayShow displayShow = SHOW_INC;

//=====[Declarations (prototypes) of private functions]========================

void displayIntroEnded();
void displaySwitchButtonPressed();
void displaySwitchButtonDebounced();
void setInitButtonPressed();
void setInitButtonDebounced();
void displaySwitch(DisplayShow ds);
void displayRefresh();
void UpdateIncView();
void UpdateImuView();
void UpdateWindView();

// Funciones interfaz
void sendInclinometerData(InterfacePort port);
void sendImuData(InterfacePort port);
void sendWindData(InterfacePort port);
void sendImuCalibrationStatus(InterfacePort port);
void sendConfiguration(InterfacePort port);
void receiveConfiguration(unsigned char* data);
void setInitialSensorValues();

//=====[Implementations of public functions]===================================

void interfaceInit() {

    // Inicializa boton setInit
    setInitButton.mode(PullUp);

    // Inicializa Display
    displayInit( DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER );  
    displayCharPositionWrite ( 0,0 );
    displayStringWrite( "CEIoT - ISE" );
    displayCharPositionWrite ( 0,1 );
    displayStringWrite( "Proyecto Final" );
    displayCharPositionWrite ( 0,2 );
    displayStringWrite( "Prueba de Sensores" );
    displayCharPositionWrite ( 0,3 );
    displayStringWrite( "Mon. Verticalidad" );
    displayIntroTimeout.attach(displayIntroEnded, DISPLAY_INTRO_TIME);

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
        case 1: // Get Inclinometer Data
            sendInclinometerData(port);
            break;

        case 2: // Get Data
            sendImuData(port);
            break;
        
        case 3: // Get Wind Data
            sendWindData(port);
            break;

        case 4: // Get Imu Calibration Status
            sendImuCalibrationStatus(port);
            break;

        case 5: // 
            setInitialSensorValues();
            break;

        case 6: // Configuration
            if(size > 2) {
                receiveConfiguration(data);
            }
            sendConfiguration(port);
            break;

    }
}

//=====[Implementations of private functions]==================================

void displayIntroEnded() {
    // Cambia la vista del display
    displayShow = SHOW_INC;
    mainQueue.call(displaySwitch, displayShow);
    // Activa el boton de cambio de vista
    displaySwitchButton.fall(displaySwitchButtonPressed);
    // Activa el boton de inicializacion de sensores
    setInitButton.fall(setInitButtonPressed);
    // Inicializa el ticker de refresco
    displayRefreshTicker.attach(displayRefresh,DISPLAY_REFRESH_PERIOD);
}

void displaySwitchButtonPressed() {
    debounceDisplaySwitchButtonTimeout.attach(displaySwitchButtonDebounced, DEBOUNCE_TIME);
}

void displaySwitchButtonDebounced() {
    if(!displaySwitchButton) {
        mainQueue.call(toggleDisplayView);
    }
}

void setInitButtonPressed() {
    debounceSetInitButtonTimeout.attach(setInitButtonDebounced, DEBOUNCE_TIME);
}

void setInitButtonDebounced() {
    if(!setInitButton) {
        setImuInitial = true;
        setInclinometerInitial = true;
    }
}

void toggleDisplayView() {
    switch(displayShow) {
        case SHOW_INC:
            displayShow = SHOW_IMU;
            break;
        case SHOW_IMU:
            displayShow = SHOW_WIND;
            break;
        case SHOW_WIND:
            displayShow = SHOW_INC;
            break;
    }
    displaySwitch(displayShow);
}

void displaySwitch(DisplayShow ds) {
    if(ds == SHOW_INC) {
        displayClear();
        displayCharPositionWrite ( 0,0 );
        displayStringWrite( "Inclinometro" );
        displayCharPositionWrite ( 0,1 );
        displayStringWrite( "X[gr] =" );
        displayCharPositionWrite ( 0,2 );
        displayStringWrite( "Y[gr] =" );
        return;
    }
    if(ds == SHOW_IMU) {
        displayClear();
        displayCharPositionWrite ( 0,0 );
        displayStringWrite( "Inercial" );
        displayCharPositionWrite ( 0,1 );
        displayStringWrite( "X[gr] =" );
        displayCharPositionWrite ( 0,2 );
        displayStringWrite( "Y[gr] =" );
        displayCharPositionWrite ( 0,3 );
        displayStringWrite( "Z[gr] =" );
        return;
    }
    if(ds == SHOW_WIND) {
        displayClear();
        displayCharPositionWrite ( 0,0 );
        displayStringWrite( "Viento" );
        displayCharPositionWrite ( 0,1 );
        displayStringWrite( "Vel[Km/h] =" );
        displayCharPositionWrite ( 0,2 );
        displayStringWrite( "Dir[grados] =" );
        return;
    }
}

void displayRefresh() {
    switch(displayShow) {
        case SHOW_INC:
            mainQueue.call(UpdateIncView);
            break;
        case SHOW_IMU:
            mainQueue.call(UpdateImuView);
            break;
        case SHOW_WIND:
            mainQueue.call(UpdateWindView);
            break;
    }
}

void UpdateIncView() {
    char incXString[9];
    char incYString[9];

    CriticalSectionLock::enable();
    sprintf(incXString, "% 7.3f", inclinometer.tiltX);
    sprintf(incYString, "% 7.3f", inclinometer.tiltY);
    CriticalSectionLock::disable();
    
    displayCharPositionWrite ( 13,1 );
    displayStringWrite( incXString );
    
    displayCharPositionWrite ( 13,2 );
    displayStringWrite( incYString );
   
}

void UpdateImuView() {
    char imuXString[9];
    char imuYString[9];
    char imuZString[9];
    char calibString[10];

    CriticalSectionLock::enable();
    sprintf(imuXString, "% 7.3f", imu.tiltX);
    sprintf(imuYString, "% 7.3f", imu.tiltY);
    sprintf(imuZString, "% 7.3f", imu.tiltZ);
    sprintf(calibString, "[%d %d %d %d]", calib.system, calib.gyroscope, calib.accelerometer, calib.magnetometer);
    CriticalSectionLock::disable();
    
    displayCharPositionWrite ( 11,0 );
    displayStringWrite( calibString );

    displayCharPositionWrite ( 13,1 );
    displayStringWrite( imuXString );
    
    displayCharPositionWrite ( 13,2 );
    displayStringWrite( imuYString );

    displayCharPositionWrite ( 13,3 );
    displayStringWrite( imuZString );
    
}

void UpdateWindView() {

    char speedString[7];
    char directionString[5];

    CriticalSectionLock::enable();
    sprintf(speedString, "% 5.1f", wind.speed);
    sprintf(directionString, "% 3.0f", wind.direction);
    CriticalSectionLock::disable();

    displayCharPositionWrite ( 15,1 );
    displayStringWrite( speedString );

    displayCharPositionWrite ( 17,2 );
    displayStringWrite( directionString );
    
}

void sendInclinometerData(InterfacePort port) {

    unsigned char api[API_MAX_SIZE];
    int idx = 0;
    int i;

    api[idx++] = moduleAddress;             // Direccion
    api[idx++] = 0x01;                      // Funcion
    CriticalSectionLock::enable();
    for(i=0; i<sizeof(inclinometer.bytes); i++) {   // Datos
        api[idx++] = inclinometer.bytes[i];
    }
    CriticalSectionLock::disable();

    if(port == USB) {
        usbPortSendData(api, 2 + sizeof(inclinometer.bytes));
    }
    if(port == UART) {
        uartPortSendData(api, 2 + sizeof(inclinometer.bytes));
    }

}

void sendImuData(InterfacePort port) {

    unsigned char api[API_MAX_SIZE];
    int idx = 0;
    int i;

    api[idx++] = moduleAddress;             // Direccion
    api[idx++] = 0x02;                      // Funcion
    CriticalSectionLock::enable();
    for(i=0; i<sizeof(imu.bytes); i++) {   // Datos
        api[idx++] = imu.bytes[i];
    }
    CriticalSectionLock::disable();

    if(port == USB) {
        usbPortSendData(api, 2 + sizeof(imu.bytes));
    }
    if(port == UART) {
        uartPortSendData(api, 2 + sizeof(imu.bytes));
    }

}

void sendWindData(InterfacePort port) {

    unsigned char api[API_MAX_SIZE];
    int idx = 0;
    int i;

    api[idx++] = moduleAddress;             // Direccion
    api[idx++] = 0x03;                      // Funcion
    CriticalSectionLock::enable();
    for(i=0; i<sizeof(wind.bytes); i++) {   // Datos
        api[idx++] = wind.bytes[i];
    }
    CriticalSectionLock::disable();
    
    if(port == USB) {
        usbPortSendData(api, 2 + sizeof(wind.bytes));
    }
    if(port == UART) {
        uartPortSendData(api, 2 + sizeof(wind.bytes));
    }

}

void sendImuCalibrationStatus(InterfacePort port) {

    unsigned char api[API_MAX_SIZE];
    int idx = 0;
    int i;

    api[idx++] = moduleAddress;             // Direccion
    api[idx++] = 0x04;                      // Funcion
    CriticalSectionLock::enable();
    for(i=0; i<sizeof(calib.bytes); i++) {   // Datos
        api[idx++] = calib.bytes[i];
    }
    CriticalSectionLock::disable();
    
    if(port == USB) {
        usbPortSendData(api, 2 + sizeof(calib.bytes));
    }
    if(port == UART) {
        uartPortSendData(api, 2 + sizeof(calib.bytes));
    }

}

void setInitialSensorValues() {

    setImuInitial = true;
    setInclinometerInitial = true;
    
}

void receiveConfiguration(unsigned char* data) {

    CriticalSectionLock::enable();
    memcpy(&configuration.bytes, &data[2], sizeof(Configuration));
    CriticalSectionLock::disable();
    configurationSave();
    
}

void sendConfiguration(InterfacePort port) {

    unsigned char api[API_MAX_SIZE];
    int idx = 0;
    int i;

    api[idx++] = moduleAddress;             // Direccion
    api[idx++] = 0x06;                      // Funcion
    CriticalSectionLock::enable();
    for(i=0; i<sizeof(configuration.bytes); i++) {   // Datos
        api[idx++] = configuration.bytes[i];
    }
    CriticalSectionLock::disable();
    
    if(port == USB) {
        usbPortSendData(api, 2 + sizeof(calib.bytes));
    }
    if(port == UART) {
        uartPortSendData(api, 2 + sizeof(calib.bytes));
    }
}