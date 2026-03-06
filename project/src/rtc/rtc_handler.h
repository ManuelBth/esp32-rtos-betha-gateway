/**
 * @file rtc_handler.h
 * @brief Gestión del RTC DS3231 y sincronización de tiempo
 * @details Proporciona funciones para inicializar el RTC, leer la hora actual
 *          y mantener actualizada la estructura de tiempo del sistema.
 */

#ifndef RTC_HANDLER_H
#define RTC_HANDLER_H

#include <Arduino.h>
#include <RTClib.h>
#include "../core/hardware_config.h"

// ============================================
// INICIALIZACIÓN
// ============================================

/**
 * @brief Inicializa el módulo RTC DS3231
 * @details Configura el bus I2C y verifica la presencia del RTC.
 *          Debe llamarse una vez en setup().
 */
void rtc_init();

// ============================================
// LECTURA DE TIEMPO
// ============================================

/**
 * @brief Actualiza la estructura de tiempo del sistema
 * @details Lee el RTC y guarda año, mes, día, hora, minuto, segundo y epoch
 *          en sistema.tiempo. Debe llamarse periódicamente.
 */
void rtc_actualizar();

/**
 * @brief Consulta si el RTC está disponible
 * @return true si el RTC fue detectado correctamente
 */
bool rtc_inicializado();

/**
 * @brief Obtiene la hora actual formateada como string
 * @return String con formato "YYYY-MM-DD HH:MM:SS"
 */
String rtc_obtener_formato();

#endif // RTC_HANDLER_H
