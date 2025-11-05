#include "motor.h"
#include "config.h" // Incluimos nuestra configuración de pines

/**
 * @brief Inicializa el motor.
 */
void motor_init() {
    // 1. Configurar pines de dirección y standby como SALIDA
    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);
    pinMode(PIN_MOTOR_STBY, OUTPUT);

    // 2. Configurar el canal PWM (LEDC)
    // Se usa la configuración de config.h
    ledcSetup(PWM_CHANNEL_MOTOR, PWM_FREQ, PWM_RES_BITS);

    // 3. Asociar el pin PWM al canal configurado
    ledcAttachPin(PIN_MOTOR_PWM, PWM_CHANNEL_MOTOR);

    // 4. Asegurar que el motor comience detenido
    motor_brake();
}

/**
 * @brief Controla el motor.
 */
void motor_drive(int duty_cycle, bool direction) {
    // 1. Asegurar que el driver esté activo
    digitalWrite(PIN_MOTOR_STBY, HIGH);

    // 2. Establecer la dirección
    if (direction == true) {
        // Dirección "Adelante" (arbitraria)
        digitalWrite(PIN_MOTOR_IN1, HIGH);
        digitalWrite(PIN_MOTOR_IN2, LOW);
    } else {
        // Dirección "Atrás"
        digitalWrite(PIN_MOTOR_IN1, LOW);
        digitalWrite(PIN_MOTOR_IN2, HIGH);
    }

    // 3. Establecer la velocidad (potencia)
    ledcWrite(PWM_CHANNEL_MOTOR, duty_cycle);
}

/**
 * @brief Frena el motor.
 */
void motor_brake() {
    // 1. Poner pines de dirección en BAJO
    // (Esto causa un "short brake" en el TB6612, frenando activamente)
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    
    // 2. Poner la velocidad a cero
    ledcWrite(PWM_CHANNEL_MOTOR, 0);

    // 3. Mantener el driver activo (listo para el próximo comando)
    digitalWrite(PIN_MOTOR_STBY, HIGH);
}