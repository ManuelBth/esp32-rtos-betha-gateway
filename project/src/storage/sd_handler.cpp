/**
 * @file sd_handler.cpp
 * @brief Implementación de manejo de SD
 * @details Funciones de bajo nivel para inicializar, leer, escribir
 *          y listar archivos en la tarjeta SD.
 */

#include "sd_handler.h"
#include "../core/debug.h"
#include "../core/hardware_config.h"
#include <SD.h>
#include <SPI.h>

// ============================================
// ESTADO
// ============================================

static bool sd_mounted = false;

// ============================================
// INICIALIZACIÓN
// ============================================

/**
 * @brief Inicializa y monta la tarjeta SD
 */
bool sd_init() {
    DEBUG_INFO("SD: Inicializando...");
    
    if (!SD.begin(SD_CS_PIN)) {
        DEBUG_ERROR("SD: Fallo al montar");
        sd_mounted = false;
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    
    if (cardType == CARD_NONE) {
        DEBUG_ERROR("SD: No detectada");
        sd_mounted = false;
        return false;
    }
    
    sd_mounted = true;
    
    const char* typeStr = "UNKNOWN";
    if (cardType == CARD_MMC) typeStr = "MMC";
    else if (cardType == CARD_SD) typeStr = "SDSC";
    else if (cardType == CARD_SDHC) typeStr = "SDHC";
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    
    DEBUG_INFOF("SD: OK - Tipo:%s Tamaño:%lluMB", typeStr, cardSize);
    
    return true;
}

/**
 * @brief Verifica si la SD está montada
 */
bool sd_is_mounted() {
    return sd_mounted;
}

// ============================================
// OPERACIONES CON ARCHIVOS
// ============================================

/**
 * @brief Verifica si un archivo existe
 */
bool sd_file_exists(const char* path) {
    if (!sd_mounted) return false;
    return SD.exists(path);
}

/**
 * @brief Lee el contenido de un archivo
 */
String sd_read_file(const char* path) {
    if (!sd_mounted) {
        DEBUG_ERROR("SD: No montada (read)");
        return "";
    }
    
    if (!SD.exists(path)) {
        DEBUG_WARNF("SD: Archivo no existe: %s", path);
        return "";
    }
    
    File file = SD.open(path, FILE_READ);
    if (!file) {
        DEBUG_ERRORF("SD: No se puede abrir: %s", path);
        return "";
    }
    
    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    
    file.close();
    
    DEBUG_INFOF("SD: Leído %s (%d bytes)", path, content.length());
    
    return content;
}

/**
 * @brief Escribe contenido a un archivo
 */
bool sd_write_file(const char* path, const String& content) {
    if (!sd_mounted) {
        DEBUG_ERROR("SD: No montada (write)");
        return false;
    }
    
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        DEBUG_ERRORF("SD: No se puede crear: %s", path);
        return false;
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    if (bytesWritten != content.length()) {
        DEBUG_ERRORF("SD: Escritura incompleta: %s", path);
        return false;
    }
    
    DEBUG_INFOF("SD: Guardado %s (%d bytes)", path, bytesWritten);
    
    return true;
}

/**
 * @brief Obtiene el tamaño de un archivo
 */
uint32_t sd_get_file_size(const char* path) {
    if (!sd_mounted) return 0;
    
    if (!SD.exists(path)) return 0;
    
    File file = SD.open(path, FILE_READ);
    if (!file) return 0;
    
    uint32_t size = file.size();
    file.close();
    
    return size;
}

// ============================================
// DEBUG
// ============================================

/**
 * @brief Lista archivos en un directorio
 */
void sd_list_files(const char* dir) {
    if (!sd_mounted) {
        DEBUG_ERROR("SD: No montada (list)");
        return;
    }
    
    File root = SD.open(dir);
    if (!root) {
        DEBUG_ERRORF("SD: No se puede abrir dir: %s", dir);
        return;
    }
    
    if (!root.isDirectory()) {
        DEBUG_ERRORF("SD: No es directorio: %s", dir);
        root.close();
        return;
    }
    
    DEBUG_INFOF("SD: Listando archivos en %s", dir);
    
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            DEBUG_INFOF("  DIR:  %s", file.name());
        } else {
            DEBUG_INFOF("  FILE: %s (%lu bytes)", file.name(), file.size());
        }
        file.close();
        file = root.openNextFile();
    }
    
    root.close();
}
