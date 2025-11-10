/*
 * src/main.cpp
 * Prueba de Lógica DIRECTA (bypass motor.cpp)
 */

#include <Arduino.h>
#include <config.h> // Incluimos los pines

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- PRUEBA DIRECTA DE PINES ---");

    // Configurar pines de dirección
    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);

    // Configurar el pin STBY
    pinMode(PIN_MOTOR_STBY, OUTPUT);
    digitalWrite(PIN_MOTOR_STBY, HIGH); // Encender el driver

    // Configurar PWM (la misma lógica de motor.cpp)
    ledcSetup(PWM_MOTOR_CHANNEL, PWM_MOTOR_FREQ, PWM_MOTOR_RES_BITS);
    ledcAttachPin(PIN_MOTOR_PWM, PWM_MOTOR_CHANNEL);

    Serial.println("Pines configurados. Iniciando secuencia...");
}

void loop() {
    
    Serial.println("PRUEBA: ADELANTE 50%");
    // Adelante (IN1=H, IN2=L)
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    // 50% de potencia
    ledcWrite(PWM_MOTOR_CHANNEL, PWM_MOTOR_MAX_DUTY / 2); 
    delay(3000);

    Serial.println("PRUEBA: ATRÁS 100%");
    // Atrás (IN1=L, IN2=H)
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, HIGH);
    // 100% de potencia
    ledcWrite(PWM_MOTOR_CHANNEL, PWM_MOTOR_MAX_DUTY);
    delay(3000);

    Serial.println("PRUEBA: FRENO");
    // Freno (IN1=L, IN2=L)
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    // 0% de potencia
    ledcWrite(PWM_MOTOR_CHANNEL, 0);
    delay(3000);
}