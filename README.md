# Software de Adquisición de Datos para Motor DC

## 📋 Descripción

Este repositorio contiene el **firmware embebido** para un sistema de adquisición de datos de un motor DC, desarrollado como parte del proyecto de tesis *"Diseño y construcción de una plataforma para la adquisición de datos de un motor DC"*.

El objetivo principal de este software es **recolectar datos experimentales** de un motorreductor JGA25-371 con encoder integrado, facilitando a estudiantes e investigadores el estudio y diseño de estrategias de control (PID, control adaptativo, etc.) sin necesidad de implementar desde cero la capa de hardware y sensado.

> **Nota:** Este proyecto se enfoca exclusivamente en la **adquisición de datos**. El diseño e implementación de algoritmos de control queda fuera del alcance de esta tesis, siendo responsabilidad del usuario final.

---

## 🧩 Parte del Proyecto de Tesis

Este repositorio corresponde al **componente de Software** del proyecto de tesis, el cual está dividido en cuatro partes complementarias:

| Componente | Descripción | Repositorio |
|------------|-------------|-------------|
| **1. Hardware** | Diseño de PCB en KiCad con driver de motor TB6612FNG, sensores de corriente (ACS712) y circuitos de acondicionamiento | *[Enlace al repo de Hardware]* |
| **2. Software** | Firmware para ESP32-S3 con PlatformIO (este repositorio) | **📍 Estás aquí** |
| **3. Interfaz** | Aplicación en MATLAB para visualización en tiempo real y análisis de datos | *[Enlace al repo de Interfaz]* |
| **4. Documentación** | Documento completo de la tesis en LaTeX | *[Enlace al repo de Documentación]* |

---

## ✨ Características Principales

El sistema de adquisición de datos implementa las siguientes funcionalidades:

### 📊 Variables Medidas en Tiempo Real

- **Velocidad del motor (RPM):** Lectura de encoder en cuadratura mediante periférico PCNT (Pulse Counter) del ESP32-S3
- **Corriente consumida (A):** Sensor ACS712-05A con filtrado RC y calibración automática en arranque
- **Voltaje de alimentación (V):** Divisor resistivo (opcional, configurable en `config.h`)
- **Posición del encoder (pulsos):** Contador de 64 bits con manejo de desborde automático
- **Setpoint de velocidad (RPM):** Control manual mediante potenciómetro con zona muerta configurable
- **Ciclo de trabajo del PWM (%):** Porcentaje de potencia aplicado al motor

### 🎛️ Control de Motor

- **Control bidireccional:** Rotación horaria y antihoraria mediante driver TB6612FNG
- **Zona muerta configurable:** Previene movimientos no deseados en reposo del potenciómetro
- **Frecuencia PWM ajustable:** 1.22 kHz por defecto (configurable en `config.h`)
- **Protección por software:** Límites de velocidad y saturación de señales

### 📡 Telemetría

- **Formato de salida:** CSV por puerto Serial a 115200 baud
- **Frecuencia de muestreo:** 10 Hz (configurable)
- **Campos exportados:**
  ```
  Tiempo(ms), Setpoint(RPM), Real(RPM), Corriente(A), PWM(%)
  ```

### 🔧 Arquitectura Modular

El código está organizado en bibliotecas independientes para facilitar el mantenimiento y la reutilización:

- `lib/motor` - Control del driver TB6612FNG con PWM
- `lib/encoder` - Lectura de encoder con PCNT
- `lib/potenciometro` - Filtrado de señal analógica con promedio móvil
- `lib/sensores` - Lectura de corriente (ACS712) y voltaje
- `lib/control` - Lógica principal de adquisición y telemetría

---

## 🛠️ Tecnologías Utilizadas

