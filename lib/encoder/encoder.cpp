#include <encoder.h>
#include <config.h>
#include <Arduino.h>
#include <driver/pcnt.h>
#include <driver/gpio.h> // <--- VITAL para el arreglo
#include <esp_log.h>

static const char* TAG = "EncoderPCNT";
#define ENCODER_PCNT_UNIT PCNT_UNIT_0

// Variables para cálculo de velocidad
static int64_t g_last_position = 0;
static unsigned long g_last_time_ms = 0;
static float g_last_valid_rpm = 0.0; // Memoria para evitar caídas a 0

// Variables internas del driver
static int16_t g_last_raw_count = 0;
static int64_t g_accumulated_count = 0;

void encoder_init() {
    ESP_LOGI(TAG, "Iniciando Encoder PCNT en Pines %d y %d", PIN_ENC_A, PIN_ENC_B);

    // 1. Configurar Canal 0
    pcnt_config_t pcnt_config_a = {
        .pulse_gpio_num = PIN_ENC_A,
        .ctrl_gpio_num = PIN_ENC_B,
        .lctrl_mode = PCNT_MODE_REVERSE,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DEC,
        .counter_h_lim = 32767,
        .counter_l_lim = -32768,
        .unit = ENCODER_PCNT_UNIT,
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&pcnt_config_a);

    // 2. Configurar Canal 1
    pcnt_config_t pcnt_config_b = {
        .pulse_gpio_num = PIN_ENC_B,
        .ctrl_gpio_num = PIN_ENC_A,
        .lctrl_mode = PCNT_MODE_REVERSE,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_DEC,
        .neg_mode = PCNT_COUNT_INC,
        .counter_h_lim = 32767,
        .counter_l_lim = -32768,
        .unit = ENCODER_PCNT_UNIT,
        .channel = PCNT_CHANNEL_1,
    };
    pcnt_unit_config(&pcnt_config_b);

    // --- CORRECCIÓN MAESTRA ---
    // Activamos Pull-Ups DESPUÉS de configurar el PCNT
    // Esto asegura que el pin no quede flotando
    gpio_set_pull_mode((gpio_num_t)PIN_ENC_A, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)PIN_ENC_B, GPIO_PULLUP_ONLY);
    // --------------------------

    // 3. Filtro de Glitch (Vital para encoders mecánicos)
    pcnt_filter_enable(ENCODER_PCNT_UNIT);
    pcnt_set_filter_value(ENCODER_PCNT_UNIT, 100); 

    // 4. Iniciar
    pcnt_counter_pause(ENCODER_PCNT_UNIT);
    pcnt_counter_clear(ENCODER_PCNT_UNIT);
    pcnt_counter_resume(ENCODER_PCNT_UNIT);

    // Reset variables
    g_last_raw_count = 0;
    g_accumulated_count = 0;
    g_last_position = 0;
    g_last_time_ms = millis();
    g_last_valid_rpm = 0.0;
}

void encoder_reset_position() {
    pcnt_counter_clear(ENCODER_PCNT_UNIT);
    g_last_raw_count = 0;
    g_accumulated_count = 0;
    g_last_position = 0;
}

int64_t encoder_get_position() {
    int16_t current_raw_count = 0;
    pcnt_get_counter_value(ENCODER_PCNT_UNIT, &current_raw_count);
    int16_t delta = current_raw_count - g_last_raw_count;
    g_accumulated_count += delta;
    g_last_raw_count = current_raw_count;
    return g_accumulated_count;
}

float encoder_get_velocity_rpm() {
    unsigned long current_time_ms = millis();
    unsigned long delta_time_ms = current_time_ms - g_last_time_ms;

    // --- CORRECCIÓN DE MUESTREO ---
    // Si ha pasado muy poco tiempo (<40ms), el cálculo de velocidad es inestable.
    // Devolvemos el último valor bueno conocido.
    if (delta_time_ms < 40) {
        return g_last_valid_rpm;
    }

    int64_t current_position = encoder_get_position();
    int64_t delta_position = current_position - g_last_position;

    g_last_position = current_position;
    g_last_time_ms = current_time_ms;

    float pps = (float)delta_position / (float)delta_time_ms * 1000.0;
    float rps = pps / (float)ENCODER_PPR;
    float rpm = rps * 60.0;

    g_last_valid_rpm = rpm; // Guardamos para la próxima
    return rpm;
}