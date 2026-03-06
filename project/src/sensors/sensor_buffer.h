/**
 * @file sensor_buffer.h
 * @brief Buffer circular para filtrado de lecturas de sensores
 * @details Implementa un buffer circular de 5 muestras para calcular
 *          la mediana y reducir ruido en las lecturas de sensores.
 */

#ifndef SENSOR_BUFFER_H
#define SENSOR_BUFFER_H

#include <Arduino.h>

/**
 * @brief Número de muestras en el buffer (5 lecturas = 5 segundos)
 */
#define BUFFER_SIZE 5

/**
 * @brief Buffer circular para almacenar muestras y calcular mediana
 * @details Almacena las últimas BUFFER_SIZE muestras y calcula la mediana
 *          para filtrar valores atípicos.
 */
struct CircularBuffer {
    float data[BUFFER_SIZE];
    uint8_t index;
    uint8_t count;
    
    /**
     * @brief Inicializa el buffer
     */
    void init();
    
    /**
     * @brief Agrega una nueva muestra al buffer
     * @param value Valor a agregar
     */
    void add(float value);
    
    /**
     * @brief Calcula la mediana de las muestras válidas
     * @return Valor de la mediana
     */
    float getMedian();
};

/**
 * @brief Contenedor de todos los buffers del sistema
 * @note Excluye temperatura RS485 (ya promediada por el sensor)
 */
struct SensorBuffers {
    CircularBuffer tempAmb;
    CircularBuffer humedad;
    CircularBuffer tempAgua;
    CircularBuffer humedadSuelo;
    CircularBuffer ec;
    CircularBuffer ph;
    CircularBuffer nitrogen;
    CircularBuffer phosphorus;
    CircularBuffer potassium;
    
    /**
     * @brief Inicializa todos los buffers
     */
    void initAll();
};

/**
 * @brief Instancia global de buffers de sensores
 */
extern SensorBuffers sensorBuffers;

#endif // SENSOR_BUFFER_H
