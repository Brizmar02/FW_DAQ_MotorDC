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
// __attribute__((packed)) es CRUCIAL. Obliga al compilador a no dejar espacios vacíos
// entre variables, asegurando que el tamaño en bytes sea exacto.
struct TelemetryPacket {
    uint8_t header[2];      // 2 bytes de cabecera para sincronización (0xA5, 0x5A)
    uint32_t time_ms;       // 4 bytes: Tiempo
    float setpoint_rpm;     // 4 bytes: Setpoint
    float real_rpm;         // 4 bytes: Velocidad Real
    float current_A;        // 4 bytes: Corriente
    float pwm_val;          // 4 bytes: PWM
    uint8_t terminator[2];  // 2 bytes de final (0x0D, 0x0A - CR/LF)
} __attribute__((packed));

TelemetryPacket dataPacket;

// Variables para el timing
unsigned long last_print_time = 0;
// NOTA: Con binario podemos bajar este tiempo. 20ms (50Hz) es excelente para gráficas fluidas.
const int PRINT_INTERVAL_MS = 20; 

void control_init() {
    Serial.begin(115200);
    delay(1000);
    
    // Inicializar la cabecera y el final una sola vez
    dataPacket.header[0] = 0xA5; // Byte mágico 1
    dataPacket.header[1] = 0x5A; // Byte mágico 2
    dataPacket.terminator[0] = 0x0D; 
    dataPacket.terminator[1] = 0x0A;

    // Inicializar Hardware
    sensores_init(); 
    pot_init();
    motor_init();
    encoder_init(); 
    
    // Ya no imprimimos texto de bienvenida porque ensuciaría la trama binaria
    // Si necesitas depurar, usa el LED del ESP32 parpadeando
}

// Helper mapf (se mantiene igual)
static float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void control_loop() {
    // --- 1. LECTURA DE ENTRADAS ---
    float adc_pot = pot_get_filtered_value();
    float rpm_real = encoder_get_velocity_rpm();
    float corriente_A = sensor_get_corriente_A();

    // --- 2. LÓGICA DE CONTROL ---
    float porcentaje_motor = 0.0;
    float rpm_setpoint = 0.0; 

    if (adc_pot > (POT_MID_POINT + POT_DEADZONE)) {
        porcentaje_motor = mapf(adc_pot, POT_MID_POINT + POT_DEADZONE, ADC_MAX_RAW, 0.0, 100.0);
        rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM;
    } else if (adc_pot < (POT_MID_POINT - POT_DEADZONE)) {
        porcentaje_motor = mapf(adc_pot, 0.0, POT_MID_POINT - POT_DEADZONE, -100.0, 0.0);
        rpm_setpoint = (porcentaje_motor / 100.0) * MAX_RPM; 
    }
    
    if (porcentaje_motor > 100.0) porcentaje_motor = 100.0;
    if (porcentaje_motor < -100.0) porcentaje_motor = -100.0;

    // --- 3. ACTUACIÓN ---
    motor_move(porcentaje_motor);

    // --- 4. TELEMETRÍA BINARIA ---
    unsigned long current_time = millis();
    
    if (current_time - last_print_time >= PRINT_INTERVAL_MS) {
        last_print_time = current_time;
        
        // Llenamos la estructura con los datos actuales
        dataPacket.time_ms = current_time;
        dataPacket.setpoint_rpm = rpm_setpoint;
        dataPacket.real_rpm = rpm_real;
        dataPacket.current_A = corriente_A;
        dataPacket.pwm_val = porcentaje_motor;

        // ENVIAR PAQUETE
        // sizeof(dataPacket) debería ser: 2+4+4+4+4+4+2 = 24 bytes
        Serial.write((uint8_t*)&dataPacket, sizeof(dataPacket));
    }
}