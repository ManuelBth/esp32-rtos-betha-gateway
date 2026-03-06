/**
 * @file sensor_stats.cpp
 * @brief Implementación de cálculo de estadísticas de sensores
 * @details Calcula medianas de buffers y limpia datos.
 */

#include "sensor_stats.h"
#include "sensor_buffer.h"
#include "../core/config_data.h"
#include "../core/debug.h"

/**
 * @brief Calcula las medianas de todos los buffers y guarda en sistema.sensores
 */
void sensor_stats_calcular_medianas() {
    // DHT22
    if (sensorBuffers.tempAmb.count > 0) {
        sistema.sensores.dht22.temperatura = sensorBuffers.tempAmb.getMedian();
        sistema.sensores.dht22.valido = true;
    }
    
    if (sensorBuffers.humedad.count > 0) {
        sistema.sensores.dht22.humedad = sensorBuffers.humedad.getMedian();
    }
    
    // DS18B20
    if (sensorBuffers.tempAgua.count > 0) {
        sistema.sensores.ds18b20.temperatura = sensorBuffers.tempAgua.getMedian();
        sistema.sensores.ds18b20.valido = true;
    }
    
    // RS485 NPK
    bool rs485DataValid = false;
    
    if (sensorBuffers.ec.count > 0) {
        sistema.sensores.rs485.ec = sensorBuffers.ec.getMedian();
        rs485DataValid = true;
    }
    
    if (sensorBuffers.ph.count > 0) {
        sistema.sensores.rs485.ph = sensorBuffers.ph.getMedian();
        rs485DataValid = true;
    }
    
    if (sensorBuffers.nitrogen.count > 0) {
        sistema.sensores.rs485.nitrogen = sensorBuffers.nitrogen.getMedian();
        rs485DataValid = true;
    }
    
    if (sensorBuffers.phosphorus.count > 0) {
        sistema.sensores.rs485.phosphorus = sensorBuffers.phosphorus.getMedian();
        rs485DataValid = true;
    }
    
    if (sensorBuffers.potassium.count > 0) {
        sistema.sensores.rs485.potassium = sensorBuffers.potassium.getMedian();
        rs485DataValid = true;
    }
    
    if (sensorBuffers.humedadSuelo.count > 0) {
        sistema.sensores.rs485.humedad = sensorBuffers.humedadSuelo.getMedian();
        rs485DataValid = true;
    }
    
    sistema.sensores.rs485.valido = rs485DataValid;
}

/**
 * @brief Limpia todos los buffers y marca sensores como inválidos
 */
void sensor_stats_limpiar_buffers() {
    sensorBuffers.initAll();
    sistema.sensores.dht22.valido = false;
    sistema.sensores.ds18b20.valido = false;
    sistema.sensores.rs485.valido = false;
}
