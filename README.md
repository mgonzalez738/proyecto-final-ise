Autor: Martín González - 2021

# Evaluación sensores verticalidad

## Introducción

El objeto de este proyecto es finalizar la asignatura Introducción a los Sistemas Embebidos de la CEIoT.

## Descripción General

En el siguiente link puede verse un video del sistema funcionando: https://drive.google.com/file/d/1c9M5KXBU6D3OMw-yhQpGqFv-cFzJAjnl/view?usp=sharing

Al iniciar el sistema, el microcontrolador adquiere cada un segundo los datos de todos los sensores (inclinómetro, imu y viento). 

Los valores de los sensores pueden ser visualizados en la pantalla del display o en la interfaz de computadora a través del puerto USB, En ambos casos la actualización es cada un segundo.

Es posible calibrar la unidad inercial e inicializar a cero las orientaciones tanto del inclinómetro como de la unidad mediante el pulsador en la protoboard como desde la interfaz de Pc.

La Raspberry consulta los datos del módulo por el puerto Uart cada 10 segundos y transmite los datos al IoT Hub de Azure. Los datos son recibidos por el backend, almacenados en la base de datos Cosmos DB y visualizados en el frontend.

