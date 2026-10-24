
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "xtensa/core-macros.h"

#include "hal_motor.h"
#include "fsm_control.h"

// Pin del paro de emergencia
constexpr uint8_t PIN_EMERGENCIA = 33;

// Antirrebote de 25 ms a 160 MHz
constexpr uint32_t REBOTE_CICLOS = 4000000UL;

// Variables compartidas con la interrupcion
volatile bool eventoEmergencia = false;
volatile uint32_t contadorPulsaciones = 0;
volatile uint32_t ultimoCiclo = 0;

// Proteccion de variables compartidas
portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

// Interrupcion del boton de emergencia
void IRAM_ATTR emergenciaISR() {

    uint32_t cicloActual = XTHAL_GET_CCOUNT();

    portENTER_CRITICAL_ISR(&spinlock);

    if ((uint32_t)(cicloActual - ultimoCiclo)
        >= REBOTE_CICLOS) {

        ultimoCiclo = cicloActual;
        eventoEmergencia = true;
        contadorPulsaciones++;
    }

    portEXIT_CRITICAL_ISR(&spinlock);
}

// Inicializacion del sistema
void setup() {

    Serial.begin(115200);

    pinMode(PIN_EMERGENCIA, INPUT_PULLUP);

    motorInit();
    fsmInit();

    // Configurar interrupcion
    attachInterrupt(
        digitalPinToInterrupt(PIN_EMERGENCIA),
        emergenciaISR,
        FALLING
    );

    Serial.println("SISTEMA INICIADO");
    Serial.println("Comandos disponibles:");
    Serial.println("A = Arrancar banda");
    Serial.println("P = Parar banda");
    Serial.println("R = Reiniciar sistema");

    // Comprobar emergencia al encender
    if (digitalRead(PIN_EMERGENCIA) == LOW) {
        fsmProcesar(Evento::EMERGENCY);
    }
}

// Bucle principal
void loop() {

    bool emergenciaPendiente = false;
    uint32_t totalPulsaciones = 0;

    // Leer variables protegidas
    portENTER_CRITICAL(&spinlock);

    emergenciaPendiente = eventoEmergencia;
    eventoEmergencia = false;
    totalPulsaciones = contadorPulsaciones;

    portEXIT_CRITICAL(&spinlock);

    // Prioridad del paro de emergencia
    if (emergenciaPendiente ||
        digitalRead(PIN_EMERGENCIA) == LOW) {

        if (fsmObtenerEstado() != Estado::FAULT) {

            fsmProcesar(Evento::EMERGENCY);

            Serial.print("Pulsaciones: ");
            Serial.println(totalPulsaciones);
        }
    }

    // Leer comandos del monitor serial
    if (Serial.available() > 0) {

        char comando = Serial.read();

        switch (comando) {

            case 'A':
            case 'a':

                if (digitalRead(PIN_EMERGENCIA) == HIGH &&
                    fsmObtenerEstado() == Estado::IDLE) {

                    fsmProcesar(Evento::START);
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

                if (digitalRead(PIN_EMERGENCIA) == HIGH) {
                    fsmProcesar(Evento::RESET);
                } else {
                    Serial.println("Libere la emergencia");
                }
                break;

            default:
                break;
        }
    }
}
