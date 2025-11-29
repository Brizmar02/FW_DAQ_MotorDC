#!/bin/bash

# Configuración
PUERTO_SERIAL="/dev/ttyACM0"
PUERTO_TCP="8080"
BAUDIOS="115200"

echo "------------------------------------------------"
echo "   Puente Serial <-> TCP (ESP32 a MATLAB)       "
echo "------------------------------------------------"

# 1. Verificar si el ESP32 está conectado
if [ ! -e "$PUERTO_SERIAL" ]; then
    echo "ERROR: No se detecta el dispositivo en $PUERTO_SERIAL"
    echo "Intenta: ls /dev/tty* para ver si cambió a /dev/ttyACM0"
    exit 1
fi

# 2. Verificar permisos (opcional, suele arreglarse con sudo o dialout group)
if [ ! -r "$PUERTO_SERIAL" ]; then
    echo "ADVERTENCIA: No tienes permiso de lectura en $PUERTO_SERIAL"
    echo "Intentando ejecutar con sudo..."
    CMD_PREFIX="sudo"
else
    CMD_PREFIX=""
fi

echo "ESP32 detectado."
echo "Escuchando en el puerto $PUERTO_TCP..."
echo "Presiona Ctrl+C para detener."

# 3. Ejecutar socat
# La opción -d -d hace que socat imprima info útil si algo falla
$CMD_PREFIX socat -d -d TCP-LISTEN:$PUERTO_TCP,fork,reuseaddr FILE:$PUERTO_SERIAL,b$BAUDIOS,raw,echo=0