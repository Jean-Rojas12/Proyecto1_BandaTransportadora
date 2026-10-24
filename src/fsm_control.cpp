
#include <Arduino.h>
#include "fsm_control.h"
#include "hal_motor.h"

// Estado inicial de la banda
static Estado estadoActual = Estado::IDLE;

// Velocidad del motor
constexpr uint16_t VELOCIDAD_MOTOR = 700;

// Funciones de control
void accionReposo() {
    motorLibre();
    Serial.println("Banda detenida");
}

void accionArrancar() {
    motorAvanzar(VELOCIDAD_MOTOR);
    Serial.println("Banda en funcionamiento");
}

void accionFalla() {
    motorStop();
    Serial.println("Emergencia activada");
}

void accionReiniciar() {
    motorStop();
    Serial.println("Sistema reiniciado");
}

// No realizar ninguna accion
static void sinAccion() {
}

// Matriz de transiciones
// Columnas: NONE, START, EMERGENCY, TIMEOUT, RESET
static constexpr Transition matrizFSM[3][5] = {

    // IDLE
    {
        {Estado::IDLE, sinAccion},
        {Estado::RUNNING, accionArrancar},
        {Estado::FAULT, accionFalla},
        {Estado::IDLE, sinAccion},
        {Estado::IDLE, accionReposo}
    },

    // RUNNING
    {
        {Estado::RUNNING, sinAccion},
        {Estado::RUNNING, sinAccion},
        {Estado::FAULT, accionFalla},
        {Estado::IDLE, accionReposo},
        {Estado::IDLE, accionReiniciar}
    },

    // FAULT
    {
        {Estado::FAULT, sinAccion},
        {Estado::FAULT, sinAccion},
        {Estado::FAULT, accionFalla},
        {Estado::FAULT, sinAccion},
        {Estado::IDLE, accionReiniciar}
    }
};

// Inicializar la FSM
void fsmInit() {
    estadoActual = Estado::IDLE;
    accionReposo();
}

// Procesar evento mediante matriz
void fsmProcesar(Evento evento) {

    const uint8_t fila =
        static_cast<uint8_t>(estadoActual);

    const uint8_t columna =
        static_cast<uint8_t>(evento);

    if (fila >= static_cast<uint8_t>(Estado::TOTAL_ESTADOS) ||
        columna >= static_cast<uint8_t>(Evento::TOTAL_EVENTOS)) {
        return;
    }

    const Transition &transicion = matrizFSM[fila][columna];

    // Ejecutar primero la accion de seguridad
    if (evento == Evento::EMERGENCY) {
        transicion.accion();
        estadoActual = transicion.siguienteEstado;
    }
    else {
        estadoActual = transicion.siguienteEstado;
        transicion.accion();
    }
}

// Consultar estado actual
Estado fsmObtenerEstado() {
    return estadoActual;
}