- **Plataforma:** [ESP32-S3-DevKitC-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html) (Espressif Systems)
- **Framework:** Arduino para ESP32 (v6.7.0)
- **Entorno de desarrollo:** [PlatformIO](https://platformio.org/) con Visual Studio Code
- **Lenguaje:** C++ (estándar Arduino)
- **Periféricos utilizados:**
  - PCNT (Pulse Counter) para encoder en cuadratura
  - LEDC (LED PWM Controller) para control de motor
  - ADC (Analog-Digital Converter) de 12 bits para sensores analógicos

---

## 🚀 Instalación y Configuración

### Requisitos Previos

1. **Visual Studio Code** instalado ([Descargar](https://code.visualstudio.com/))
2. **Extensión PlatformIO IDE** instalada en VS Code ([Guía de instalación](https://platformio.org/install/ide?install=vscode))
3. **Cable USB-C** para conectar el ESP32-S3
4. **Drivers CP210x** (si tu sistema operativo no los detecta automáticamente)

### Pasos de Instalación

1. **Clonar el repositorio:**

   ```bash
   git clone https://github.com/Brizmar02/SW_DAQ_Tesis_PIO.git
   cd SW_DAQ_Tesis_PIO
   ```

2. **Abrir el proyecto en PlatformIO:**

   - Abrir Visual Studio Code
   - `File` → `Open Folder...` → Seleccionar la carpeta `SW_DAQ_Tesis_PIO`
   - PlatformIO detectará automáticamente el proyecto

3. **Configurar pines y parámetros (opcional):**

   Editar el archivo `include/config.h` para ajustar:
   - Pines GPIO del hardware
   - Frecuencia y resolución del PWM
   - Parámetros del encoder (PPR)
   - Zona muerta del potenciómetro
   - Activar/desactivar sensor de voltaje

4. **Compilar el proyecto:**

   - Clic en el ícono ✓ (Build) en la barra inferior de VS Code
   - O presionar `Ctrl+Alt+B`

5. **Flashear el ESP32-S3:**

   - Conectar el ESP32-S3 al puerto USB
   - Clic en el ícono → (Upload) en la barra inferior
   - O presionar `Ctrl+Alt+U`

6. **Verificar funcionamiento:**

   - Abrir el Monitor Serial: ícono 🔌 (Serial Monitor) o `Ctrl+Alt+S`
   - Velocidad: 115200 baud
   - Deberías ver la salida CSV con los datos en tiempo real

### Ejemplo de Salida Esperada

```
--- INICIANDO SISTEMA DE RECOLECCION DE DATOS ---
Formato CSV: Tiempo(ms), Setpoint(RPM), Real(RPM), Corriente(A), PWM(%)
Calibrando sensores...
Sistema listo. Inicio de loop.
1234, 0.00, 0.00, 0.000, 0.0
1334, 45.20, 43.78, 0.842, 35.8
1434, 45.20, 44.91, 0.856, 35.8
1534, 90.50, 87.23, 1.524, 71.7
```

---

## 📖 Uso

### Flujo de Operación

1. **Arranque del sistema:**
   - Al energizar el ESP32-S3, se ejecuta la calibración automática del sensor de corriente (tarda ~1 segundo)
   - El motor debe estar **detenido** durante esta calibración inicial

2. **Control manual:**
   - Girar el potenciómetro conectado al pin GPIO 4 para establecer el setpoint de velocidad
   - Zona central (±100 unidades ADC alrededor de 2048): motor detenido (zona muerta)
   - Girar hacia un extremo: rotación horaria (0 a +100% PWM)
   - Girar hacia el otro extremo: rotación antihoraria (0 a -100% PWM)

3. **Adquisición de datos:**
   - El sistema envía datos por el puerto Serial USB en formato CSV a 10 Hz
   - Los datos pueden ser leídos por:
     - Monitor Serial de PlatformIO (para depuración)
     - **Interfaz de MATLAB** del proyecto (para análisis y visualización)
     - Cualquier terminal serial (PuTTY, screen, minicom, etc.)

4. **Integración con MATLAB:**
   - Los datos CSV son compatibles con la interfaz gráfica de MATLAB del proyecto
   - La interfaz permite graficar en tiempo real y exportar datos para análisis offline

### Modificación de Variables Sensadas

Para añadir nuevas variables a la telemetría (ej. temperatura, aceleración), modificar:

1. `lib/control/control.cpp` - Función `control_loop()`
2. Añadir la lectura del nuevo sensor
3. Actualizar el `Serial.printf()` con el nuevo campo CSV

**Ejemplo:**

```cpp
// En control_loop(), después de leer corriente_A:
float temperatura = sensor_get_temperatura(); // Nueva función

// Actualizar la impresión:
Serial.printf("%lu, %.2f, %.2f, %.3f, %.1f, %.1f\n", 
              current_time, 
              rpm_setpoint, 
              rpm_real, 
              corriente_A, 
              porcentaje_motor,
              temperatura); // Nuevo campo
```

---

## 📁 Estructura del Proyecto

```
SW_DAQ_Tesis_PIO/
├── include/
│   └── config.h              # Configuración de pines y constantes
├── lib/
│   ├── control/              # Lógica principal de adquisición
│   │   ├── control.h
│   │   └── control.cpp
│   ├── encoder/              # Lectura de encoder con PCNT
│   │   ├── encoder.h
│   │   └── encoder.cpp
│   ├── motor/                # Control del driver TB6612FNG
│   │   ├── motor.h
│   │   └── motor.cpp
│   ├── potenciometro/        # Filtrado de potenciómetro
│   │   ├── potenciometro.h
│   │   └── potenciometro.cpp
│   └── sensores/             # Sensores de corriente y voltaje
│       ├── sensores.h
│       └── sensores.cpp
├── src/
│   └── main.cpp              # Punto de entrada (setup/loop)
├── platformio.ini            # Configuración de PlatformIO
└── README.md                 # Este archivo
```

---

## 🤝 Contribución

Este proyecto es parte de una tesis de grado. Las contribuciones son bienvenidas para:

- Reportar bugs o problemas de hardware
- Mejorar la documentación
- Añadir compatibilidad con otros motores o sensores
- Optimizar algoritmos de filtrado

### Cómo Contribuir

1. Fork este repositorio
2. Crear una rama para tu feature (`git checkout -b feature/nueva-funcionalidad`)
3. Commit de tus cambios (`git commit -m 'feat: añade nueva funcionalidad'`)
4. Push a la rama (`git push origin feature/nueva-funcionalidad`)
5. Abrir un Pull Request

### Reportar Issues

Si encuentras algún problema:

1. Verificar que los pines en `config.h` coincidan con tu hardware
2. Revisar que el ESP32-S3 esté correctamente flasheado (versión del bootloader)
3. Abrir un issue en GitHub con:
   - Descripción del problema
   - Salida del Monitor Serial
   - Configuración de hardware utilizada

---

## 📄 Licencia

Este proyecto fue desarrollado como parte de una tesis de pregrado en Ingeniería Mecatrónica.

**Autor:** David Brizuela Martínez
**Institución:** Centro de Enseñanza Técnica Industrial 
**Año:** 2025

---

## 🙏 Agradecimientos

- A los profesores y asesor que guiaron y motivaron en este proyecto
- A la comunidad de PlatformIO y ESP32
- A los desarrolladores de las librerías de Arduino para Espressif

---

## 📞 Contacto

Para consultas sobre el proyecto de tesis o colaboraciones académicas:

- **Email:** [brizuelad532@gmail.com]
- **GitHub:** [@Brizmar02](https://github.com/Brizmar02)