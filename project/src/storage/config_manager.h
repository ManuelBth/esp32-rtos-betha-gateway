/**
 * @file config_manager.h
 * @brief Gestión de configuración persistente en SD
 * @details Proporciona funciones para cargar y guardar la configuración
 *          del sistema en un archivo JSON en la tarjeta SD.
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

// ============================================
// CONSTANTES
// ============================================

/** @brief Ruta del archivo de configuración en la SD */
#define CONFIG_FILE_PATH "/config.json"

/** @brief Versión del formato de configuración */
#define CONFIG_VERSION 1

// ============================================
// API
// ============================================

/**
 * @brief Carga la configuración desde /config.json en SD
 * @return true si se cargó correctamente, false si falla
 * @note Si falla, sistema mantiene sus valores actuales
 */
bool config_load_from_sd();

/**
 * @brief Guarda la configuración actual en /config.json en SD
 * @return true si se guardó correctamente, false si falla
 * @note Sobrescribe el archivo existente
 */
bool config_save_to_sd();

/**
 * @brief Verifica si existe el archivo de configuración en SD
 * @return true si existe /config.json
 */
bool config_exists_on_sd();

#endif // CONFIG_MANAGER_H
