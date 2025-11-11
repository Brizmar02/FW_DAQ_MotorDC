#pragma once // Encabezado de guarda, previene doble inclusión

#include <Arduino.h> // Para los tipos como const int, uint8_t

// =========================================
// === CONFIGURACIÓN DE PINES (Hardware) ===
// =========================================

// --- Driver del Motor (TB6612) ---
const int PIN_MOTOR_IN1  = 11;
const int PIN_MOTOR_IN2  = 12;
const int PIN_MOTOR_PWM  = 17;
const int PIN_MOTOR_STBY = 14;

// --- Sensores Analógicos (ADC) ---
const int PIN_POTENCIOMETRO = 4;
const int PIN_SENSOR_CORRIENTE = 5;

// ¡IMPORTANTE! Asigna un pin analógico libre para el divisor de voltaje
// Sugerencia: GPIO 6. Confirma que esté libre en tu protoboard.
const int PIN_SENSOR_VOLTAJE = 6; 

// --- Encoder ---
const int PIN_ENC_A = 36;
const int PIN_ENC_B = 37;

// ====================================
// === PARÁMETROS DE FUNCIONAMIENTO ===
// ====================================

// --- PWM (LEDC Periférico) ---
const int PWM_MOTOR_CHANNEL = 0;       // Canal LEDC 0
const int PWM_MOTOR_FREQ = 1220;             // Frecuencia (Hz)
const int PWM_MOTOR_RES_BITS = 10;           // Resolución de 10 bits
const int PWM_MOTOR_MAX_DUTY = 1023;        // Valor máximo (2^10 - 1)

// --- Sensores ---
const float VREF = 3.3;                // Voltaje de referencia del ADC
const float ADC_RESOLUTION = 4095.0;   // 12 bits (0-4095)
const float ACS712_SENSITIVITY = 0.185; // 185mV/A para el modelo de 5A

// --- RPM Máximas para el POTENCIÓMETRO ---
#define MAX_RPM 126.0 // RPM máximas del motor con reductor 34:1

// --- Configuración del Divisor de Voltaje ---
const bool VOLTAGE_SENSOR_ENABLED = false; // Habilitar/Dehabilitar sensor de voltaje

// Si mides 12V, un divisor con R1=30k y R2=7.5k es ideal.
// Vout = Vin * (R2 / (R1 + R2)) -> 12V * (7.5 / 37.5) = 2.4V (Seguro para 3.3V)
// El factor de multiplicación es (R1 + R2) / R2
const float VOLTAGE_DIVIDER_FACTOR = (30000.0 + 7500.0) / 7500.0; // = 5.0

// ===================================
// ==== CONFIGURACIÓN DEL ENCODER ====
// ===================================
// Este es el número de pulsos (contando ambos bordes de A y B)
// que el encoder genera por CADA REVOLUCIÓN del eje de salida.
const int ENCODER_PPR = 374; // Para un reductor 34:1 y encoder de 11 pulsos

// --- Control ---
const int POT_MID_POINT = 2048; // Punto central aprox. del ADC (4096 / 2)
const int POT_DEADZONE  = 100;  // Zona muerta (ej. 2048 +/- 100) para evitar ruido