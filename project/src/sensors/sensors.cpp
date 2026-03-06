/**
 * @file sensors.cpp
 * @brief Implementación de drivers de sensores
 * @details Controla DHT22, DS18B20 y RS485 NPK. Lee datos, valida rangos
 *          y almacena en buffers para filtrado por mediana.
 */

#include "sensors.h"
#include "../core/config_data.h"
#include "../core/debug.h"
#include "../core/hardware_config.h"
#include "sensor_buffer.h"

#include <DHT_U.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

// ============================================
// INSTANCIAS DE SENSORES
// ============================================

/**
 * @brief Instancia DHT22 para temperatura y humedad ambiente
 */
static DHT_Unified dht(DHT22_PIN, DHT22);

/**
 * @brief Instancia OneWire para DS18B20
 */
static OneWire oneWire(DS18B20_PIN);

/**
 * @brief Biblioteca DallasTemperature para DS18B20
 */
static DallasTemperature ds18b20(&oneWire);

/**
 * @brief Puerto serie software para RS485
 */
static SoftwareSerial rs485Serial(RS485_RO_PIN, RS485_DI_PIN);

// ============================================
// CONSTANTES RS485 (Modbus RTU)
// ============================================

/**
 * @brief Consulta Modbus para sensor NPK 7-in-1
 */
