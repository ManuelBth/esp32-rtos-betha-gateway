/**
 * @file scheduler.cpp
 * @brief Implementación del scheduler de tareas FreeRTOS
 * @details Gestiona timers periódicos y tareas dedicadas para telemetría,
 *          lectura de sensores, verificación de conexión y actualización del RTC.
 */

#include "scheduler.h"
#include "../core/config_data.h"
#include "../core/debug.h"
#include "../core/hardware_config.h"
#include "../connectivity/connectivity.h"
#include "../telemetry/telemetry.h"
#include "../sensors/sensors.h"
#include "../rtc/rtc_handler.h"

// ============================================
// HANDLES DE TIMERS
// ============================================

/**
 * @brief Timer de telemetría (envío de datos al broker MQTT)
 */
static TimerHandle_t timerTelemetria = NULL;

/**
 * @brief Timer de heartbeat (señal de vida al servidor)
 */
static TimerHandle_t timerHeartbeat = NULL;

/**
 * @brief Timer de lectura de sensores
 */
static TimerHandle_t timerLecturaSensores = NULL;

/**
 * @brief Timer de lectura del RTC
 */
static TimerHandle_t timerLecturaRTC = NULL;

/**
 * @brief Timer de verificación de conexión WiFi/MQTT
 */
static TimerHandle_t timerVerificacionConexion = NULL;

// ============================================
// HANDLES DE TAREAS
// ============================================

/**
 * @brief Handle de tarea de telemetría
 */
static TaskHandle_t taskTelemetriaHandle = NULL;

/**
 * @brief Handle de tarea de sensores
 */
static TaskHandle_t taskSensoresHandle = NULL;

/**
 * @brief Handle de tarea de conexión
 */
static TaskHandle_t taskConexionHandle = NULL;

// ============================================
// FLAGS DE EJECUCIÓN
// ============================================

/**
 * @brief Flag para ejecutar telemetría
 */
static volatile bool flag_ejecutar_telemetria = false;

/**
 * @brief Flag para ejecutar heartbeat
 */
static volatile bool flag_ejecutar_heartbeat = false;

/**
 * @brief Flag para ejecutar lectura de sensores
 */
static volatile bool flag_ejecutar_sensores = false;

/**
 * @brief Flag para ejecutar verificación de conexión
 */
static volatile bool flag_ejecutar_conexion = false;

/**
 * @brief Flag de seguridad: evita ejecución antes de scheduler_start()
 */
static volatile bool scheduler_listo = false;

// ============================================
// CALLBACKS DE TIMERS
// ============================================

/**
 * @brief Callback del timer de telemetría
 * @details Activa flag y notifica a la tarea si está bloqueada
 */
static void callback_telemetria(TimerHandle_t xTimer) {
    flag_ejecutar_telemetria = true;
    if (taskTelemetriaHandle != NULL) {
        xTaskNotifyGive(taskTelemetriaHandle);
    }
}

/**
 * @brief Callback del timer de heartbeat
 * @details Comparte tarea con telemetría para optimizar recursos
 */
static void callback_heartbeat(TimerHandle_t xTimer) {
    flag_ejecutar_heartbeat = true;
    if (taskTelemetriaHandle != NULL) {
        xTaskNotifyGive(taskTelemetriaHandle);
    }
}

/**
 * @brief Callback del timer de lectura de sensores
 */
static void callback_sensores(TimerHandle_t xTimer) {
    flag_ejecutar_sensores = true;
    if (taskSensoresHandle != NULL) {
        xTaskNotifyGive(taskSensoresHandle);
    }
}

/**
 * @brief Callback del timer de RTC
 * @note La lectura RTC es liviana, se ejecuta directamente en el timer
 */
static void callback_rtc(TimerHandle_t xTimer) {
    rtc_actualizar();
}

/**
 * @brief Callback del timer de verificación de conexión
 */
static void callback_conexion(TimerHandle_t xTimer) {
    flag_ejecutar_conexion = true;
    if (taskConexionHandle != NULL) {
        xTaskNotifyGive(taskConexionHandle);
    }
}

// ============================================
// TAREAS DEDICADAS
// ============================================

/**
 * @brief Tarea dedicada para telemetría y heartbeat
 * @details Construye JSON y envía vía MQTT
 * @param pvParameters Parámetros de la tarea (no usado)
 */
static void tarea_telemetria(void* pvParameters) {
    while (!scheduler_listo) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    while (true) {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));
        
        if (flag_ejecutar_telemetria) {
            flag_ejecutar_telemetria = false;
            telemetry_enviar_telemetria();
        }
        
        if (flag_ejecutar_heartbeat) {
            flag_ejecutar_heartbeat = false;
            telemetry_enviar_heartbeat();
        }
    }
}

/**
 * @brief Tarea dedicada para lectura de sensores
 * @details Lee DHT22, DS18B20 y RS485 NPK
 * @param pvParameters Parámetros de la tarea (no usado)
 */
static void tarea_sensores(void* pvParameters) {
    while (!scheduler_listo) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    while (true) {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000));
        
        if (flag_ejecutar_sensores) {
            flag_ejecutar_sensores = false;
            sensors_leer_todos();
        }
    }
}

/**
 * @brief Tarea dedicada para verificación de conexión
 * @details Maneja WiFi y MQTT que pueden bloquear
 * @param pvParameters Parámetros de la tarea (no usado)
 */
static void tarea_conexion(void* pvParameters) {
    while (!scheduler_listo) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    while (true) {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10000));
        
        if (flag_ejecutar_conexion) {
            flag_ejecutar_conexion = false;
            connectivity_verificar();
        }
    }
}

