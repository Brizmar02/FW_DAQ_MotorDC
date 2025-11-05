#include "potenciometro.h" // Cabecera de nuestro módulo
#include <config.h>       // Para PIN_POTENCIOMETRO y MAX_RPM
#include <Arduino.h>      // Para analogRead, pinMode

// --- Variables para el filtro de promedio móvil ---
// Un promedio de 10 lecturas da una buena suavidad sin mucho retraso.
// Puedes aumentar este valor si la lectura es muy ruidosa.
#define NUM_LECTURAS 10 
static int lecturas[NUM_LECTURAS]; // Array para guardar las últimas lecturas
static int indice_lectura = 0;   // Índice actual en el array
static long total_lecturas = 0;   // Suma de las lecturas

// El ADC del ESP32-S3 es (por defecto) de 12 bits.
// 2^12 = 4096. El rango es 0-4095.
// Usamos .0 para forzar cálculos en punto flotante.
#define ADC_MAX_RAW 4095.0

// --- Implementación de Funciones ---

void pot_init() {
    // Configura el pin del potenciómetro como entrada
    pinMode(PIN_POTENCIOMETRO, INPUT);

    // Opcional: En el ESP32, puedes configurar la atenuación
    // y la resolución, pero para Arduino esto suele ser automático.
    // Dejamos la configuración por defecto de Arduino (12 bits, atenuación 11dB).
    
    // Inicializa el buffer del filtro.
    // Llenamos el buffer con la primera lectura para que el promedio
    // sea válido desde el primer momento, en lugar de empezar en 0.
    int primera_lectura = analogRead(PIN_POTENCIOMETRO);
    for (int i = 0; i < NUM_LECTURAS; i++) {
        lecturas[i] = primera_lectura;
    }
    total_lecturas = (long)primera_lectura * NUM_LECTURAS;
    indice_lectura = 0;
}

int pot_get_raw_value() {
    // Simplemente lee y devuelve el valor analógico crudo
    return analogRead(PIN_POTENCIOMETRO);
}

float pot_get_target_rpm() {
    // --- Algoritmo de Filtro de Promedio Móvil ---
    
    // 1. Restar la lectura más antigua del total
    total_lecturas = total_lecturas - lecturas[indice_lectura];

    // 2. Leer el nuevo valor del ADC
    int nueva_lectura = analogRead(PIN_POTENCIOMETRO);

    // 3. Guardar la nueva lectura en el array
    lecturas[indice_lectura] = nueva_lectura;

    // 4. Sumar la nueva lectura al total
    total_lecturas = total_lecturas + nueva_lectura;

    // 5. Avanzar el índice (y darle la vuelta si llega al final)
    indice_lectura++; // Avanza al siguiente slot
    if (indice_lectura >= NUM_LECTURAS) {
        indice_lectura = 0; // Se reinicia al llegar al final
    }

    // 6. Calcular el promedio
    // Usamos float para la división para no perder decimales
    float valor_promedio = (float)total_lecturas / (float)NUM_LECTURAS;

    // --- Mapeo a RPM ---
    // Mapea el valor filtrado (0 a 4095) al rango de RPM (0 a MAX_RPM)
    
    // (valor_promedio / ADC_MAX_RAW) -> nos da un porcentaje (0.0 a 1.0)
    // Multiplicamos ese porcentaje por las RPM máximas.
    float rpm_objetivo = (valor_promedio / ADC_MAX_RAW) * MAX_RPM;

    return rpm_objetivo;
}