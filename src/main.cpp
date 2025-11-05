/*
 *  src/main.cpp
 *  Prueba de Aislamiento para el Potenciómetro
 */

#include <Arduino.h>
#include <config.h>       // Para MAX_RPM (y PIN_POTENCIOMETRO)
#include <potenciometro.h>  // Nuestro nuevo módulo

void setup() {
    Serial.begin(115200);
    delay(1000); // Un segundo para que el monitor serial se conecte
    
    Serial.println("--- INICIANDO PRUEBA DE POTENCIÓMETRO ---");
    
    // Inicializa el módulo del potenciómetro
    // (Esto leerá el pin definido en config.h como PIN_POTENCIOMETRO)
    pot_init(); 
    
    Serial.println("Potenciómetro inicializado. Gira el dial.");
}

void loop() {
    // 1. Lee el valor crudo (útil para depurar, 0-4095)
    int valor_crudo = pot_get_raw_value();

    // 2. Lee el valor en RPM (ya filtrado y mapeado)
    float rpm_objetivo = pot_get_target_rpm();
    
    // 3. Imprime los valores en el Monitor Serial
    Serial.printf("Valor Crudo: %d \t|  RPM Objetivo: %.2f RPM\n", valor_crudo, rpm_objetivo);

    delay(250); // Imprime 4 veces por segundo
}