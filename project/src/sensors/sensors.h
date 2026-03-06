/**
 * @file sensors.h
 * @brief Gestión de sensores agrícolas
 * @details Proporciona funciones para inicializar y leer todos los sensores:
 *          DHT22 (temp/humedad ambiente), DS18B20 (temp agua/suelo), 
 *          RS485 NPK (nutrientes del suelo).
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// ============================================
// INICIALIZACIÓN
// ============================================

/**
 * @brief Inicializa todos los sensores
 * @details Configura DHT22, DS18B20, RS485 y buffers.
 *          Debe llamarse una vez en setup().
 */
void sensors_init();

// ============================================
// LECTURA DE SENSORES
// ============================================

/**
 * @brief Lee todos los sensores y almacena en buffers
 * @details Lee DHT22, DS18B20 y RS485 NPK. Solo ejecuta si
 *          lecturaActivaUI está habilitada.
 */
void sensors_leer_todos();

/**
 * @brief Fuerza una lectura inmediata de sensores
 * @details Omite el flag lecturaActivaUI para obtener datos instantáneos.
 */
void sensors_forzar_lectura();

#endif
