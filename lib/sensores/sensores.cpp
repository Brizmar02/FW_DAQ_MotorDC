#include "sensores.h"
#include <config.h>
#include <Arduino.h>

// Variable global para guardar el "punto cero" calibrado
// Volvemos a poner 'static' si ya terminaste de depurar con main.cpp
float g_adc_offset_corriente = 0.0; 

// Número de muestras para el promedio en tiempo real
#define AVG_MUESTRAS 200

void sensores_init() {
    // 1. Configurar pines
    pinMode(PIN_SENSOR_CORRIENTE, INPUT);
    
    if (VOLTAGE_SENSOR_ENABLED) {
        pinMode(PIN_SENSOR_VOLTAJE, INPUT);
    }

    // --- MEJORA 1: TIEMPO DE ESTABILIZACIÓN ---
    // El filtro RC (10k + 10uF) tarda un tiempo en cargarse hasta 2.5V.
    // Esperamos 1 segundo completo para asegurar estabilidad eléctrica.
    delay(1000);

    // --- MEJORA 2: CALIBRACIÓN ROBUSTA ---
    // Tomamos un promedio largo para determinar el "cero" exacto.
    long sum_offset = 0;
    const int muestras_calibracion = 500;

    // Descartamos las primeras 10 lecturas por seguridad
    for(int i=0; i<10; i++) analogRead(PIN_SENSOR_CORRIENTE);

    for (int i = 0; i < muestras_calibracion; i++) {
        sum_offset += analogRead(PIN_SENSOR_CORRIENTE);
        // Un pequeño retardo entre muestras ayuda a reducir ruido aleatorio
        delayMicroseconds(500); 
    }
    
    // Guardamos el promedio como nuestro offset base
    g_adc_offset_corriente = (float)sum_offset / (float)muestras_calibracion;
}

float sensor_get_corriente_A() {
    // Usamos el método de PROMEDIO (no RMS) que funciona mejor con el filtro RC
    
    long suma_lecturas = 0;

    for (int i = 0; i < AVG_MUESTRAS; i++) {
        int valor_crudo = analogRead(PIN_SENSOR_CORRIENTE);
        
        // Restar el offset calibrado. 
        // Nota: Al restar floats, mantenemos la precisión decimal.
        suma_lecturas += ((float)valor_crudo - g_adc_offset_corriente);
    }

    // Promedio de las diferencias
    float valor_promedio_adc = (float)suma_lecturas / (float)AVG_MUESTRAS;

    // Convertir a Voltios (delta de voltaje respecto al centro)
    float voltaje_delta = (valor_promedio_adc / ADC_RESOLUTION) * VREF;

    // Convertir a Amperios
    // Usamos abs() porque la corriente puede oscilar levemente en negativo por ruido
    float corriente = abs(voltaje_delta / ACS712_SENSITIVITY);

    // Supresión de ruido de fondo (Deadzone)
    // Si la corriente es absurdamente baja (ej. < 20mA), forzamos a 0
    // para que se vea limpio en la gráfica.
    if (corriente < 0.02) {
        corriente = 0.0;
    }

    return corriente;
}

float sensor_get_voltaje_V() {
    if (!VOLTAGE_SENSOR_ENABLED) {
        return 0.0;
    }

    int valor_crudo = analogRead(PIN_SENSOR_VOLTAJE);
    float voltaje_adc = ((float)valor_crudo / ADC_RESOLUTION) * VREF;
    return voltaje_adc * VOLTAGE_DIVIDER_FACTOR;
}