/**
 * @file config_manager.cpp
 * @brief Implementación de gestión de configuración
 * @details Serializa y parsea la estructura sistema a/from JSON
 *          para persistencia en tarjeta SD.
 */

#include "config_manager.h"
#include "sd_handler.h"
#include "../core/config_data.h"
#include "../core/debug.h"
#include "../../lib/custom_json/SimpleJson.h"
#include <time.h>

// ============================================
// VERIFICAR EXISTENCIA
// ============================================

bool config_exists_on_sd() {
    return sd_file_exists(CONFIG_FILE_PATH);
}

// ============================================
// GUARDAR CONFIGURACIÓN
// ============================================

/**
 * @brief Serializa sistema a JSON y guarda en SD
 */
bool config_save_to_sd() {
    if (!sd_is_mounted()) {
        DEBUG_ERROR("CFG: SD no montada");
        return false;
    }
    
    DEBUG_INFO("CFG: Serializando configuración...");
    
    SimpleJson json;
    json.create_object();
    
    json.add_int("version", CONFIG_VERSION);
    
    // WiFi
    JsonValue* wifi = json.add_object("wifi");
    if (wifi) {
        wifi->add_string("ssid", sistema.wifi.ssid);
        wifi->add_string("password", sistema.wifi.password);
    }
    
    // MQTT
    JsonValue* mqtt = json.add_object("mqtt");
    if (mqtt) {
        mqtt->add_string("broker", sistema.mqtt.broker);
        mqtt->add_number("port", sistema.mqtt.port);
        mqtt->add_string("username", sistema.mqtt.username);
        mqtt->add_string("password", sistema.mqtt.password);
        mqtt->add_string("topic", sistema.mqtt.topic);
        mqtt->add_string("user_id", sistema.mqtt.user_id);
    }
    
    // Dispositivo
    JsonValue* dispositivo = json.add_object("dispositivo");
    if (dispositivo) {
        dispositivo->add_string("modelo", sistema.dispositivo.modelo);
        dispositivo->add_string("serie", sistema.dispositivo.serie);
        dispositivo->add_string("firmware", sistema.dispositivo.vFirmware);
        dispositivo->add_string("hardware", sistema.dispositivo.vHardware);
        dispositivo->add_string("mac", sistema.dispositivo.mac);
    }
    
    // Cultivos
    JsonValue* cultivos_array = json.add_array("cultivos");
    if (cultivos_array) {
        for (uint8_t i = 0; i < sistema.cultivos.numCultivos; i++) {
            JsonValue* cultivo = new JsonValue(JsonType::OBJECT);
            cultivo->add_string("nombre", sistema.cultivos.cultivos[i].nombre);
            
            JsonValue* variables = new JsonValue(JsonType::ARRAY);
            for (int j = 0; j < 16; j++) {
                if (sistema.cultivos.cultivos[i].variables[j].length() > 0) {
                    JsonValue* var_str = new JsonValue(JsonType::STRING);
                    var_str->set_string(sistema.cultivos.cultivos[i].variables[j]);
                    variables->add_array_value(var_str);
                }
            }
            cultivo->set_object_value("variables", variables);
            
            cultivos_array->add_array_value(cultivo);
        }
    }
    
    // Intervalos
    JsonValue* intervalos = json.add_object("intervalos");
    if (intervalos) {
        uint8_t telemetria_min = sistema.intervalos.telemetria / 60000;
        intervalos->add_number("telemetria_min", telemetria_min);
        intervalos->add_number("lectura_sensores_ms", sistema.intervalos.lecturaSensores);
        intervalos->add_number("lectura_rtc_ms", sistema.intervalos.lecturaRTC);
        intervalos->add_number("heartbeat_ms", sistema.intervalos.heartbeat);
        intervalos->add_number("verificacion_conexion_ms", sistema.intervalos.verificacionConexion);
    }
    
    // Control usuario
    JsonValue* control = json.add_object("control_usuario");
    if (control) {
        control->add_number("cultivo_seleccionado_idx", sistema.controlUsuario.cultivoSeleccionado);
    }
    
    // Sistema metadata
    JsonValue* sistema_meta = json.add_object("sistema");
    if (sistema_meta) {
        time_t now;
        time(&now);
        sistema.sistema.ultima_actualizacion = now;
        
        sistema_meta->add_number("ultima_actualizacion", (double)sistema.sistema.ultima_actualizacion);
        sistema_meta->add_number("ultima_config_servidor", (double)sistema.sistema.ultima_config_servidor);
    }
    
    String json_str = json.to_string();
    
    DEBUG_INFOF("CFG: JSON generado (%d bytes)", json_str.length());
    
    if (sd_write_file(CONFIG_FILE_PATH, json_str)) {
        DEBUG_INFO("CFG: Guardada en SD OK");
        return true;
    } else {
        DEBUG_ERROR("CFG: Fallo al guardar en SD");
        return false;
    }
}

// ============================================
// CARGAR CONFIGURACIÓN
// ============================================

/**
 * @brief Lee JSON desde SD y parsea a sistema
 */