// ============================================
// IMPLEMENTACIÓN
// ============================================

/**
 * @brief Inicializa el scheduler
 * @details Valida intervalos, crea tareas y timers FreeRTOS.
 *          Las tareas permanecen bloqueadas hasta scheduler_start().
 */
void scheduler_init() {
    DEBUG_INFO("SCHED: Validando intervalos antes de crear timers...");
    
    bool intervalos_corregidos = false;
    
    if (sistema.intervalos.telemetria == 0) {
        DEBUG_ERROR("SCHED: telemetria=0, restaurando a 60000ms");
        sistema.intervalos.telemetria = 60000;
        intervalos_corregidos = true;
    }
    if (sistema.intervalos.heartbeat == 0) {
        DEBUG_ERROR("SCHED: heartbeat=0, restaurando a 30000ms");
        sistema.intervalos.heartbeat = 30000;
        intervalos_corregidos = true;
    }
    if (sistema.intervalos.lecturaSensores == 0) {
        DEBUG_ERROR("SCHED: lecturaSensores=0, restaurando a 1000ms");
        sistema.intervalos.lecturaSensores = 1000;
        intervalos_corregidos = true;
    }
    if (sistema.intervalos.lecturaRTC == 0) {
        DEBUG_ERROR("SCHED: lecturaRTC=0, restaurando a 1000ms");
        sistema.intervalos.lecturaRTC = 1000;
        intervalos_corregidos = true;
    }
    if (sistema.intervalos.verificacionConexion == 0) {
        DEBUG_ERROR("SCHED: verificacionConexion=0, restaurando a 5000ms");
        sistema.intervalos.verificacionConexion = 5000;
        intervalos_corregidos = true;
    }
    
    if (intervalos_corregidos) {
        DEBUG_WARN("SCHED: Se corrigieron intervalos inválidos");
    } else {
        DEBUG_INFO("SCHED: Todos los intervalos son válidos");
    }
    
    DEBUG_INFOF("SCHED: Intervalos finales:");
    DEBUG_INFOF("  telemetria: %lu ms", sistema.intervalos.telemetria);
    DEBUG_INFOF("  heartbeat: %lu ms", sistema.intervalos.heartbeat);
    DEBUG_INFOF("  lecturaSensores: %lu ms", sistema.intervalos.lecturaSensores);
    DEBUG_INFOF("  lecturaRTC: %lu ms", sistema.intervalos.lecturaRTC);
    DEBUG_INFOF("  verificacionConexion: %lu ms", sistema.intervalos.verificacionConexion);
    
    // Crear tareas dedicadas
    xTaskCreatePinnedToCore(
        tarea_telemetria, "TareaTele",
        TASK_TELEMETRIA_STACK, NULL, TASK_PRIORITY,
        &taskTelemetriaHandle, 1
    );
    
    xTaskCreatePinnedToCore(
        tarea_sensores, "TareaSens",
        TASK_SENSORES_STACK, NULL, TASK_PRIORITY,
        &taskSensoresHandle, 1
    );
    
    xTaskCreatePinnedToCore(
        tarea_conexion, "TareaConn",
        TASK_CONEXION_STACK, NULL, TASK_PRIORITY,
        &taskConexionHandle, 0
    );
    
    DEBUG_INFO("SCHED: Tareas creadas");
    
    // Crear timers
    DEBUG_INFO("SCHED: Creando timers FreeRTOS...");
    
    timerTelemetria = xTimerCreate(
        "Telemetria", pdMS_TO_TICKS(sistema.intervalos.telemetria),
        pdTRUE, (void*)0, callback_telemetria
    );

    timerHeartbeat = xTimerCreate(
        "Heartbeat", pdMS_TO_TICKS(sistema.intervalos.heartbeat),
        pdTRUE, (void*)1, callback_heartbeat
    );

    timerLecturaSensores = xTimerCreate(
        "Sensores", pdMS_TO_TICKS(sistema.intervalos.lecturaSensores),
        pdTRUE, (void*)2, callback_sensores
    );

    timerLecturaRTC = xTimerCreate(
        "RTC", pdMS_TO_TICKS(sistema.intervalos.lecturaRTC),
        pdTRUE, (void*)3, callback_rtc
    );

    timerVerificacionConexion = xTimerCreate(
        "Conexion", pdMS_TO_TICKS(sistema.intervalos.verificacionConexion),
        pdTRUE, (void*)4, callback_conexion
    );

    DEBUG_INFO("SCHED: Todos los timers creados exitosamente");
}

/**
 * @brief Inicia los timers y permite ejecución de tareas
 */
void scheduler_start() {
    scheduler_listo = true;
    
    if (timerVerificacionConexion) xTimerStart(timerVerificacionConexion, 0);
    if (timerLecturaRTC) xTimerStart(timerLecturaRTC, 0);
    if (timerLecturaSensores) xTimerStart(timerLecturaSensores, 0);
    if (timerHeartbeat) xTimerStart(timerHeartbeat, 0);
    if (timerTelemetria) xTimerStart(timerTelemetria, 0);
    
    DEBUG_INFO("SCHED: Timers iniciados");
}

/**
 * @brief Detiene todos los timers
 */
void scheduler_stop() {
    if (timerTelemetria) xTimerStop(timerTelemetria, 0);
    if (timerHeartbeat) xTimerStop(timerHeartbeat, 0);
    if (timerLecturaSensores) xTimerStop(timerLecturaSensores, 0);
    if (timerLecturaRTC) xTimerStop(timerLecturaRTC, 0);
    if (timerVerificacionConexion) xTimerStop(timerVerificacionConexion, 0);
    
    DEBUG_INFO("SCHED: Timers detenidos");
}
