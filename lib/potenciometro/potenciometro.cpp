#include "potenciometro.h"
#include <config.h>
#include <Arduino.h>

// --- Definiciones (se quedan igual) ---
#define NUM_LECTURAS 10 
static int lecturas[NUM_LECTURAS];
static int indice_lectura = 0;
static long total_lecturas = 0;
#define ADC_MAX_RAW 4095.0

// --- pot_init() y pot_get_raw_value() se quedan igual ---

void pot_init() {
    pinMode(PIN_POTENCIOMETRO, INPUT);
    int primera_lectura = analogRead(PIN_POTENCIOMETRO);
    for (int i = 0; i < NUM_LECTURAS; i++) {
        lecturas[i] = primera_lectura;
    }
    total_lecturas = (long)primera_lectura * NUM_LECTURAS;
    indice_lectura = 0;
}

int pot_get_raw_value() {
    return analogRead(PIN_POTENCIOMETRO);
}

// --- ¡NUEVA LÓGICA! ---

float pot_get_filtered_value() {
    // 1. Restar la lectura más antigua
    total_lecturas = total_lecturas - lecturas[indice_lectura];

    // 2. Leer y guardar la nueva lectura
    lecturas[indice_lectura] = analogRead(PIN_POTENCIOMETRO);

    // 3. Sumar la nueva lectura al total
    total_lecturas = total_lecturas + lecturas[indice_lectura];

    // 4. Avanzar el índice
    indice_lectura++;
    if (indice_lectura >= NUM_LECTURAS) {
        indice_lectura = 0;
    }

    // 5. Calcular el promedio
    return (float)total_lecturas / (float)NUM_LECTURAS;
}

float pot_get_target_rpm() {
    // Esta función ahora solo mapea el valor filtrado
    float valor_promedio = pot_get_filtered_value();
    
    // Mapea el valor filtrado (0 a 4095) al rango de RPM (0 a MAX_RPM)
    float rpm_objetivo = (valor_promedio / ADC_MAX_RAW) * MAX_RPM;

    return rpm_objetivo;
}