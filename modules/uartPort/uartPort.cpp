//=====[Libraries]=============================================================

#include "mbed.h"
#include "UnbufferedSerial.h"

#include "main.h"
#include "uartPort.h"
#include "interface.h"
#include "activityLeds.h"

//=====[Declaration of private defines]========================================

//=====[Declaration of private data types]=====================================

typedef enum
{ IDLE = 0,  DEL,  SIZE_MSB,  SIZE_MSB_ESC,  SIZE_LSB,  SIZE_LSB_ESC,  DATA,  DATA_ESC,  CRC_ESC }
UartPortReceiveState;

//=====[Declaration and initialization of public global objects]===============

UnbufferedSerial uartPort(PC_12, PD_2, 115200);

//=====[Declaration of external public global variables]=======================

extern EventQueue mainQueue;

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

UartPortReceiveState uartPortReceiveState;
unsigned char uartPortReceivedApi[API_MAX_SIZE];
int uartPortReceivedApiSize;
int uartPortReceivedApiIndex;

//=====[Declarations (prototypes) of private functions]========================

void uartPortReadByte();
void uartProcessReceivedByte(unsigned char byte);
unsigned char uartGenerateCRC(unsigned char* data, int size);
bool uartCheckEscape(unsigned char data);

//=====[Implementations of public functions]===================================

void uartPortInit() {

    uartPortReceiveState = IDLE;
    uartPort.set_blocking(false);
    uartPort.attach(&uartPortReadByte, UnbufferedSerial::RxIrq);

}

int uartPortSendData(unsigned char* api, int size)
{
    unsigned char transmitFrame[1 + 4 + 2 * API_MAX_SIZE + 2];
    int transmitFrameSize;
    unsigned char lengthMsb, lengthLsb, crc;
    int i, j, ret;

    // Calcula la cantidad de datos y crc

    lengthMsb = (unsigned char)(size / 256);
    lengthLsb = (unsigned char)(size & 0x00FF);

    crc = uartGenerateCRC(api, size);

    // Genera la trama escapando los caracteres especiales
    i = 0;
    // Delimitador
    transmitFrame[i++] = 0x7E;
    // Size
    if(uartCheckEscape(lengthMsb)) {
        transmitFrame[i++] = 0x7D;
        transmitFrame[i++] = lengthMsb ^ 0x20;
    } else {
        transmitFrame[i++] = lengthMsb;
    }
    if (uartCheckEscape(lengthLsb)) {
        transmitFrame[i++] = 0x7D;
        transmitFrame[i++] = lengthLsb ^ 0x20;
    } else {
        transmitFrame[i++] = lengthLsb;
    }
    // Api
    for(j=0; j<size; j++) {
        if (uartCheckEscape(api[j])) {
            transmitFrame[i++] = 0x7D;
            transmitFrame[i++] = api[j] ^ 0x20;
        } else {
            transmitFrame[i++] = api[j];
        }
    }
    // Crc
    if (uartCheckEscape(crc)) {
         transmitFrame[i++] = 0x7D;
         transmitFrame[i++] = crc ^ 0x20;
     } else {
         transmitFrame[i++] = crc;
     }

    // Led
    blinkUartLed();

    // Envia la trama por uart
    ret = uartPort.write(transmitFrame, i);

    return ret;
}

//=====[Implementations of private functions]==================================

void uartPortReadByte()
{
	unsigned char byte;
    // Lee el byte recibido en el contexto de interrupcion
    uartPort.read(&byte, 1);
    // Procesa el byte en el contexto del loop principal
    mainQueue.call(uartProcessReceivedByte, byte);
}

