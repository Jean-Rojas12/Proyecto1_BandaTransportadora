
#include <Arduino.h>
#include "fsm_control.h"
#include "hal_motor.h"

// Estado inicial de la banda
static Estado estadoActual = Estado::IDLE;

// Velocidad del motor
constexpr uint16_t VELOCIDAD_MOTOR = 700;

// Acciones de la banda
void accionReposo() {
    motorLibre();
    Serial.println("Banda en reposo");
}

void accionArrancar() {
    motorAvanzar(VELOCIDAD_MOTOR);
    Serial.println("Banda en funcionamiento");
}

void accionFalla() {
    motorStop();
    Serial.println("Paro de emergencia activado");
}

void accionReiniciar() {
    motorLibre();
    Serial.println("Sistema reiniciado");
}

// Accion que no modifica el motor
static void sinAccion() {
}

// Matriz de transiciones
// Filas: estados
// Columnas: eventos
static constexpr Transition matrizFSM[3][4] = {

    // Estado IDLE
    {
        {Estado::IDLE, sinAccion},       // NONE
        {Estado::RUNNING, accionArrancar}, // BTN_PRESS
        {Estado::IDLE, sinAccion},       // TIMEOUT
        {Estado::IDLE, accionReposo}    // RESET
    },

    // Estado RUNNING
    {
        {Estado::RUNNING, sinAccion},    // NONE
        {Estado::FAULT, accionFalla},   // BTN_PRESS
        {Estado::IDLE, accionReposo},   // TIMEOUT
        {Estado::IDLE, accionReiniciar}  // RESET
    },

    // Estado FAULT
    {
        {Estado::FAULT, sinAccion},     // NONE
        {Estado::FAULT, sinAccion},     // BTN_PRESS
        {Estado::FAULT, sinAccion},     // TIMEOUT
        {Estado::IDLE, accionReiniciar} // RESET
    }
};

// Inicializar la maquina de estados
void fsmInit() {
    estadoActual = Estado::IDLE;
    accionReposo();
}

// Procesar un evento
void fsmProcesar(Evento evento) {

    uint8_t fila = static_cast<uint8_t>(estadoActual);
    uint8_t columna = static_cast<uint8_t>(evento);

    // Verificar que los indices sean validos
    if (fila >= 3 || columna >= 4) {
        return;
    }

    // Buscar la transicion en la matriz
    const Transition &transicion = matrizFSM[fila][columna];

    // Actualizar estado
    estadoActual = transicion.siguienteEstado;

    // Ejecutar accion correspondiente
    transicion.accion();
}

// Consultar estado actual
Estado fsmObtenerEstado() {
    return estadoActual;
}
