#include "sensores.h"
#include "config.h" // Incluimos nuestra configuración

// --- VARIABLES PRIVADAS (Globales a este archivo) ---

/**
 * @brief Almacena el voltaje de "cero Amperios" del ACS712.
 * Se calibra una sola vez en sensores_init()
 */
static float g_acs712_offset_volts = 0.0;

// --- IMPLEMENTACIÓN DE FUNCIONES ---

/**
 * @brief Inicializa y calibra sensores.
 */
void sensores_init() {
    // 1. Configurar pines analógicos (buena práctica)
    pinMode(PIN_POTENCIOMETRO, INPUT);
    pinMode(PIN_SENSOR_CORRIENTE, INPUT);
    pinMode(PIN_SENSOR_VOLTAJE, INPUT);

    // 2. ¡LA CALIBRACIÓN CRÍTICA!
    // Mide el offset del ACS712 antes de que el motor consuma corriente.
    // Esto es lo que el código original hacía mal.
    
    long suma_adc = 0;
    const int muestras_calibracion = 1000; // Tomamos 1000 muestras para un offset preciso
    
    Serial.println("Calibrando sensor de corriente... No mueva el motor.");
    
    for (int i = 0; i < muestras_calibracion; i++) {
        suma_adc += analogRead(PIN_SENSOR_CORRIENTE);
        delay(1); // Pequeña pausa entre lecturas
    }

    // Calcular el voltaje promedio de offset
    float adc_promedio = (float)suma_adc / (float)muestras_calibracion;
    g_acs712_offset_volts = (adc_promedio * VREF) / ADC_RESOLUTION;

    Serial.print("Calibración completa. Offset ACS712: ");
    Serial.print(g_acs712_offset_volts, 4);
    Serial.println(" V");
}

/**
 * @brief Lee el valor crudo del potenciómetro.
 */
int sensores_leer_potenciometro_raw() {
    return analogRead(PIN_POTENCIOMETRO);
}

/**
 * @brief Mide la corriente.
 */
float sensores_leer_corriente() {
    // Para una lectura estable, tomamos un pequeño promedio.
    // No usamos un 'for' bloqueante como el código original.
    // Usaremos un filtro simple más adelante si es necesario,
    // por ahora, una lectura directa es más rápida.
    // (Nota: Para un sistema final, un filtro de promedio móvil es ideal)

    int lectura_adc = analogRead(PIN_SENSOR_CORRIENTE);
    
    // 1. Convertir lectura a voltaje
    float voltaje_sensor = (lectura_adc * VREF) / ADC_RESOLUTION;
    
    // 2. Restar el offset calibrado y calcular Amperios
    float corriente = (voltaje_sensor - g_acs712_offset_volts) / ACS712_SENSITIVITY;
    
    return corriente;
}

/**
 * @brief Mide el voltaje del motor.
 */
float sensores_leer_voltaje() {
    
    // Aquí usamos el "interruptor" de config.h
    if (VOLTAGE_SENSOR_ENABLED == false) {
        return 0.0; // Devolvemos 0.0V si no está instalado
    }

    // Si está habilitado, medimos:
    int lectura_adc = analogRead(PIN_SENSOR_VOLTAJE);
    
    // 1. Convertir lectura ADC al voltaje en el pin (ej. 2.4V)
    float voltaje_pin = (lectura_adc * VREF) / ADC_RESOLUTION;
    
    // 2. Aplicar el factor de multiplicación para obtener el voltaje real (ej. 12V)
    float voltaje_real = voltaje_pin * VOLTAGE_DIVIDER_FACTOR;
    
    return voltaje_real;
}