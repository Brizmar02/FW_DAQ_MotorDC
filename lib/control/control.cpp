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
    Serial.begin(115200);
    delay(1000);
    
    // Inicializar la cabecera
    dataPacket.header[0] = 0xA5; 
    dataPacket.header[1] = 0x5A; 
    dataPacket.terminator[0] = 0x0D; 
    dataPacket.terminator[1] = 0x0A;

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
    // 0. RECEPCIÓN DE COMANDOS DESDE MATLAB (Slider/Switch)
    // ---------------------------------------------------------
    // Protocolo: [@][Cmd][Byte1][Byte2][Byte3][Byte4][\n] (7 Bytes)
    if (Serial.available() >= 7) { 
        // "Peek" para ver si es el Header '@' (64)
        if (Serial.peek() == 64) {
            Serial.read(); // Consumir '@'
            
            char cmd = (char)Serial.read(); // Leer comando ('M' o 'P')
            
            // Leer los 4 bytes del float
            float valorRecibido = 0.0;
            Serial.readBytes((uint8_t*)&valorRecibido, 4);
            
            uint8_t footer = Serial.read(); // Leer terminador '\n' (10)
            
            if (footer == 10) { // Integridad OK
                
                // CASO 'M': Cambio de MODO (Auto/Manual)
                if (cmd == 'M') {
                    // MATLAB envía 1.0 para Auto, 0.0 para Manual
                    if (valorRecibido > 0.5) {
                        is_auto_mode = true;
                        gui_pwm_target = 0; // Seguridad: Al entrar a auto, empezar en 0
                    } else {
                        is_auto_mode = false;
                    }
                }
                
                // CASO 'P': Comando de PWM (Solo efectivo si is_auto_mode = true)
                if (cmd == 'P') {
                    // Actualizamos la variable objetivo
                    if (valorRecibido > 100.0) valorRecibido = 100.0;
                    if (valorRecibido < -100.0) valorRecibido = -100.0;
                    gui_pwm_target = valorRecibido;
                }
            }
        } else {
            // Limpieza de buffer si hay basura (byte no es '@')
            Serial.read(); 
        }
    }

    // --- 1. LECTURA DE SENSORES ---
    float adc_pot = pot_get_filtered_value();
    float rpm_real = encoder_get_velocity_rpm();
    float corriente_A = sensor_get_corriente_A();

    // --- 2. LÓGICA DE CONTROL (SELECTOR DE MODO) ---
    float porcentaje_motor = 0.0;
    float rpm_setpoint = 0.0; 

    if (is_auto_mode) {
        // --- MODO AUTOMÁTICO (GUI) ---
        // Usamos el valor que llegó por Serial
        porcentaje_motor = gui_pwm_target;
        
        // Calculamos el setpoint teórico de RPM solo para graficarlo
        rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM;

    } else {
        // --- MODO MANUAL (POTENCIÓMETRO) ---
        // Usamos la lógica original
        if (adc_pot > (POT_MID_POINT + POT_DEADZONE)) {
            porcentaje_motor = mapf(adc_pot, POT_MID_POINT + POT_DEADZONE, ADC_MAX_RAW, 0.0, 100.0);
            rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM;
        } else if (adc_pot < (POT_MID_POINT - POT_DEADZONE)) {
            porcentaje_motor = mapf(adc_pot, 0.0, POT_MID_POINT - POT_DEADZONE, -100.0, 0.0);
            rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM; 
        }
    }
    
    // Clamp final de seguridad
    if (porcentaje_motor > 100.0) porcentaje_motor = 100.0;
    if (porcentaje_motor < -100.0) porcentaje_motor = -100.0;

    // --- 3. ACTUACIÓN ---
    motor_move(porcentaje_motor);

    // --- 4. TELEMETRÍA BINARIA ---
    unsigned long current_time = millis();
    
    if (current_time - last_print_time >= PRINT_INTERVAL_MS) {
        last_print_time = current_time;
        
        dataPacket.time_ms = current_time;
        dataPacket.setpoint_rpm = rpm_setpoint;
        dataPacket.real_rpm = rpm_real;
        dataPacket.current_A = corriente_A;
        dataPacket.pwm_val = porcentaje_motor;

        Serial.write((uint8_t*)&dataPacket, sizeof(dataPacket));
    }
}