/**
 * @file main.cpp
 * @brief Punto de entrada del sistema de monitoreo agrícola
 * @details Inicializa todos los módulos: SD, UI, conectividad, sensores,
 *          RTC, telemetría, scheduler y acciones de UI.
 */

#include <Arduino.h>

// Presentation Layer (UI)
#include "presentation/UiHandler.h"
#include "connectivity/connectivity.h"
#include "sensors/sensors.h"
#include "rtc/rtc_handler.h"
#include "telemetry/telemetry.h"
#include "scheduler/scheduler.h"
#include "presentation/ui_actions.h"

// Core
#include "core/config_data.h"
#include "core/debug.h"

// Storage
#include "storage/sd_handler.h"
#include "storage/config_manager.h"
#include "storage/sd_logger.h"

/**
 * @brief Inicialización del sistema
 * @details Secuencia de arranque:
 *          1. Serial
 *          2. SD y configuración
 *          3. Presentation (TFT, LVGL)
 *          4. Conectividad (WiFi, MQTT)
 *          5. Sensores
 *          6. RTC
 *          7. Telemetría
 *          8. Scheduler y tareas
 */
void setup() {
    Serial.begin(115200);
    delay(100);

    // Cargar configuración desde SD
    if (sd_init()) {
        if (config_exists_on_sd()) {
            DEBUG_INFO("MAIN: Cargando configuración desde SD...");
            if (config_load_from_sd()) {
                DEBUG_INFO("MAIN: Configuración cargada desde SD OK");
            } else {
                DEBUG_WARN("MAIN: Fallo al cargar SD, usando defaults");
            }
        } else {
            DEBUG_INFO("MAIN: Primera ejecución, creando config.json...");
            config_save_to_sd();
        }
    } else {
        DEBUG_WARN("MAIN: SD no disponible, usando defaults");
    }

    // Ver intervalos cargados
    DEBUG_INFO("MAIN: Verificando intervalos cargados:");
    DEBUG_INFOF("  telemetria: %lu ms", sistema.intervalos.telemetria);
    DEBUG_INFOF("  heartbeat: %lu ms", sistema.intervalos.heartbeat);
    DEBUG_INFOF("  lecturaSensores: %lu ms", sistema.intervalos.lecturaSensores);
    DEBUG_INFOF("  lecturaRTC: %lu ms", sistema.intervalos.lecturaRTC);
    DEBUG_INFOF("  verificacionConexion: %lu ms", sistema.intervalos.verificacionConexion);

    presentation_init();
    connectivity_init();
    sensors_init();
    rtc_init();
    telemetry_init();
    sd_logger_init();
    scheduler_init();
    ui_actions_init();
    scheduler_start();
}

/**
 * @brief Ciclo principal
 * @details Ejecuta el loop de LVGL para renderizado de UI
 */
void loop() {
   presentation_loop(); 
}
