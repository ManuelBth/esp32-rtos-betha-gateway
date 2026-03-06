/**
 * @file rtc_handler.cpp
 * @brief Implementación del manejador de RTC
 * @details Gestiona el RTC DS3231 vía I2C, lee fecha/hora y actualiza
 *          la estructura global sistema.tiempo.
 */

#include "rtc_handler.h"
#include "../core/config_data.h"
#include "../core/debug.h"
#include "../core/hardware_config.h"
#include <time.h>
#include <Wire.h>

// ============================================
// INSTANCIAS GLOBALES
// ============================================

/**
 * @brief Instancia del RTC DS3231
 */
RTC_DS3231 rtc;

/**
 * @brief Buffer para formateo de fecha/hora
 */
static char formatBuffer[32];

/**
 * @brief Flag que indica si el RTC está disponible
 */
static bool rtcDisponible = false;

// ============================================
// INICIALIZACIÓN
// ============================================

/**
 * @brief Inicializa el módulo RTC DS3231
 * @details Configura el bus I2C con los pines definidos en hardware_config.h,
 *          detecta el módulo y lee la hora inicial.
 */
void rtc_init() {
    DEBUG_INFO("Inicializando RTC DS3231...");
    
    // Inicializar bus I2C
    Wire.begin(RTC_PIN_SDA, RTC_PIN_SCL);
    
    // Verificar si el RTC está disponible
    if (!rtc.begin()) {
        DEBUG_ERROR("No se encontró el módulo RTC DS3231!");
        rtcDisponible = false;
        return;
    }
    
    rtcDisponible = true;
    DEBUG_INFO("RTC DS3231 detectado correctamente");

    // Leer hora inicial
    rtc_actualizar();
    DEBUG_INFOF("Hora RTC inicial: %s", rtc_obtener_formato().c_str());
}

// ============================================
// LECTURA DE TIEMPO
// ============================================

/**
 * @brief Actualiza la estructura de tiempo del sistema
 * @details Lee el RTC y actualiza todos los campos de sistema.tiempo:
 *          año, mes, día, hora, minuto, segundo, epoch y formato string.
 */
void rtc_actualizar() {
    sistema.ultimaEjecucion.lecturaRTC = millis();
    
    if (!rtcDisponible) {
        DEBUG_ERROR("RTC no disponible. No se puede actualizar la hora.");
        return;
    }
    
    // Obtener fecha y hora del RTC DS3231
    DateTime now = rtc.now();
    
    // Actualizar estructura del sistema
    sistema.tiempo.anho = now.year();
    sistema.tiempo.mes = now.month();
    sistema.tiempo.dia = now.day();
    sistema.tiempo.hora = now.hour();
    sistema.tiempo.minuto = now.minute();
    sistema.tiempo.segundo = now.second();
    
    // Calcular epoch (segundos desde 1970)
    sistema.tiempo.epoch = now.unixtime();
    
    // Formatear fecha y hora
    snprintf(sistema.tiempo.formato, sizeof(sistema.tiempo.formato),
        "%04d-%02d-%02d %02d:%02d:%02d",
        sistema.tiempo.anho, sistema.tiempo.mes, sistema.tiempo.dia,
        sistema.tiempo.hora, sistema.tiempo.minuto, sistema.tiempo.segundo
    );
}

/**
 * @brief Obtiene la hora actual formateada como string
 * @return String con formato "YYYY-MM-DD HH:MM:SS"
 */
String rtc_obtener_formato() {
    snprintf(formatBuffer, sizeof(formatBuffer), 
        "%04d-%02d-%02d %02d:%02d:%02d",
        sistema.tiempo.anho, sistema.tiempo.mes, sistema.tiempo.dia,
        sistema.tiempo.hora, sistema.tiempo.minuto, sistema.tiempo.segundo
    );
    return String(formatBuffer);
}

/**
 * @brief Consulta si el RTC está disponible
 * @return true si el RTC fue detectado correctamente
 */
bool rtc_inicializado() {
    return rtcDisponible;
}
