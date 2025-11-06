/*
 * src/main.cpp
 * Prueba de Aislamiento para el Módulo Motor
 */

#include <Arduino.h>
#include <config.h>  // Para las definiciones del motor
#include <motor.h>   // Nuestro nuevo módulo de motor

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("--- INICIANDO PRUEBA DE MOTOR ---");
    
    // Inicializa el módulo del motor (configura pines y PWM)
    motor_init();
    
    Serial.println("Motor inicializado. Iniciando secuencia de prueba...");
}

void loop() {
    // --- SECUENCIA DE PRUEBA ---

    // 1. Mover "Adelante" (Horario) al 50% de potencia
    Serial.println("Moviendo ADELANTE al 50%");
    motor_move(50.0);
    delay(3000); // Durante 3 segundos

    // 2. Frenar
    Serial.println("FRENANDO");
    motor_stop(); // O motor_move(0.0)
    delay(2000); // Durante 2 segundos

    // 3. Mover "Atrás" (Antihorario) al 75% de potencia
    Serial.println("Moviendo ATRÁS al 75%");
    motor_move(-75.0);
    delay(3000); // Durante 3 segundos

    // 4. Frenar
    Serial.println("FRENANDO");
    motor_stop();
    delay(2000); // Durante 2 segundos
}