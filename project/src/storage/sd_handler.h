/**
 * @file sd_handler.h
 * @brief Manejo de bajo nivel de la tarjeta SD
 * @details Proporciona funciones para inicializar, leer, escribir
 *          y manipular archivos en la tarjeta SD.
 */

#ifndef SD_HANDLER_H
#define SD_HANDLER_H

#include <Arduino.h>
#include "../core/hardware_config.h"

// ============================================
// API
// ============================================

/**
 * @brief Inicializa y monta la tarjeta SD
 * @return true si se montó correctamente
 */
bool sd_init();

/**
 * @brief Verifica si la SD está montada
 * @return true si está disponible
 */
bool sd_is_mounted();

/**
 * @brief Verifica si un archivo existe
 * @param path Ruta del archivo
 * @return true si existe
 */
bool sd_file_exists(const char* path);

/**
 * @brief Lee el contenido de un archivo
 * @param path Ruta del archivo
 * @return Contenido del archivo (vacío si falla)
 */
String sd_read_file(const char* path);

/**
 * @brief Escribe contenido a un archivo
 * @param path Ruta del archivo
 * @param content Contenido a escribir
 * @return true si se escribió correctamente
 */
bool sd_write_file(const char* path, const String& content);

/**
 * @brief Obtiene el tamaño de un archivo
 * @param path Ruta del archivo
 * @return Tamaño en bytes
 */
uint32_t sd_get_file_size(const char* path);

/**
 * @brief Lista archivos en un directorio
 * @param dir Ruta del directorio
 */
void sd_list_files(const char* dir = "/");

#endif // SD_HANDLER_H
