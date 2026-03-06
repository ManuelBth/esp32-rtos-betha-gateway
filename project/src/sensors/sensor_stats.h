/**
 * @file sensor_stats.h
 * @brief Cálculo de estadísticas de sensores
 * @details Proporciona funciones para calcular medianas de los buffers
 *          y limpiar los datos para nuevas lecturas.
 */

#ifndef SENSOR_STATS_H
#define SENSOR_STATS_H

/**
 * @brief Calcula las medianas de todos los buffers y guarda en sistema.sensores
 * @details Lee sensorBuffers.* y actualiza sistema.sensores.* con valores filtrados
 * @note Los valores en sistema.sensores.* son las medianas finales para UI y telemetría
 */
void sensor_stats_calcular_medianas();

/**
 * @brief Limpia todos los buffers circulares
 * @details Reinicia contadores y marca sensores como inválidos.
 *          Llamar cuando el usuario activa "comenzar_medir" para datos frescos.
 */
void sensor_stats_limpiar_buffers();

#endif // SENSOR_STATS_H