bool config_load_from_sd() {
    if (!sd_is_mounted()) {
        DEBUG_ERROR("CFG: SD no montada");
        return false;
    }
    
    if (!config_exists_on_sd()) {
        DEBUG_WARN("CFG: Archivo no existe");
        return false;
    }
    
    String json_str = sd_read_file(CONFIG_FILE_PATH);
    if (json_str.length() == 0) {
        DEBUG_ERROR("CFG: Archivo vacío");
        return false;
    }
    
    DEBUG_INFOF("CFG: Parseando JSON (%d bytes)...", json_str.length());
    
    SimpleJson json;
    if (!json.parse(json_str)) {
        DEBUG_ERROR("CFG: JSON inválido");
        return false;
    }
    
    // Validar versión
    int version = json.get_int("version", 0);
    if (version != CONFIG_VERSION) {
        DEBUG_WARNF("CFG: Versión %d != %d", version, CONFIG_VERSION);
    }
    
    // Cargar WiFi
    JsonValue* wifi = json.get_value("wifi");
    if (wifi && wifi->get_type() == JsonType::OBJECT) {
        JsonValue* ssid = wifi->get_object_value("ssid");
        JsonValue* password = wifi->get_object_value("password");
        
        if (ssid && ssid->get_type() == JsonType::STRING) {
            snprintf(sistema.wifi.ssid, sizeof(sistema.wifi.ssid), "%s", ssid->as_string().c_str());
        }
        if (password && password->get_type() == JsonType::STRING) {
            snprintf(sistema.wifi.password, sizeof(sistema.wifi.password), "%s", password->as_string().c_str());
        }
    }
    
    // Cargar MQTT
    JsonValue* mqtt = json.get_value("mqtt");
    if (mqtt && mqtt->get_type() == JsonType::OBJECT) {
        JsonValue* broker = mqtt->get_object_value("broker");
        JsonValue* port = mqtt->get_object_value("port");
        JsonValue* username = mqtt->get_object_value("username");
        JsonValue* password = mqtt->get_object_value("password");
        JsonValue* topic = mqtt->get_object_value("topic");
        JsonValue* user_id = mqtt->get_object_value("user_id");
        
        if (broker && broker->get_type() == JsonType::STRING) {
            snprintf(sistema.mqtt.broker, sizeof(sistema.mqtt.broker), "%s", broker->as_string().c_str());
        }
        if (port && port->get_type() == JsonType::NUMBER) {
            sistema.mqtt.port = port->as_int();
        }
        if (username && username->get_type() == JsonType::STRING) {
            snprintf(sistema.mqtt.username, sizeof(sistema.mqtt.username), "%s", username->as_string().c_str());
        }
        if (password && password->get_type() == JsonType::STRING) {
            snprintf(sistema.mqtt.password, sizeof(sistema.mqtt.password), "%s", password->as_string().c_str());
        }
        if (topic && topic->get_type() == JsonType::STRING) {
            snprintf(sistema.mqtt.topic, sizeof(sistema.mqtt.topic), "%s", topic->as_string().c_str());
        }
        if (user_id && user_id->get_type() == JsonType::STRING) {
            snprintf(sistema.mqtt.user_id, sizeof(sistema.mqtt.user_id), "%s", user_id->as_string().c_str());
        }
    }
    
    // Cargar Dispositivo
    JsonValue* dispositivo = json.get_value("dispositivo");
    if (dispositivo && dispositivo->get_type() == JsonType::OBJECT) {
        JsonValue* modelo = dispositivo->get_object_value("modelo");
        JsonValue* serie = dispositivo->get_object_value("serie");
        JsonValue* firmware = dispositivo->get_object_value("firmware");
        JsonValue* hardware = dispositivo->get_object_value("hardware");
        JsonValue* mac = dispositivo->get_object_value("mac");
        
        if (modelo && modelo->get_type() == JsonType::STRING) {
            snprintf(sistema.dispositivo.modelo, sizeof(sistema.dispositivo.modelo), "%s", modelo->as_string().c_str());
        }
        if (serie && serie->get_type() == JsonType::STRING) {
            snprintf(sistema.dispositivo.serie, sizeof(sistema.dispositivo.serie), "%s", serie->as_string().c_str());
        }
        if (firmware && firmware->get_type() == JsonType::STRING) {
            snprintf(sistema.dispositivo.vFirmware, sizeof(sistema.dispositivo.vFirmware), "%s", firmware->as_string().c_str());
        }
        if (hardware && hardware->get_type() == JsonType::STRING) {
            snprintf(sistema.dispositivo.vHardware, sizeof(sistema.dispositivo.vHardware), "%s", hardware->as_string().c_str());
        }
        if (mac && mac->get_type() == JsonType::STRING) {
            snprintf(sistema.dispositivo.mac, sizeof(sistema.dispositivo.mac), "%s", mac->as_string().c_str());
        }
    }
    
    // Cargar Cultivos
    JsonValue* cultivos_array = json.get_value("cultivos");
    if (cultivos_array && cultivos_array->get_type() == JsonType::ARRAY) {
        int num_cultivos = cultivos_array->get_array_size();
        if (num_cultivos > MAX_CULTIVOS) num_cultivos = MAX_CULTIVOS;
        
        sistema.cultivos.numCultivos = num_cultivos;
        
        for (int i = 0; i < num_cultivos; i++) {
            JsonValue* cultivo = cultivos_array->get_array_value(i);
            if (!cultivo || cultivo->get_type() != JsonType::OBJECT) continue;
            
            JsonValue* nombre = cultivo->get_object_value("nombre");
            if (nombre && nombre->get_type() == JsonType::STRING) {
                snprintf(sistema.cultivos.cultivos[i].nombre, 
                        sizeof(sistema.cultivos.cultivos[i].nombre), 
                        "%s", nombre->as_string().c_str());
            }
            
            JsonValue* variables = cultivo->get_object_value("variables");
            if (variables && variables->get_type() == JsonType::ARRAY) {
                int num_vars = variables->get_array_size();
                if (num_vars > 16) num_vars = 16;
                
                for (int j = 0; j < num_vars; j++) {
                    JsonValue* var_val = variables->get_array_value(j);
                    if (var_val && var_val->get_type() == JsonType::STRING) {
                        sistema.cultivos.cultivos[i].variables[j] = var_val->as_string();
                    }
                }
            }
        }
        
        DEBUG_INFOF("CFG: %d cultivos cargados", sistema.cultivos.numCultivos);
    }
    
    // Cargar Intervalos
    JsonValue* intervalos = json.get_value("intervalos");
    if (intervalos && intervalos->get_type() == JsonType::OBJECT) {
        JsonValue* telemetria_min = intervalos->get_object_value("telemetria_min");
        JsonValue* lectura_sensores = intervalos->get_object_value("lectura_sensores_ms");
        JsonValue* lectura_rtc = intervalos->get_object_value("lectura_rtc_ms");
        JsonValue* heartbeat = intervalos->get_object_value("heartbeat_ms");
        JsonValue* verificacion = intervalos->get_object_value("verificacion_conexion_ms");
        
        if (telemetria_min && telemetria_min->get_type() == JsonType::NUMBER) {
            uint8_t min = telemetria_min->as_int();
            sistema.intervalos.telemetria = min * 60000UL;
            sistema.controlUsuario.tiempoTelemetriaMin = min;
            DEBUG_INFOF("CFG: telemetria=%lu ms", sistema.intervalos.telemetria);
        }
        if (lectura_sensores && lectura_sensores->get_type() == JsonType::NUMBER) {
            sistema.intervalos.lecturaSensores = lectura_sensores->as_int();
            DEBUG_INFOF("CFG: lecturaSensores=%lu ms", sistema.intervalos.lecturaSensores);
        }
        if (lectura_rtc && lectura_rtc->get_type() == JsonType::NUMBER) {
            sistema.intervalos.lecturaRTC = lectura_rtc->as_int();
            DEBUG_INFOF("CFG: lecturaRTC=%lu ms", sistema.intervalos.lecturaRTC);
        }
        if (heartbeat && heartbeat->get_type() == JsonType::NUMBER) {
            sistema.intervalos.heartbeat = heartbeat->as_int();
            DEBUG_INFOF("CFG: heartbeat=%lu ms", sistema.intervalos.heartbeat);
        }
        if (verificacion && verificacion->get_type() == JsonType::NUMBER) {
            sistema.intervalos.verificacionConexion = verificacion->as_int();
            DEBUG_INFOF("CFG: verificacionConexion=%lu ms", sistema.intervalos.verificacionConexion);
        }
    }
    
    // Cargar Control Usuario
    JsonValue* control = json.get_value("control_usuario");
    if (control && control->get_type() == JsonType::OBJECT) {
        JsonValue* cultivo_idx = control->get_object_value("cultivo_seleccionado_idx");
        
        if (cultivo_idx && cultivo_idx->get_type() == JsonType::NUMBER) {
            uint8_t idx = cultivo_idx->as_int();
            if (idx < sistema.cultivos.numCultivos) {
                sistema.controlUsuario.cultivoSeleccionado = idx;
            }
        }
    }
    
    // Cargar Sistema metadata
    JsonValue* sistema_meta = json.get_value("sistema");
    if (sistema_meta && sistema_meta->get_type() == JsonType::OBJECT) {
        JsonValue* ultima_act = sistema_meta->get_object_value("ultima_actualizacion");
        JsonValue* ultima_cfg = sistema_meta->get_object_value("ultima_config_servidor");
        
        if (ultima_act && ultima_act->get_type() == JsonType::NUMBER) {
            sistema.sistema.ultima_actualizacion = ultima_act->as_uint64();
        }
        if (ultima_cfg && ultima_cfg->get_type() == JsonType::NUMBER) {
            sistema.sistema.ultima_config_servidor = ultima_cfg->as_uint64();
        }
    }
    
    DEBUG_INFO("CFG: Cargada desde SD OK");
    return true;
}
