/**
 * @file scheduler.h
 * @brief Gestión de tareas y timers FreeRTOS
 * @details Proporciona funciones para inicializar, iniciar y detener
 *          el scheduler de tareas periódicas del sistema.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

// ============================================
// INICIALIZACIÓN
// ============================================

/**
 * @brief Inicializa el scheduler y crea tareas y timers FreeRTOS
 * @details Crea 3 tareas dedicadas (telemetría, sensores, conexión)
 *          y 5 timers periódicos. Las tareas permanecen bloqueadas
 *          hasta que se llama scheduler_start().
 */
void scheduler_start();

/**
 * @brief Inicia los timers FreeRTOS
 * @details Activa el flag scheduler_listo para permitir ejecución de tareas.
 *          Debe llamarse después de scheduler_init().
 */
void scheduler_init();

/**
 * @brief Detiene todos los timers FreeRTOS
 * @note Las tareas quedan en espera pero no se eliminan.
 */
void scheduler_stop();

#endif