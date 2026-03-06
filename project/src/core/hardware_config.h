/**
 * @file hardware_config.h
 * @brief Definición de pines y configuración de hardware
 * @details Contiene todos los defines de pines GPIO, configuraciones de
 *          pantalla, tarjetas SD y parámetros de FreeRTOS.
 */

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// ============================================
// TFT Display (ILI9341)
// ============================================

/** @brief Pin MISO del TFT */
#define TFT_PIN_MISO    19
/** @brief Pin MOSI del TFT */
#define TFT_PIN_MOSI    23
/** @brief Pin SCLK del TFT */
#define TFT_PIN_SCLK    18
/** @brief Pin CS del TFT */
#define TFT_PIN_CS      15
/** @brief Pin DC del TFT */
#define TFT_PIN_DC      2
/** @brief Pin RST del TFT (-1 = sin pin) */
#define TFT_PIN_RST     -1

// ============================================
// Touch Screen (XPT2046)
// ============================================

/** @brief Pin CS del touchscreen */
#define TOUCH_PIN_CS    5
/** @brief Pin IRQ del touchscreen */
#define TOUCH_PIN_IRQ   17

// ============================================
// RTC DS3231 (I2C)
// ============================================

/** @brief Pin SDA del RTC */
#define RTC_PIN_SDA     21
/** @brief Pin SCL del RTC */
#define RTC_PIN_SCL     22

// ============================================
// Sensores
// ============================================

/** @brief Pin de datos del sensor DHT22 */
#define DHT22_PIN       26

/** @brief Tipo de sensor DHT (DHT22, DHT21, DHT11) */
#define DHT22         DHT22

/** @brief Pin de datos del sensor DS18B20 */
#define DS18B20_PIN     27

// ============================================
// RS485 NPK
// ============================================

/** @brief Pin RO (Receiver Output) del módulo RS485 */
#define RS485_RO_PIN    32
/** @brief Pin DI (Data Input) del módulo RS485 */
#define RS485_DI_PIN    25
/** @brief Pin DE (Driver Enable) del módulo RS485 */
#define RS485_DE_PIN    33

// ============================================
// SD Card
// ============================================

/** @brief Pin CS de la tarjeta SD */
#define SD_CS_PIN        4
/** @brief Pin CS de la tarjeta SD (alias) */
#define SD_PIN_CS       4

// ============================================
// Screen
// ============================================

/** @brief Ancho de pantalla en píxeles */
#define SCREEN_WIDTH    320
/** @brief Alto de pantalla en píxeles */
#define SCREEN_HEIGHT   240

// ============================================
// FreeRTOS Task Stacks
// ============================================

/** @brief Tamaño de stack para tarea de telemetría */
#define TASK_TELEMETRIA_STACK   4096
/** @brief Tamaño de stack para tarea de sensores */
#define TASK_SENSORES_STACK     3072
/** @brief Tamaño de stack para tarea de conexión */
#define TASK_CONEXION_STACK     4096
/** @brief Prioridad base de las tareas */
#define TASK_PRIORITY           1

// ============================================
// Intervalos por defecto
// ============================================

/** @brief Intervalo de telemetría: 1 minuto */
#define INTERVALO_TELEMETRIA_MS    60000
/** @brief Intervalo de heartbeat: 30 segundos */
#define INTERVALO_HEARTBEAT_MS     30000
/** @brief Intervalo de lectura de sensores: 1 segundo */
#define INTERVALO_SENSORES_MS      1000
/** @brief Intervalo de lectura RTC: 1 segundo */
#define INTERVALO_RTC_MS           1000
/** @brief Intervalo de verificación de conexión: 5 segundos */
#define INTERVALO_CONEXION_MS      5000

#endif