void uartProcessReceivedByte(unsigned char byte)
{
  // Maquina de estados de recepcion de datos

  switch(uartPortReceiveState) {
    
    case IDLE:
      if (byte == 0x7E) {
        uartPortReceiveState = DEL;
      } 
      break;

    case DEL:
      uartPortReceivedApiIndex = 0;
      if((byte == 0x11) || (byte == 0x13)) {
        uartPortReceiveState = IDLE;
      } else if(byte == 0x7E) {
        uartPortReceiveState = DEL;      
      } else if(byte  == 0x7D) {
        uartPortReceiveState = SIZE_MSB_ESC;
      } else {
        uartPortReceivedApiSize = byte * 256;
        uartPortReceiveState = SIZE_MSB;
      } 
      break;

    case SIZE_MSB_ESC:
      if ((byte == 0x7D) || (byte == 0x11) || (byte == 0x13)) {
        uartPortReceiveState = IDLE;
      } else if (byte == 0x7E) {
        uartPortReceiveState = DEL;
      } else {
        uartPortReceivedApiSize = (byte ^ 0x20) * 256;
        uartPortReceiveState = SIZE_MSB;
      }
      break;

    case SIZE_MSB:
      if((byte == 0x11) || (byte == 0x13)) {
        uartPortReceiveState = IDLE;
      } else if(byte == 0x7E) {
        uartPortReceiveState = DEL;    
      } else if (byte == 0x7D) {
        uartPortReceiveState = SIZE_LSB_ESC;
      } else {
        uartPortReceivedApiSize += byte;
        uartPortReceiveState = SIZE_LSB;
      }
      break;

    case SIZE_LSB_ESC:
     if ((byte == 0x7D) || (byte == 0x11) || (byte == 0x13)) {
        uartPortReceiveState = IDLE;
      } else if (byte == 0x7E) {
        uartPortReceiveState = DEL;
      } else {
        uartPortReceivedApiSize += byte ^ 0x20;
        uartPortReceiveState = SIZE_LSB;
      }
      break;

    case SIZE_LSB:
      if((byte == 0x11) || (byte == 0x13)) {
        uartPortReceiveState = IDLE;
      } else if(byte == 0x7E) {
        uartPortReceiveState = DEL; 
      } else if (uartPortReceivedApiSize > API_MAX_SIZE) { // Evita posible overflow
        uartPortReceiveState = IDLE;
      } else if (byte == 0x7D) {
        uartPortReceiveState = DATA_ESC;
      } else {
        uartPortReceivedApi[uartPortReceivedApiIndex++] = byte;
        uartPortReceiveState = DATA;
      }
      break;

    case DATA_ESC:
      if ((byte == 0x7D) || (byte == 0x11) || (byte == 0x13)) {
        uartPortReceiveState = IDLE;
      } else if (byte == 0x7E) {
        uartPortReceiveState = DEL;
      } else {        
        uartPortReceivedApi[uartPortReceivedApiIndex++] = byte ^ 0x20;
        uartPortReceiveState = DATA;
      }
      break;

    case DATA:
      if((byte == 0x11) || (byte == 0x13)) {
        uartPortReceiveState = IDLE;
      } else if(byte == 0x7E) {
        uartPortReceiveState = DEL; 
      } else if (byte == 0x7D) {
        if (uartPortReceivedApiIndex != uartPortReceivedApiSize) {
          uartPortReceiveState = DATA_ESC;
        } else {
          uartPortReceiveState = CRC_ESC;
        }
      } else {
        if (uartPortReceivedApiIndex != uartPortReceivedApiSize) {
          uartPortReceivedApi[uartPortReceivedApiIndex++] = byte;
          uartPortReceiveState = DATA;
        } else {
          uint8_t checkCrc = uartGenerateCRC(uartPortReceivedApi, uartPortReceivedApiIndex);
          if (checkCrc == byte) {
            blinkUartLed();
            interfaceParseReceivedData(uartPortReceivedApi, uartPortReceivedApiIndex, UART);
          }
          uartPortReceiveState = IDLE;
        }
      }
      break;

    case CRC_ESC:
      if ((byte == 0x7D) || (byte == 0x11) || (byte == 0x13)) {
        uartPortReceiveState = IDLE;
      } else if (byte == 0x7E) {
        uartPortReceiveState = DEL;
      } else {
        uint8_t checkCrc = uartGenerateCRC(uartPortReceivedApi, uartPortReceivedApiIndex);
        if (checkCrc == (byte ^ 0x20)) {
          blinkUartLed();
          interfaceParseReceivedData(uartPortReceivedApi, uartPortReceivedApiIndex, UART);
        }
        uartPortReceiveState = IDLE;
      }
      break;
  }
}

// Devuelve el crc de un array de bytes
unsigned char uartGenerateCRC(unsigned char* data, int size)
{
    unsigned char sum = 0x00;
    int i;

    for (i = 0; i < size; i++)
        sum += data[i];

    return (0xFF - sum);
}

// Checkea si un byte es un caracter que requiere ser escapado
bool uartCheckEscape(unsigned char data)
{
    if ((data == 0x7E) || (data == 0x7D) || (data == 0x11) || (data == 0x13))
        return true;
    else
        return false;
}
