//=====[Libraries]=============================================================

#include "mbed.h"
#include "UnbufferedSerial.h"

#include "main.h"
#include "usbPort.h"
#include "interface.h"
#include "activityLeds.h"

//=====[Declaration of private defines]========================================

//=====[Declaration of private data types]=====================================

typedef enum
{ IDLE = 0,  DEL,  SIZE_MSB,  SIZE_MSB_ESC,  SIZE_LSB,  SIZE_LSB_ESC,  DATA,  DATA_ESC,  CRC_ESC }
UsbPortReceiveState;

//=====[Declaration and initialization of public global objects]===============

UnbufferedSerial usbPort(USBTX, USBRX, 115200);

//=====[Declaration of external public global variables]=======================

extern EventQueue mainQueue;

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

UsbPortReceiveState usbPortReceiveState;
unsigned char usbPortReceivedApi[API_MAX_SIZE];
int usbPortReceivedApiSize;
int usbPortReceivedApiIndex;

//=====[Declarations (prototypes) of private functions]========================

void usbPortReadByte();
void processReceivedByte(unsigned char byte);
unsigned char generateCRC(unsigned char* data, int size);
bool checkEscape(unsigned char data);

//=====[Implementations of public functions]===================================

void usbPortInit() {

    usbPortReceiveState = IDLE;
    usbPort.set_blocking(false);
    usbPort.attach(&usbPortReadByte, UnbufferedSerial::RxIrq);

}

int usbPortSendData(unsigned char* api, int size)
{
    unsigned char transmitFrame[1 + 4 + 2 * API_MAX_SIZE + 2];
    int transmitFrameSize;
    unsigned char lengthMsb, lengthLsb, crc;
    int i, j, ret;

    // Calcula la cantidad de datos y crc

    lengthMsb = (unsigned char)(size / 256);
    lengthLsb = (unsigned char)(size & 0x00FF);

    crc = generateCRC(api, size);

    // Genera la trama escapando los caracteres especiales
    i = 0;
    // Delimitador
    transmitFrame[i++] = 0x7E;
    // Size
    if(checkEscape(lengthMsb)) {
        transmitFrame[i++] = 0x7D;
        transmitFrame[i++] = lengthMsb ^ 0x20;
    } else {
        transmitFrame[i++] = lengthMsb;
    }
    if (checkEscape(lengthLsb)) {
        transmitFrame[i++] = 0x7D;
        transmitFrame[i++] = lengthLsb ^ 0x20;
    } else {
        transmitFrame[i++] = lengthLsb;
    }
    // Api
    for(j=0; j<size; j++) {
        if (checkEscape(api[j])) {
            transmitFrame[i++] = 0x7D;
            transmitFrame[i++] = api[j] ^ 0x20;
        } else {
            transmitFrame[i++] = api[j];
        }
    }
    // Crc
    if (checkEscape(crc)) {
         transmitFrame[i++] = 0x7D;
         transmitFrame[i++] = crc ^ 0x20;
     } else {
         transmitFrame[i++] = crc;
     }

    // Led
    blinkUsbLed();

    // Envia la trama por usb
    ret = usbPort.write(transmitFrame, i);

    return ret;
}

//=====[Implementations of private functions]==================================

void usbPortReadByte()
{
	unsigned char byte;
    // Lee el byte recibido en el contexto de interrupcion
    usbPort.read(&byte, 1);
    // Procesa el byte en el contexto del loop principal
    mainQueue.call(processReceivedByte, byte);
}

