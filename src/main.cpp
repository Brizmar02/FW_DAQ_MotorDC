/*
 * src/main.cpp
 * Prueba de DEPURACIÓN del Sensor de Corriente
 */

#include <Arduino.h>
#include <config.h>
#include <motor.h>
#include <sensores.h> // Nuestro módulo de sensores

// Esta variable global (extern) nos dejará "espiar" el valor
// que está guardado dentro de sensores.cpp
//
// ATENCIÓN: Esto es una mala práctica en código final,
// pero es una herramienta de depuración EXCELENTE.
extern float g_adc_offset_corriente;


void setup() {
    Serial.begin(115200);
    delay(2000); // Espera 2 segundos
    
    Serial.println("--- PRUEBA DE DEPURACIÓN DE CALIBRACIÓN ---");
    Serial.println("Motor PARADO.");

    // 1. Lectura manual ANTES de la calibración
    pinMode(PIN_SENSOR_CORRIENTE, INPUT);
    int lectura_manual = analogRead(PIN_SENSOR_CORRIENTE);
    Serial.printf("Lectura manual en Setup(): %d\n", lectura_manual);

    // 2. Inicializar sensores (esto debería calibrar)
    sensores_init();

    // 3. Imprimir el valor de offset que guardó la calibración
    Serial.printf("Offset guardado (g_adc_offset_corriente): %.2f\n", g_adc_offset_corriente);

    // 4. Inicializar motor
    motor_init();
    delay(1000);

    // 5. Encender el motor
    Serial.println("Encendiendo motor al 50%...");
    motor_move(100.0); 
}

void loop() {
    // 1. Leer el valor crudo del ADC
    int valor_crudo_loop = analogRead(PIN_SENSOR_CORRIENTE);

    // 2. Leer la corriente (cálculo del módulo)
    float corriente = sensor_get_corriente_A();
    
    // 3. Imprimir todo
    Serial.printf("Lectura Cruda: %d  |  Offset: %.2f  |  Corriente Calculada: %.3f A\n", 
                    valor_crudo_loop, g_adc_offset_corriente, corriente);

    delay(500); 
}