//=====[Libraries]=============================================================

#include "mbed.h"

#include "main.h"

//=====[Declaration of private defines]========================================

#define STATUS_LED_PERIOD   250ms
#define STATUS_LED_ON        20ms
#define USB_LED_ON           20ms
#define UART_LED_ON          20ms

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

DigitalOut ledStatus(LED1);
DigitalOut ledUsb(LED2);
DigitalOut ledUart(LED3);

Ticker ledStatusTicker;
Timeout ledStatusTimeout;
Timeout ledUsbTimeout;
Timeout ledUartTimeout;

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

//=====[Declarations (prototypes) of private functions]========================

void ledStatusOn();
void ledStatusOff(); 
void ledUsbOn();
void ledUsbOff(); 
void ledUartOn();
void ledUartOff(); 

//=====[Implementations of public functions]===================================

void activityLedsInit() {

    ledStatusTicker.attach(ledStatusOn, STATUS_LED_PERIOD);

}

void blinkUsbLed() {
    ledUsbOn();
    ledUsbTimeout.attach(ledUsbOff, USB_LED_ON);
}

void blinkUartLed() {
    ledUartOn();
    ledUartTimeout.attach(ledUartOff, UART_LED_ON);
}

//=====[Implementations of private functions]==================================

void ledStatusOn() {
    ledStatus = true;
    ledStatusTimeout.attach(ledStatusOff, STATUS_LED_ON);
}

void ledStatusOff() {
    ledStatus = false;
}

void ledUsbOn() {
    ledUsb = true;
}

void ledUsbOff() {
    ledUsb = false;
}

void ledUartOn() {
    ledUart = true;
}

void ledUartOff() {
    ledUart = false;
}