void processReceivedByte(unsigned char byte)
{
  // Maquina de estados de recepcion de datos

  switch(usbPortReceiveState) {
    
    case IDLE:
      if (byte == 0x7E) {
        usbPortReceiveState = DEL;
      } 
      break;

    case DEL:
      usbPortReceivedApiIndex = 0;
      if((byte == 0x11) || (byte == 0x13)) {
        usbPortReceiveState = IDLE;
      } else if(byte == 0x7E) {
        usbPortReceiveState = DEL;      
      } else if(byte  == 0x7D) {
        usbPortReceiveState = SIZE_MSB_ESC;
      } else {
        usbPortReceivedApiSize = byte * 256;
        usbPortReceiveState = SIZE_MSB;
      } 
      break;

    case SIZE_MSB_ESC:
      if ((byte == 0x7D) || (byte == 0x11) || (byte == 0x13)) {
        usbPortReceiveState = IDLE;
      } else if (byte == 0x7E) {
        usbPortReceiveState = DEL;
      } else {
        usbPortReceivedApiSize = (byte ^ 0x20) * 256;
        usbPortReceiveState = SIZE_MSB;
      }
      break;

    case SIZE_MSB:
      if((byte == 0x11) || (byte == 0x13)) {
        usbPortReceiveState = IDLE;
      } else if(byte == 0x7E) {
        usbPortReceiveState = DEL;    
      } else if (byte == 0x7D) {
        usbPortReceiveState = SIZE_LSB_ESC;
      } else {
        usbPortReceivedApiSize += byte;
        usbPortReceiveState = SIZE_LSB;
      }
      break;

    case SIZE_LSB_ESC:
     if ((byte == 0x7D) || (byte == 0x11) || (byte == 0x13)) {
        usbPortReceiveState = IDLE;
      } else if (byte == 0x7E) {
        usbPortReceiveState = DEL;
      } else {
        usbPortReceivedApiSize += byte ^ 0x20;
        usbPortReceiveState = SIZE_LSB;
      }
      break;

    case SIZE_LSB:
      if((byte == 0x11) || (byte == 0x13)) {
        usbPortReceiveState = IDLE;
      } else if(byte == 0x7E) {
        usbPortReceiveState = DEL; 
      } else if (usbPortReceivedApiSize > API_MAX_SIZE) { // Evita posible overflow
        usbPortReceiveState = IDLE;
      } else if (byte == 0x7D) {
        usbPortReceiveState = DATA_ESC;
      } else {
        usbPortReceivedApi[usbPortReceivedApiIndex++] = byte;
        usbPortReceiveState = DATA;
      }
      break;

    case DATA_ESC:
      if ((byte == 0x7D) || (byte == 0x11) || (byte == 0x13)) {
        usbPortReceiveState = IDLE;
      } else if (byte == 0x7E) {
        usbPortReceiveState = DEL;
      } else {        
        usbPortReceivedApi[usbPortReceivedApiIndex++] = byte ^ 0x20;
        usbPortReceiveState = DATA;
      }
      break;

    case DATA:
      if((byte == 0x11) || (byte == 0x13)) {
        usbPortReceiveState = IDLE;
      } else if(byte == 0x7E) {
        usbPortReceiveState = DEL; 
      } else if (byte == 0x7D) {
        if (usbPortReceivedApiIndex != usbPortReceivedApiSize) {
          usbPortReceiveState = DATA_ESC;
        } else {
          usbPortReceiveState = CRC_ESC;
        }
      } else {
        if (usbPortReceivedApiIndex != usbPortReceivedApiSize) {
          usbPortReceivedApi[usbPortReceivedApiIndex++] = byte;
          usbPortReceiveState = DATA;
        } else {
          uint8_t checkCrc = generateCRC(usbPortReceivedApi, usbPortReceivedApiIndex);
          if (checkCrc == byte) {
            blinkUsbLed();
            interfaceParseReceivedData(usbPortReceivedApi, usbPortReceivedApiIndex, USB);
          }
          usbPortReceiveState = IDLE;
        }
      }
      break;

    case CRC_ESC:
      if ((byte == 0x7D) || (byte == 0x11) || (byte == 0x13)) {
        usbPortReceiveState = IDLE;
      } else if (byte == 0x7E) {
        usbPortReceiveState = DEL;
      } else {
        uint8_t checkCrc = generateCRC(usbPortReceivedApi, usbPortReceivedApiIndex);
        if (checkCrc == (byte ^ 0x20)) {
          blinkUsbLed();
          interfaceParseReceivedData(usbPortReceivedApi, usbPortReceivedApiIndex, USB);
        }
        usbPortReceiveState = IDLE;
      }
      break;
  }
}

// Devuelve el crc de un array de bytes
unsigned char generateCRC(unsigned char* data, int size)
{
    unsigned char sum = 0x00;
    int i;

    for (i = 0; i < size; i++)
        sum += data[i];

    return (0xFF - sum);
}

// Checkea si un byte es un caracter que requiere ser escapado
bool checkEscape(unsigned char data)
{
    if ((data == 0x7E) || (data == 0x7D) || (data == 0x11) || (data == 0x13))
        return true;
    else
        return false;
}
