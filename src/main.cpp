
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "xtensa/core-macros.h"

#include "hal_motor.h"
#include "fsm_control.h"

// Configuracion del interruptor
constexpr uint8_t PIN_EMERGENCIA = 33;

// 25 ms a 160 MHz
constexpr uint32_t REBOTE_CICLOS = 4000000UL;

// Variables compartidas con la ISR
volatile bool eventoEmergencia = false;
volatile uint32_t contadorPulsaciones = 0;
volatile uint32_t ultimoCiclo = 0;
volatile bool primeraPulsacion = true;

// Proteccion entre los dos nucleos
portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

// Interrupcion del paro de emergencia
void IRAM_ATTR emergenciaISR() {

    uint32_t cicloActual = XTHAL_GET_CCOUNT();

    portENTER_CRITICAL_ISR(&spinlock);

    uint32_t diferencia = cicloActual - ultimoCiclo;

    if (primeraPulsacion ||
        diferencia >= REBOTE_CICLOS) {

        ultimoCiclo = cicloActual;
        primeraPulsacion = false;

        eventoEmergencia = true;
        contadorPulsaciones++;
    }

    portEXIT_CRITICAL_ISR(&spinlock);
}

// Consultar si existe una emergencia
bool emergenciaActiva() {

    if (digitalRead(PIN_EMERGENCIA) == LOW) {
        return true;
    }

    bool pendiente;

    portENTER_CRITICAL(&spinlock);
    pendiente = eventoEmergencia;
    portEXIT_CRITICAL(&spinlock);

    return pendiente;
}

// Procesar emergencia pendiente
void revisarEmergencia() {

    bool pendiente;
    uint32_t pulsaciones;

    portENTER_CRITICAL(&spinlock);

    pendiente = eventoEmergencia;
    eventoEmergencia = false;
    pulsaciones = contadorPulsaciones;

    portEXIT_CRITICAL(&spinlock);

    if (pendiente ||
        digitalRead(PIN_EMERGENCIA) == LOW) {

        if (fsmObtenerEstado() != Estado::FAULT) {

            fsmProcesar(Evento::EMERGENCY);

            Serial.println("PARO DE EMERGENCIA");
            Serial.print("Pulsaciones: ");
            Serial.println(pulsaciones);
        }
    }
}

// Inicializacion
void setup() {

    Serial.begin(115200);

    pinMode(PIN_EMERGENCIA, INPUT_PULLUP);

    motorInit();
    fsmInit();

    // Verificar el interruptor antes de habilitarlo
    if (digitalRead(PIN_EMERGENCIA) == LOW) {
        fsmProcesar(Evento::EMERGENCY);
    }

    attachInterrupt(
        digitalPinToInterrupt(PIN_EMERGENCIA),
        emergenciaISR,
        FALLING
    );

    Serial.println("SISTEMA INICIADO");
    Serial.println("A = Arrancar banda");
    Serial.println("P = Parar banda");
    Serial.println("R = Reiniciar sistema");
}

// Programa principal
void loop() {

    // Dar prioridad a la emergencia
    revisarEmergencia();

    // Leer comandos sin bloquear
    if (Serial.available() > 0) {

        char comando = Serial.read();

        switch (comando) {

            case 'A':
            case 'a':

                if (emergenciaActiva()) {

                    revisarEmergencia();

                    Serial.println(
                        "Arranque bloqueado por emergencia"
                    );
                }
                else if (fsmObtenerEstado() == Estado::FAULT) {

                    Serial.println(
                        "Reinicie antes de arrancar"
                    );
                }
                else if (fsmObtenerEstado() == Estado::IDLE) {

                    fsmProcesar(Evento::START);
                }
                else {

                    Serial.println(
                        "La banda ya esta encendida"
                    );
                }

                break;

            case 'P':
            case 'p':

                if (fsmObtenerEstado() == Estado::RUNNING) {

                    fsmProcesar(Evento::TIMEOUT);
                }

                break;

            case 'R':
            case 'r':

                if (emergenciaActiva()) {

                    revisarEmergencia();

                    Serial.println(
                        "No se puede reiniciar"
                    );
                }
                else if (fsmObtenerEstado() == Estado::FAULT) {

                    fsmProcesar(Evento::RESET);

                    Serial.println(
                        "Sistema listo para arrancar"
                    );
                }

                break;

            default:
                break;
        }
    }

    // Segunda comprobacion de emergencia
    revisarEmergencia();
}
