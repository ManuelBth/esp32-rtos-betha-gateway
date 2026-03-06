/**
 * @file sensor_buffer.cpp
 * @brief Implementación del buffer circular
 * @details Implementa buffer circular con cálculo de mediana para
 *          filtrado de ruido en lecturas de sensores.
 */

#include "sensor_buffer.h"
#include <algorithm>

/**
 * @brief Instancia global de buffers de sensores
 */
SensorBuffers sensorBuffers;

/**
 * @brief Inicializa el buffer
 */
void CircularBuffer::init() {
    index = 0;
    count = 0;
    memset(data, 0, sizeof(data));
}

/**
 * @brief Agrega una nueva muestra al buffer
 * @details Sobrescribe muestras antiguas cuando el buffer está lleno
 */
void CircularBuffer::add(float value) {
    data[index] = value;
    index = (index + 1) % BUFFER_SIZE;
    
    if (count < BUFFER_SIZE) {
        count++;
    }
}

/**
 * @brief Calcula la mediana de las muestras
 * @return Valor de la mediana
 */
float CircularBuffer::getMedian() {
    if (count == 0) return 0.0;
    
    float temp[BUFFER_SIZE];
    memcpy(temp, data, count * sizeof(float));
    
    std::sort(temp, temp + count);
    
    if (count % 2 == 0) {
        return (temp[count/2 - 1] + temp[count/2]) / 2.0;
    } else {
        return temp[count/2];
    }
}

/**
 * @brief Inicializa todos los buffers
 */
void SensorBuffers::initAll() {
    tempAmb.init();
    humedad.init();
    tempAgua.init();
    humedadSuelo.init();
    ec.init();
    ph.init();
    nitrogen.init();
    phosphorus.init();
    potassium.init();
}
