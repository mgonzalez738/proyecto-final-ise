//=====[Libraries]=============================================================

#include "mbed.h"

#include "main.h"
#include "interface.h"
#include "usbPort.h"
#include "display.h"


//=====[Declaration of private defines]========================================

#define DISPLAY_INTRO_TIME      5000ms
#define DISPLAY_REFRESH_PERIOD  1000ms
#define DEBOUNCE_TIME           50ms

//=====[Declaration of private data types]=====================================

typedef enum
{ SHOW_INC = 0,  SHOW_IMU,  SHOW_WIND }
DisplayShow;

//=====[Declaration and initialization of public global objects]===============

Timeout displayIntroTimeout;
Timeout debounceButtonTimeout;
Ticker displayRefreshTicker;

InterruptIn displaySwitchButton(BUTTON1);

//=====[Declaration of external public global variables]=======================

extern unsigned char moduleAddress;
extern EventQueue mainQueue;

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

//=====[Implementations of public functions]===================================

void interfaceInit() {
    
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
        case 2: // Get Data
          
            break;

    }
}

//=====[Implementations of private functions]==================================

void displayIntroEnded() {
    // Cambia la vista del display
    mainQueue.call(displaySwitch,SHOW_INC);
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
    
}

void UpdateImuView() {
    
}

void UpdateWindView() {
    
}