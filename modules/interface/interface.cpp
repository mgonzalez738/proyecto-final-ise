//=====[Libraries]=============================================================

#include "mbed.h"

#include "main.h"
#include "interface.h"
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
Timeout debounceButtonTimeout;
Ticker displayRefreshTicker;

//InterruptIn displaySwitchButton(BUTTON1);
InterruptIn displaySwitchButton(PE_15);

//=====[Declaration of external public global variables]=======================

extern unsigned char moduleAddress;
extern EventQueue mainQueue;
extern WindData wind;
extern InclinometerData inclinometer;
extern ImuData imu;

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

DisplayShow displayShow = SHOW_INC;

//=====[Declarations (prototypes) of private functions]========================

void displayIntroEnded();
void displaySwitchButtonPressed();
void displaySwitchButtonDebounced();
void displaySwitch(DisplayShow ds);
void displayRefresh();
void UpdateIncView();
void UpdateImuView();
void UpdateWindView();

void sendInclinometerData(InterfacePort port);
void sendWindData(InterfacePort port);

//=====[Implementations of public functions]===================================

void interfaceInit() {

    // Inicializa boton
    displaySwitchButton.mode(PullUp);

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
          
            break;
        
        case 3: // Get Wind Data
            sendWindData(port);
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
    // Inicializa el ticker de refresco
    displayRefreshTicker.attach(displayRefresh,DISPLAY_REFRESH_PERIOD);
}

void displaySwitchButtonPressed() {
    debounceButtonTimeout.attach(displaySwitchButtonDebounced, DEBOUNCE_TIME);
}

void displaySwitchButtonDebounced() {
    if(!displaySwitchButton) {
        mainQueue.call(toggleDisplayView);
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
        displayStringWrite( "X[grados] =" );
        displayCharPositionWrite ( 0,2 );
        displayStringWrite( "Y[grados] =" );
        return;
    }
    if(ds == SHOW_IMU) {
        displayClear();
        displayCharPositionWrite ( 0,0 );
        displayStringWrite( "Unidad Inercial" );
        displayCharPositionWrite ( 0,1 );
        displayStringWrite( "X[grados] =" );
        displayCharPositionWrite ( 0,2 );
        displayStringWrite( "Y[grados] =" );
        displayCharPositionWrite ( 0,3 );
        displayStringWrite( "Z[grados] =" );
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
    
    displayCharPositionWrite ( 12,1 );
    displayStringWrite( incXString );
    
    displayCharPositionWrite ( 12,2 );
    displayStringWrite( incYString );
   
}

void UpdateImuView() {
    char imuXString[9];
    char imuYString[9];
    char imuZString[9];

    CriticalSectionLock::enable();
    sprintf(imuXString, "% 7.3f", imu.tiltX);
    sprintf(imuYString, "% 7.3f", imu.tiltY);
    sprintf(imuZString, "% 7.3f", imu.tiltZ);
    CriticalSectionLock::disable();
    
    displayCharPositionWrite ( 12,1 );
    displayStringWrite( imuXString );
    
    displayCharPositionWrite ( 12,2 );
    displayStringWrite( imuYString );

    displayCharPositionWrite ( 12,3 );
    displayStringWrite( imuZString );
}

void UpdateWindView() {

    char speedString[7];
    char directionString[5];

    CriticalSectionLock::enable();
    sprintf(speedString, "% 5.1f", wind.speed);
    sprintf(directionString, "% 3.0f", wind.direction);
    CriticalSectionLock::disable();

    displayCharPositionWrite ( 12,1 );
    displayStringWrite( speedString );

    displayCharPositionWrite ( 14,2 );
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