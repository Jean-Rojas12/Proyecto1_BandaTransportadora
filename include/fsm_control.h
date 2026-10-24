
#ifndef FSM_CONTROL_H
#define FSM_CONTROL_H

#include <stdint.h>

// Estados del sistema
enum class Estado : uint8_t {
    IDLE,
    RUNNING,
    FAULT,
    TOTAL_ESTADOS
};

// Eventos del sistema
enum class Evento : uint8_t {
    NONE,
    START,
    EMERGENCY,
    TIMEOUT,
    RESET,
    TOTAL_EVENTOS
};

// Estructura de transiciones
struct Transition {
    Estado siguienteEstado;
    void (*accion)();
};

// Funciones de la FSM
void fsmInit();
void fsmProcesar(Evento evento);
Estado fsmObtenerEstado();

// Acciones del motor
void accionReposo();
void accionArrancar();
void accionFalla();
void accionReiniciar();

#endif