static const uint8_t RS485_QUERY[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

/**
 * @brief Tamaño de respuesta esperada del sensor NPK
 */
static const uint8_t RS485_RESPONSE_SIZE = 19;

/**
 * @brief Timeout para lectura RS485
 */
static const unsigned long RS485_TIMEOUT_MS = 1000;

// ============================================
// FUNCIONES AUXILIARES
// ============================================

/**
 * @brief Calcula CRC16 Modbus para validación de trama
 * @param data Puntero a datos
 * @param length Longitud de datos
 * @return CRC16 calculado
 */
static uint16_t calcular_crc16(const uint8_t* data, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// ============================================
// INICIALIZACIÓN
// ============================================

/**
 * @brief Inicializa todos los sensores
 * @details Configura DHT22, DS18B20, RS485 e inicializa buffers.
 */
void sensors_init() {
    DEBUG_INFO("SENS: Init");
    dht.begin();
    ds18b20.begin();
    uint8_t numDevices = ds18b20.getDeviceCount();
    if (numDevices > 0) {
        DEBUG_INFOF("SENS: DS18B20 x%d", numDevices);
    }
    rs485Serial.begin(4800);
    pinMode(RS485_DE_PIN, OUTPUT);
    digitalWrite(RS485_DE_PIN, LOW);
    sistema.sensores.dht22.valido = false;
    sistema.sensores.ds18b20.valido = false;
    sistema.sensores.rs485.valido = false;
    sensorBuffers.initAll();
}

// ============================================
// LECTURA DHT22
// ============================================

/**
 * @brief Lee temperatura y humedad del sensor DHT22
 * @details Valida rangos (-40 a 80°C, 0-100% humedad) y almacena en buffers
 */
void sensors_leer_dht22() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    float t = event.temperature;
    dht.humidity().getEvent(&event);
    float h = event.relative_humidity;

    if (isnan(h) || isnan(t)) {
        sistema.sensores.dht22.valido = false;
        return;
    }

    if (t < -40.0 || t > 80.0 || h < 0.0 || h > 100.0) {
        sistema.sensores.dht22.valido = false;
        return;
    }

    sistema.sensores.dht22.valido = true;
    sensorBuffers.tempAmb.add(t);
    sensorBuffers.humedad.add(h);
}

// ============================================
// LECTURA DS18B20
// ============================================

/**
 * @brief Lee temperatura del sensor DS18B20
 * @details Valida rango (-55 a 125°C) y almacena en buffer
 */
void sensors_leer_ds18b20() {
    ds18b20.requestTemperatures();
    float tempC = ds18b20.getTempCByIndex(0);

    if (tempC == DEVICE_DISCONNECTED_C || tempC == 85.0) {
        sistema.sensores.ds18b20.valido = false;
        return;
    }

    if (tempC < -55.0 || tempC > 125.0) {
        sistema.sensores.ds18b20.valido = false;
        return;
    }

    sistema.sensores.ds18b20.valido = true;
    sensorBuffers.tempAgua.add(tempC);
}

// ============================================
// LECTURA RS485 NPK
// ============================================

/**
 * @brief Lee sensor NPK 7-in-1 vía RS485 (Modbus RTU)
 * @details Envía consulta, valida respuesta CRC y parsea 7 parámetros del suelo:
 *          humedad, temperatura, EC, pH, N, P, K
 */
void sensors_leer_rs485() {
    uint8_t receivedData[RS485_RESPONSE_SIZE];
    
    while (rs485Serial.available() > 0) {
        rs485Serial.read();
    }
    
    digitalWrite(RS485_DE_PIN, HIGH);
    delay(10);
    rs485Serial.write(RS485_QUERY, sizeof(RS485_QUERY));
    rs485Serial.flush();
    digitalWrite(RS485_DE_PIN, LOW);
    delay(10);
    
    unsigned long startTime = millis();
    while (rs485Serial.available() < RS485_RESPONSE_SIZE) {
        if (millis() - startTime > RS485_TIMEOUT_MS) {
            DEBUG_ERROR("SENS: Timeout - No se recibieron suficientes datos del sensor NPK");
            sistema.sensores.rs485.valido = false;
            return;
        }
        delay(10);
    }
    
    rs485Serial.readBytes(receivedData, RS485_RESPONSE_SIZE);
    
    if (receivedData[0] != 0x01 || receivedData[1] != 0x03 || receivedData[2] != 0x0E) {
        DEBUG_ERROR("SENS: Respuesta Modbus invalida");
        DEBUG_ERRORF("SENS: Cabecera - Addr: 0x%02X, FC: 0x%02X, Count: 0x%02X", 
                     receivedData[0], receivedData[1], receivedData[2]);
        sistema.sensores.rs485.valido = false;
        return;
    }
    
    #if DEBUG_LEVEL >= 3
    DEBUG_INFO("SENS: Datos NPK recibidos (HEX):");
    char hexBuffer[64];
    for (int i = 0; i < RS485_RESPONSE_SIZE; i++) {
        snprintf(hexBuffer + (i * 3), 4, "%02X ", receivedData[i]);
    }
    DEBUG_INFO(hexBuffer);
    #endif
    
    sistema.sensores.rs485.humedad = (float)((uint16_t)((receivedData[3] << 8) | receivedData[4])) / 10.0;
    sistema.sensores.rs485.temperatura = (float)((int16_t)((receivedData[5] << 8) | receivedData[6])) / 10.0;
    sistema.sensores.rs485.ec = (float)((uint16_t)((receivedData[7] << 8) | receivedData[8]));
    sistema.sensores.rs485.ph = (float)((uint16_t)((receivedData[9] << 8) | receivedData[10])) / 10.0;
    sistema.sensores.rs485.nitrogen = (float)((uint16_t)((receivedData[11] << 8) | receivedData[12]));
    sistema.sensores.rs485.phosphorus = (float)((uint16_t)((receivedData[13] << 8) | receivedData[14]));
    sistema.sensores.rs485.potassium = (float)((uint16_t)((receivedData[15] << 8) | receivedData[16]));

    sistema.sensores.rs485.valido = true;
    
    sensorBuffers.humedadSuelo.add(sistema.sensores.rs485.humedad);
    sensorBuffers.ec.add(sistema.sensores.rs485.ec);
    sensorBuffers.ph.add(sistema.sensores.rs485.ph);
    sensorBuffers.nitrogen.add(sistema.sensores.rs485.nitrogen);
    sensorBuffers.phosphorus.add(sistema.sensores.rs485.phosphorus);
    sensorBuffers.potassium.add(sistema.sensores.rs485.potassium);
    
    #if DEBUG_LEVEL >= 3
    DEBUG_INFO("=== Datos Sensor NPK 7-in-1 ===");
    DEBUG_INFOF("Humedad suelo: %.1f %%", sistema.sensores.rs485.humedad);
    DEBUG_INFOF("Temperatura suelo: %.1f C", sistema.sensores.rs485.temperatura);
    DEBUG_INFOF("Conductividad: %.0f uS/cm", sistema.sensores.rs485.ec);
    DEBUG_INFOF("pH: %.1f", sistema.sensores.rs485.ph);
    DEBUG_INFOF("Nitrogeno: %.0f mg/kg", sistema.sensores.rs485.nitrogen);
    DEBUG_INFOF("Fosforo: %.0f mg/kg", sistema.sensores.rs485.phosphorus);
    DEBUG_INFOF("Potasio: %.0f mg/kg", sistema.sensores.rs485.potassium);
    DEBUG_INFO("================================");
    #endif
}

// ============================================
// FUNCIONES PÚBLICAS
// ============================================

/**
 * @brief Lee todos los sensores si la lectura está activa
 */
void sensors_leer_todos() {
    if (!sistema.controlUsuario.lecturaActivaUI) return;
    
    sistema.ultimaEjecucion.lecturaSensores = millis();
    sensors_leer_dht22();
    sensors_leer_ds18b20();
    sensors_leer_rs485();
    sistema.controlUsuario.lecturaSensoresManual = false;
}

/**
 * @brief Fuerza una lectura inmediata ignorando el flag de lectura activa
 */
void sensors_forzar_lectura() {
    sistema.controlUsuario.lecturaSensoresManual = true;
    sensors_leer_todos();
}
