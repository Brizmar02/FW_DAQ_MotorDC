# BITÁCORA / HANDOFF TÉCNICO — Migración de desarrollo a otro chat

## 0) Objetivo de este documento
Dejar un **handoff completo** para continuar el desarrollo en otro chat con mejor panorama del objetivo final. Incluye:
- Estado real del firmware y arquitectura
- Qué tipo de comunicación USB existe hoy
- Qué falta para “cargar/transferir archivos” por USB
- Decisiones pendientes (CDC vs MSC/MTP)
- Plan de trabajo sugerido
- Snippets de código (firmware + host) para entender la telemetría

---

## 1) Resumen ejecutivo (qué es este repo)
Firmware embebido para **ESP32-S3** (PlatformIO + Arduino) orientado a **adquisición de datos** y control básico de un motor DC con:
- Encoder en cuadratura vía PCNT
- Driver TB6612FNG vía PWM LEDC
- Potenciómetro (ADC) para setpoint manual
- Sensor de corriente ACS712 (ADC) y voltaje opcional

Punto de entrada: [src/main.cpp](src/main.cpp) que delega todo a:
- [`control_init`](lib/control/control.cpp) y [`control_loop`](lib/control/control.cpp) en [lib/control/control.cpp](lib/control/control.cpp) / [lib/control/control.h](lib/control/control.h)

---

## 2) Estado actual del firmware (lo que hace hoy)

### 2.1 Módulos y responsabilidades
- Control principal + telemetría: [`control_init`](lib/control/control.cpp), [`control_loop`](lib/control/control.cpp) en [lib/control/control.cpp](lib/control/control.cpp)
- Encoder PCNT: [`encoder_init`](lib/encoder/encoder.cpp), [`encoder_get_position`](lib/encoder/encoder.cpp), [`encoder_get_velocity_rpm`](lib/encoder/encoder.cpp) en [lib/encoder/encoder.cpp](lib/encoder/encoder.cpp) / [lib/encoder/encoder.h](lib/encoder/encoder.h)
- Motor TB6612: [`motor_init`](lib/motor/motor.cpp), [`motor_move`](lib/motor/motor.cpp), [`motor_stop`](lib/motor/motor.cpp) en [lib/motor/motor.cpp](lib/motor/motor.cpp) / [lib/motor/motor.h](lib/motor/motor.h)
- Potenciómetro: [`pot_init`](lib/potenciometro/potenciometro.cpp), [`pot_get_filtered_value`](lib/potenciometro/potenciometro.cpp), [`pot_get_target_rpm`](lib/potenciometro/potenciometro.cpp) en [lib/potenciometro/potenciometro.cpp](lib/potenciometro/potenciometro.cpp) / [lib/potenciometro/potenciometro.h](lib/potenciometro/potenciometro.h)
- Sensores: [`sensores_init`](lib/sensores/sensores.cpp), [`sensor_get_corriente_A`](lib/sensores/sensores.cpp), [`sensor_get_voltaje_V`](lib/sensores/sensores.cpp) en [lib/sensores/sensores.cpp](lib/sensores/sensores.cpp) / [lib/sensores/sensores.h](lib/sensores/sensores.h)

Configuración hardware y constantes: [include/config.h](include/config.h)

### 2.2 Telemetría actual (importante)
En el firmware actual, la salida por `Serial` NO es texto/CSV: es **binaria**.

- En [`control_init`](lib/control/control.cpp) se configura `Serial.begin(115200)` y se inicializan bytes mágicos y terminador.
- En [`control_loop`](lib/control/control.cpp) se arma un `TelemetryPacket` y se envía con:
  - `Serial.write((uint8_t*)&dataPacket, sizeof(dataPacket));` (ver [lib/control/control.cpp](lib/control/control.cpp))

El struct está marcado como `__attribute__((packed))` (en [lib/control/control.cpp](lib/control/control.cpp)). Tamaño esperado: **24 bytes**:
- Header: 2 bytes (0xA5, 0x5A)
- time_ms: uint32 (4)
- setpoint_rpm: float (4)
- real_rpm: float (4)
- current_A: float (4)
- pwm_val: float (4)
- terminator: 2 bytes (0x0D, 0x0A)

> Nota: El README dice CSV, pero el código implementa binario. Esto puede romper herramientas host existentes si esperaban CSV.

### 2.3 Frecuencia de envío
`PRINT_INTERVAL_MS = 20` en [lib/control/control.cpp](lib/control/control.cpp) → **50 Hz**.

---

## 3) Comunicación USB: qué soporta hoy y qué NO

### 3.1 Lo que sí hay
- Comunicación por `Serial` (Arduino) iniciada en [`control_init`](lib/control/control.cpp).
- En ESP32-S3 esto normalmente puede ser **USB CDC** (dependiendo de flags/board) o “UART-USB bridge” si la placa lo trae.

**Resultado:** “comunicación por USB” tipo puerto COM/tty **sí**.

