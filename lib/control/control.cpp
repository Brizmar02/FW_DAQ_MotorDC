#include "control.h"
#include <config.h>
#include <Arduino.h>

// --- INCLUIR TUS LIBRERÍAS DE HARDWARE ---
#include <potenciometro.h>
#include <motor.h>
#include <encoder.h>
#include <sensores.h>

// Definición del ADC_MAX_RAW
#define ADC_MAX_RAW 4095.0

// --- DEFINICIÓN DE LA ESTRUCTURA DE DATOS (PACKET) ---
struct TelemetryPacket {
    uint8_t header[2];      
    uint32_t time_ms;       
    float setpoint_rpm;     
    float real_rpm;         
    float current_A;
    float voltage_V;        
    float pwm_val;          
    uint8_t terminator[2];  
} __attribute__((packed));

TelemetryPacket dataPacket;

// Variables para el timing
unsigned long last_print_time = 0;
const int PRINT_INTERVAL_MS = 20; 

// --- VARIABLES DE ESTADO DEL SISTEMA (NUEVAS) ---
static bool is_auto_mode = false;   // false = Manual (Pot), true = Auto (GUI)
static float gui_pwm_target = 0.0;  // Valor recibido desde la GUI

void control_init() {
    Serial.begin(9600);
    delay(1000);
    
    // Inicializar la cabecera
    dataPacket.header[0] = 0xA5; 
    dataPacket.header[1] = 0x5A; 
    dataPacket.terminator[0] = 0x0D; 
    dataPacket.terminator[1] = 0x0A;

    // Deshabilitar control Manual (Potenciómetro) al inicio
    motor_move(0);
    gui_pwm_target = 0.0;
    is_auto_mode = true; // Arranca en Auto para ignorar el pin flotante del pot

    // Inicializar Hardware
    sensores_init(); 
    pot_init();
    motor_init();
    encoder_init(); 
}

// Helper mapf
static float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void control_loop() {
    // ---------------------------------------------------------
    // 0. RECEPCIÓN LIGERA (9600 Baud)
    // ---------------------------------------------------------
    if (Serial.available() >= 7) { 
        if (Serial.peek() == 64) { // ¿Es '@'?
            Serial.read(); // Consumir '@'
            
            char cmd = (char)Serial.read(); 
            float valorRecibido = 0.0;
            Serial.readBytes((uint8_t*)&valorRecibido, 4);
            uint8_t footer = Serial.read(); 
            
            if (footer == 10) { // Trama Válida
                if (!isnan(valorRecibido) && !isinf(valorRecibido) && abs(valorRecibido) <= 105.0) {
                    if (cmd == 'M') {
                        bool nuevoModo = (valorRecibido > 0.5);
                        // Resetear target solo si CAMBIAMOS de modo, no siempre
                        if (nuevoModo != is_auto_mode) gui_pwm_target = 0; 
                        is_auto_mode = nuevoModo;
                    }
                    if (cmd == 'P') {
                        // Clamp del valor recibido
                        if (valorRecibido > 100.0) valorRecibido = 100.0;
                        if (valorRecibido < -100.0) valorRecibido = -100.0;
                        gui_pwm_target = valorRecibido;
                    }
                }
            }
        } else {
            Serial.read(); 
        }
    }

    // --- 1. LECTURA DE SENSORES ---
    float adc_pot = pot_get_filtered_value();
    float rpm_raw = encoder_get_velocity_rpm();
    float corriente_A = sensor_get_corriente_A();
    float voltaje_V = sensor_get_voltaje_V();

    // --- CORRECCIÓN DE SIGNO ---
    float rpm_real = rpm_raw * -1.0;

    // --- 2. LÓGICA DE CONTROL ---
    float porcentaje_motor = 0.0;
    float rpm_setpoint = 0.0; 

    // FORZADO DE SEGURIDAD (Opcional):
    // Si quitaste el potenciómetro físico, descomenta la siguiente línea para que SIEMPRE sea auto
    // is_auto_mode = true; 

    if (is_auto_mode) {
        // --- MODO AUTOMÁTICO ---
        porcentaje_motor = gui_pwm_target;
        rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM;
    } else {
        // --- MODO MANUAL (SI EL POTENCIÓMETRO EXISTIERA) ---
        // Si no hay pot, esto leerá ruido, pero si is_auto_mode es true, nunca entrará aquí.
        if (adc_pot > (POT_MID_POINT + POT_DEADZONE)) {
            porcentaje_motor = mapf(adc_pot, POT_MID_POINT + POT_DEADZONE, ADC_MAX_RAW, 0.0, 100.0);
        } else if (adc_pot < (POT_MID_POINT - POT_DEADZONE)) {
            porcentaje_motor = mapf(adc_pot, 0.0, POT_MID_POINT - POT_DEADZONE, -100.0, 0.0);
        }
        rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM; 
    }
    
    // Clamp final
    if (porcentaje_motor > 100.0) porcentaje_motor = 100.0;
    if (porcentaje_motor < -100.0) porcentaje_motor = -100.0;

    // --- 3. ACTUACIÓN (CORREGIDO) ---
    // AQUÍ ES DONDE OCURRE LA MAGIA. 
    // Le mandamos al motor lo que calculamos arriba.
    motor_move(porcentaje_motor); 

    // --- 4. TELEMETRÍA ---
    unsigned long current_time = millis();
    if (current_time - last_print_time >= PRINT_INTERVAL_MS) {
        last_print_time = current_time;
        
        dataPacket.time_ms = current_time;
        dataPacket.setpoint_rpm = rpm_setpoint;
        dataPacket.real_rpm = rpm_real;
        dataPacket.current_A = corriente_A;
        dataPacket.voltage_V = voltaje_V;
        dataPacket.pwm_val = porcentaje_motor;

        Serial.write((uint8_t*)&dataPacket, sizeof(dataPacket));
    }
}