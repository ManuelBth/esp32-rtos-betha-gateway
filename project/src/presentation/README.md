# Presentation Layer

Capa de presentación del sistema: UI TFT con LVGL y touch.

## Estructura

### UiHandler.cpp/h
- Inicializa hardware TFT (ILI9341) y Touch (XPT2046)
- Inicializa LVGL y carga pantallas de EEZ Studio
- `presentation_loop()` - ciclo de renderizado LVGL

### ui_updater.cpp/h
- **Puente entre datos del sistema y UI LVGL**
- Sincroniza `sistema.sensores.*` → variables LVGL
- Maneja cambios de cultivo e intervalos

### ui/ (Generado por EEZ Studio - NO editar manualmente)
- `screens.c/h` - Definiciones de pantallas LVGL
- `actions.h` - Declaraciones de acciones de la UI
- `vars.h` - Variables globales LVGL (binding con UI)
- `eez-flow.cpp/h` - Lógica de bindings y expresiones
- `fonts/` - Fuentes embebidas
- `images/` - Imágenes embebidas
- `styles.c/h` - Estilos LVGL

### ui_actions.cpp/h
- Callbacks de interacción de usuario (botones, sliders, etc.)
- Respuestas a eventos touch

## Flujo de datos

```
Sensores → sistema.sensores.* (config_data.h)
                ↓
        ui_updater_update() (cada 500ms)
                ↓
        setGlobalVariable() (vars.h)
                ↓
        g_globalVariables (EEZ Flow interno)
                ↓
        lv_label_set_text() → TFT Display
```

## Nota Importante

Los archivos en la carpeta `ui/` son **generados automáticamente por EEZ Studio**. 
Cualquier modificación debe hacerse en EEZ Studio y luego exportar/regenerar los archivos.

Para modificar la UI:
1. Abrir proyecto en EEZ Studio
2. Realizar cambios
3. Exportar → copiar archivos a `src/presentation/ui/`