### 3.2 Lo que NO hay
No hay implementación de:
- USB Mass Storage (MSC) para que el ESP32-S3 se monte como “memoria USB”
- MTP (Media Transfer Protocol)
- Protocolo de transferencia de archivos por Serial (XMODEM/YMODEM/ZMODEM/CRC custom)

**Resultado:** “cargar archivos” como si fuera una USB **no** está implementado por el firmware actual.

---

## 4) Decisión pendiente (lo más importante a definir en el nuevo chat)
**¿Qué significa “comunicación directamente desde una USB” y “cargar archivos”?**

Elegir 1 ruta principal:

### Ruta A — USB CDC (Serial) + protocolo propio (recomendado si es simple)
- El ESP32-S3 aparece como puerto serial
- Se manda telemetría binaria (ya existe) y/o comandos
- Para “cargar archivos”, se define un protocolo:
  - `START name size crc32` → chunks → `END crc`
  - Guardar en flash (LittleFS/SPIFFS) o SD

Pros: simple, portable, no requiere “montar disco”.
Contras: no es “arrastrar y soltar” en el explorador.

### Ruta B — USB Mass Storage (MSC) (si se necesita “unidad USB” real)
- El ESP32-S3 se presenta como **dispositivo de almacenamiento**
- La PC monta una unidad y se copian archivos

Pros: UX tipo USB real.
Contras: complejidad, cuidado con concurrencia (PC escribiendo mientras firmware lee), necesidad de FAT y backend (flash o SD). Requiere TinyUSB/USBMSC.

### Ruta C — MTP (alternativa a MSC)
Pros: mejor que MSC para evitar corrupción por acceso concurrente.
Contras: también complejo, soporte variable según OS.

**Siguiente paso del nuevo chat:** elegir Ruta A/B/C según el caso de uso real.

---

## 5) “Checklist” de información clave para el nuevo chat

### 5.1 Hardware / board
- Board: ESP32-S3 DevKitC-1 (según README)
- Pines en [include/config.h](include/config.h):
  - Motor: IN1=11, IN2=12, PWM=17, STBY=14
  - ADC: POT=4, CORRIENTE=5, VOLTAJE=6 (opcional)
  - Encoder: A=36, B=37

### 5.2 Consideraciones de datos binarios
- Endianness: ESP32 es little-endian; floats IEEE-754 32-bit
- `packed`: elimina padding, pero en host hay que decodificar con el layout exacto
- Sincronización:
  - header fijo `A5 5A`
  - terminator `0D 0A`
  - recomendable buscar header y validar terminator para re-sync

### 5.3 Diferencia README vs firmware
- README menciona CSV; firmware actual envía binario.
- Si existe software host (MATLAB) esperando CSV, hay que:
  - O volver a CSV (menos eficiente)
  - O actualizar host para parsear binario (más robusto)

---

## 6) Snippets de referencia (para que el nuevo chat entienda rápido)

### 6.1 Layout del paquete (referencia documental)
Tomado de [`TelemetryPacket`](lib/control/control.cpp) en [lib/control/control.cpp](lib/control/control.cpp):

````cpp
// Referencia de layout (no necesariamente para pegar tal cual en el firmware)
/*
struct TelemetryPacket {
  uint8_t  header[2];      // A5 5A
  uint32_t time_ms;        // little-endian
  float    setpoint_rpm;   // IEEE754 float32
  float    real_rpm;
  float    current_A;
  float    pwm_val;
  uint8_t  terminator[2];  // 0D 0A
} __attribute__((packed)); // total 24 bytes
*/

### 6.2 Parser en Python (host) para telemetría binaria

import serial, struct

PORT = "COM5"      # o "/dev/ttyACM0"
BAUD = 115200
PKT_LEN = 24
HDR = b"\xA5\x5A"
TMR = b"\x0D\x0A"

def read_packet(ser: serial.Serial):
    # Re-sync buscando header
    while True:
        b = ser.read(1)
        if not b:
            return None
        if b == HDR[:1]:
            b2 = ser.read(1)
            if b2 == HDR[1:]:
                payload = ser.read(PKT_LEN - 2)  # ya leímos 2 bytes header
                if len(payload) != PKT_LEN - 2:
                    return None
                pkt = HDR + payload
                if pkt[-2:] != TMR:
                    continue
                # < = little-endian; I=uint32; f=float32
                time_ms, sp, real, cur, pwm = struct.unpack_from("<Iffff", pkt, 2)
                return time_ms, sp, real, cur, pwm

with serial.Serial(PORT, BAUD, timeout=1) as ser:
    while True:
        out = read_packet(ser)
        if out:
            print(out)

## 10) Puntos de entrada y símbolos “clave” (para navegar rápido)

- Arranque: src/main.cpp
- Init: control_init en lib/control/control.cpp
- Loop: control_loop en lib/control/control.cpp
- Encoder RPM: encoder_get_velocity_rpm en lib/encoder/encoder.cpp
- ADC setpoint: pot_get_target_rpm en lib/potenciometro/potenciometro.cpp
- Corriente: sensor_get_corriente_A en lib/sensores/sensores.cpp
- Motor: motor_move en lib/motor/motor.cpp
- Config: include/config.h