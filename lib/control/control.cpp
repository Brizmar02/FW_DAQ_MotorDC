#include "control.h"
#include <config.h>
#include <Arduino.h>

// Incluimos todos los módulos
#include <potenciometro.h>
#include <motor.h>
#include <encoder.h>
#include <sensores.h>

// Definición del ADC_MAX_RAW
#define ADC_MAX_RAW 4095.0

// Variables para el timing del reporte serial (no usar delay)
unsigned long last_print_time = 0;
const int PRINT_INTERVAL_MS = 100; // Imprimir datos cada 100ms (10Hz)

void control_init() {
    Serial.begin(115200);
    // Esperar un poco para que el serial estabilice
    delay(1000);
    
    Serial.println("--- INICIANDO SISTEMA DE RECOLECCION DE DATOS ---");
    Serial.println("Formato CSV: Tiempo(ms), Setpoint(RPM), Real(RPM), Corriente(A), PWM(%)");

    // 1. Inicializar Sensores (CRÍTICO: Hacer esto con motor parado para calibrar corriente)
    Serial.println("Calibrando sensores...");
    sensores_init(); 

    // 2. Inicializar resto del hardware
    pot_init();
    motor_init();
    encoder_init(); 
    
    Serial.println("Sistema listo. Inicio de loop.");
    }

    // Función helper para mapeo flotante
    static float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void control_loop() {
    // --- 1. LECTURA DE ENTRADAS ---
    
    // Potenciómetro (Setpoint filtrado)
    float adc_pot = pot_get_filtered_value();
    
    // Encoder (Velocidad Real)
    float rpm_real = encoder_get_velocity_rpm();

    // Corriente (Consumo)
    float corriente_A = sensor_get_corriente_A();


    // --- 2. LÓGICA DE CONTROL (Lazo Abierto por ahora) ---
    
    float porcentaje_motor = 0.0;
    float rpm_setpoint = 0.0; // Solo para registro

    // Lógica de Zona Muerta y Bidireccional
    if (adc_pot > (POT_MID_POINT + POT_DEADZONE)) {
        // ADELANTE
        porcentaje_motor = mapf(adc_pot, POT_MID_POINT + POT_DEADZONE, ADC_MAX_RAW, 0.0, 100.0);
        // Calculamos qué RPM "desea" el usuario teóricamente (solo para referencia)
        rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM;

    } else if (adc_pot < (POT_MID_POINT - POT_DEADZONE)) {
        // REVERSA
        porcentaje_motor = mapf(adc_pot, 0.0, POT_MID_POINT - POT_DEADZONE, -100.0, 0.0);
        rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM; // Será negativo
    }
    
    // Limites de seguridad
    if (porcentaje_motor > 100.0) porcentaje_motor = 100.0;
    if (porcentaje_motor < -100.0) porcentaje_motor = -100.0;


    // --- 3. ACTUACIÓN ---
    motor_move(porcentaje_motor);


    // --- 4. TELEMETRÍA (REPORTE DE DATOS) ---
    // Usamos millis() para no bloquear el loop con delays
    unsigned long current_time = millis();
    
    if (current_time - last_print_time >= PRINT_INTERVAL_MS) {
        last_print_time = current_time;
        
        // Imprimimos en formato CSV:
        // Tiempo, Setpoint, VelocidadReal, Corriente, PWM_Enviado
        Serial.printf("%lu, %.2f, %.2f, %.3f, %.1f\n", 
                    current_time, 
                    rpm_setpoint, 
                    rpm_real, 
                    corriente_A, 
                    porcentaje_motor);
    }
}