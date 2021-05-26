//=====[Libraries]=============================================================

#include "mbed.h"


#include "configuration.h"
//#include <cstring>

//=====[Declaration of private defines]========================================

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

FlashIAP flash;

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

Configuration configuration;

//=====[Declaration and initialization of private global variables]============

unsigned int flashSectorInitAddress;    // Direccion de inicio del sector de flash
unsigned int flashSectorSize;           // Tamaño del sector

//=====[Declarations (prototypes) of private functions]========================

bool configurationLoad();
unsigned char configurationGenerateCRC(unsigned char* data, int size);

//=====[Implementations of public functions]===================================

void configurationInit() {

    flash.init();           // Inicializa la interfaz a la flash

    // Ubicacion donde se guarda la configuración en flash
    const unsigned int flash_start = flash.get_flash_start();   // Direccion inicio memoria flash
    const unsigned int flash_size = flash.get_flash_size();     // Tamaño de la flash    
    const unsigned int flash_end = flash_start + flash_size;    // Direccion de fin de la flash
    const unsigned int page_size = flash.get_page_size();       // La escritura y lectura debe ser en multiplos de page size (para este dispositivo 1 Byte)
    flashSectorSize = flash.get_sector_size(flash_end - 1);     // El borrado es por sector completo
    flashSectorInitAddress = flash_end - flashSectorSize;       // Los datos se guardan en el ultimo sector de la flash del dispositivo
    
    // Carga la configuracion de la flash
    configurationLoad();

}

//=====[Implementations of private functions]==================================

// Carga la configuracion de la flash
bool configurationLoad() {

    Configuration confLoaded;
    unsigned char crcLoaded;

    // Lee la configuracion desde la flash
    flash.read(confLoaded.bytes, flashSectorInitAddress, sizeof(Configuration));  

    // Calcula el crc de la configuracion leida
    crcLoaded =  configurationGenerateCRC(confLoaded.bytes, sizeof(Configuration)-1);

    // Verifica el crc
    if (crcLoaded == confLoaded.crc) {
        // Copia la configuracion leida a la configuracion global
        memcpy(&configuration, &confLoaded, sizeof(Configuration));
        return true; 
    } else {
        // Carga la configuración por defecto
        configuration.moduleAddress = 0;
        return false;
    }     
}

// Guarda la configuración en la flash
void configurationSave() {

    // Guarda el crc de la configuracion actual
    configuration.crc =  configurationGenerateCRC(configuration.bytes, sizeof(configuration)-1);

    // Borra el sector de flash
    flash.erase(flashSectorInitAddress, flashSectorSize);         

    // Escribe la configuración
    flash.program(configuration.bytes, flashSectorInitAddress, sizeof(configuration));   
}

// Calcula el crc de la configuracion
unsigned char configurationGenerateCRC(unsigned char* data, int size)
{
    unsigned char sum = 0x00;
    int i;

    for (i = 0; i < size; i++)
        sum += data[i];

    return (0xFF - sum);
}
