#include "motor.h"      // La cabecera de nuestro módulo
#include <config.h>    // Para todos los pines y definiciones de PWM
#include <Arduino.h>     // Para pinMode, digitalWrite, ledc...

/**
 * @brief Inicializa el módulo del motor (TB6612FNG).
 */
void motor_init() {
    // 1. Configurar pines de dirección y standby como SALIDA
    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);
    pinMode(PIN_MOTOR_STBY, OUTPUT);

    // 2. Configurar el canal PWM (LEDC)
    // Se usa la configuración de config.h
    ledcSetup(PWM_MOTOR_CHANNEL, PWM_MOTOR_FREQ, PWM_MOTOR_RES_BITS);

    // 3. Asociar el pin PWM al canal configurado
    ledcAttachPin(PIN_MOTOR_PWM, PWM_MOTOR_CHANNEL);

    // 4. Asegurar que el motor comience frenado y el driver activo
    motor_stop();
}

/**
 * @brief Controla la velocidad y dirección del motor.
 */
void motor_move(float porcentaje) {
    // 1. Activar el driver (STBY = HIGH)
    // Esto "despierta" el chip TB6612, listo para recibir comandos
    digitalWrite(PIN_MOTOR_STBY, HIGH);

    // 2. Limitar el porcentaje de entrada al rango válido [-100.0, 100.0]
    if (porcentaje > 100.0) {
        porcentaje = 100.0;
    } else if (porcentaje < -100.0) {
        porcentaje = -100.0;
    }

    // 3. Determinar la dirección y establecer los pines IN
    if (porcentaje > 0.0) {
        // Dirección "Adelante" (Horario)
        digitalWrite(PIN_MOTOR_IN1, HIGH);
        digitalWrite(PIN_MOTOR_IN2, LOW);
    } else if (porcentaje < 0.0) {
        // Dirección "Atrás" (Antihorario)
        digitalWrite(PIN_MOTOR_IN1, LOW);
        digitalWrite(PIN_MOTOR_IN2, HIGH);
    } else {
        // El porcentaje es 0, frenar el motor
        motor_stop();
        return; // Salir de la función
    }

    // 4. Calcular el Duty Cycle (potencia)
    // abs() obtiene el valor absoluto (ej. -50.0 -> 50.0)
    // Dividimos por 100 para obtener un factor (ej. 50.0 -> 0.5)
    float duty_factor = abs(porcentaje) / 100.0;
    
    // Multiplicamos el factor por el valor máximo de nuestro PWM
    // (ej. 0.5 * 1023 = 511)
    int duty_cycle = (int)(duty_factor * PWM_MOTOR_MAX_DUTY);

    // 5. Enviar la velocidad al pin PWM
    ledcWrite(PWM_MOTOR_CHANNEL, duty_cycle);
}

/**
 * @brief Detiene el motor inmediatamente (freno).
 */
void motor_stop() {
    // 1. Poner ambos pines de dirección en BAJO
    // En el TB6612, esto provoca un "short brake" (freno activo)
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    
    // 2. Poner la velocidad a cero
    ledcWrite(PWM_MOTOR_CHANNEL, 0);

    // 3. Mantener el driver activo (listo para el próximo comando)
    digitalWrite(PIN_MOTOR_STBY, HIGH);
}