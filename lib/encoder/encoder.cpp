#include <encoder.h>
#include <config.h>
#include <Arduino.h>

// Incluimos el driver de hardware ANTIGUO (Legacy)
#include <driver/pcnt.h>
#include <esp_log.h>

// --- VARIABLES PRIVADAS (Globales a este archivo) ---

static const char* TAG = "EncoderPCNT"; // Para mensajes de depuración

// Definimos qué unidad de hardware usaremos (El S3 tiene 8, de 0 a 7)
#define ENCODER_PCNT_UNIT PCNT_UNIT_0

// Para el cálculo de velocidad
static int64_t g_last_position = 0;
static unsigned long g_last_time_ms = 0;

// Para manejar el desborde (overflow) del contador de 16 bits
static int16_t g_last_raw_count = 0;      // Último valor leído del hardware
static int64_t g_accumulated_count = 0; // Nuestro contador "infinito" de 64 bits

// --- IMPLEMENTACIÓN DE FUNCIONES ---

void encoder_init() {
    ESP_LOGI(TAG, "Inicializando encoder con PCNT (Driver Antiguo)...");

    // 1. Configurar Canal A (Pin A como pulso, Pin B como control)
    pcnt_config_t pcnt_config_a = {
        // Pines
        .pulse_gpio_num = PIN_ENC_A,
        .ctrl_gpio_num = PIN_ENC_B,
        
        // Modos (ESTE ES EL ORDEN CORRECTO)
        .lctrl_mode = PCNT_MODE_REVERSE,  // B=0 -> Invierte lógica de pos/neg
        .hctrl_mode = PCNT_MODE_KEEP,     // B=1 -> Mantiene lógica de pos/neg
        .pos_mode = PCNT_COUNT_INC,     // B=0, A sube -> Incrementa
        .neg_mode = PCNT_COUNT_DEC,     // B=0, A baja -> Decrementa

        // Límites
        .counter_h_lim = 32767,
        .counter_l_lim = -32768,

        // Unidad y Canal
        .unit = ENCODER_PCNT_UNIT,
        .channel = PCNT_CHANNEL_0,
    };
    ESP_ERROR_CHECK(pcnt_unit_config(&pcnt_config_a));

    // 2. Configurar Canal B (Pin B como pulso, Pin A como control)
    pcnt_config_t pcnt_config_b = {
        // Pines
        .pulse_gpio_num = PIN_ENC_B,
        .ctrl_gpio_num = PIN_ENC_A,
        
        // Modos (ESTE ES EL ORDEN CORRECTO)
        .lctrl_mode = PCNT_MODE_REVERSE,  // A=0 -> Invierte lógica de pos/neg
        .hctrl_mode = PCNT_MODE_KEEP,     // A=1 -> Mantiene lógica de pos/neg
        .pos_mode = PCNT_COUNT_DEC,     // A=0, B sube -> Decrementa
        .neg_mode = PCNT_COUNT_INC,     // A=0, B baja -> Incrementa

        // Límites
        .counter_h_lim = 32767,
        .counter_l_lim = -32768,

        // Unidad y Canal
        .unit = ENCODER_PCNT_UNIT,
        .channel = PCNT_CHANNEL_1,
    };
    ESP_ERROR_CHECK(pcnt_unit_config(&pcnt_config_b));

    // 3. Añadir un filtro de ruido (¡muy importante!)
    // Tu valor era 1000ns. 1000ns / (1 / 80MHz) = 80 ciclos de reloj APB.
    ESP_ERROR_CHECK(pcnt_filter_enable(ENCODER_PCNT_UNIT));
    ESP_ERROR_CHECK(pcnt_set_filter_value(ENCODER_PCNT_UNIT, 80));

    // 4. Iniciar la unidad PCNT
    ESP_ERROR_CHECK(pcnt_counter_pause(ENCODER_PCNT_UNIT));
    ESP_ERROR_CHECK(pcnt_counter_clear(ENCODER_PCNT_UNIT)); // Poner contador a 0
    ESP_ERROR_CHECK(pcnt_counter_resume(ENCODER_PCNT_UNIT));

    // Inicializar variables de software
    g_last_raw_count = 0;
    g_accumulated_count = 0;
    g_last_position = 0;
    g_last_time_ms = millis(); // Inicializar timer de velocidad
    
    ESP_LOGI(TAG, "PCNT (Driver Antiguo) inicializado y corriendo.");
}

void encoder_reset_position() {
    ESP_ERROR_CHECK(pcnt_counter_clear(ENCODER_PCNT_UNIT));
    g_last_raw_count = 0;
    g_accumulated_count = 0;
    g_last_position = 0;
}

int64_t encoder_get_position() {
    int16_t current_raw_count = 0;
    // Leer el valor actual del hardware (un valor de 16 bits)
    ESP_ERROR_CHECK(pcnt_get_counter_value(ENCODER_PCNT_UNIT, &current_raw_count));

    // Calcular el delta (diferencia) desde la última lectura
    // Esta resta de int16_t maneja automáticamente el desborde (wrap-around)
    // Ejemplo: (nuevo) -32768 - (viejo) 32767 = 1 (se movió un pulso positivo)
    int16_t delta = current_raw_count - g_last_raw_count;

    // Añadir ese delta a nuestro contador acumulado de 64 bits
    g_accumulated_count += delta;
    
    // Guardar el valor raw actual para la próxima llamada
    g_last_raw_count = current_raw_count;

    return g_accumulated_count;
}

float encoder_get_velocity_rpm() {
    unsigned long current_time_ms = millis();
    unsigned long delta_time_ms = current_time_ms - g_last_time_ms;

    // Evitar division por cero y actualizar solo en intervalos razonables (ej. > 10ms)
    if (delta_time_ms < 10) {
        // No ha pasado suficiente tiempo, devolver la última velocidad calculada (o 0)
        // (Mejora: devolver la última velocidad válida)
        return 0; // Simplificación por ahora
    }

    // Obtener posición acumulada actual (esto ahora es un int64_t gracias
    // a la nueva lógica en encoder_get_position())
    int64_t current_position = encoder_get_position();
    
    // El delta ahora es una resta limpia de 64 bits
    int64_t delta_position = current_position - g_last_position;

    // Guardar estado actual para la próxima llamada
    g_last_position = current_position;
    g_last_time_ms = current_time_ms;

    // --- Cálculo ---
    // 1. Pulsos por segundo (PPS)
    // (pulsos / milisegundos) * 1000 = pulsos / segundo
    float pps = (float)delta_position / (float)delta_time_ms * 1000.0;

    // 2. Revoluciones por segundo (RPS)
    // (pulsos / seg) / (pulsos / rev) = rev / seg
    float rps = pps / (float)ENCODER_PPR;

    // 3. Revoluciones por minuto (RPM)
    // (rev / seg) * 60 = rev / min
    float rpm = rps * 60.0;

    return rpm;
}