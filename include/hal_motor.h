
#ifndef HAL_MOTOR_H
#define HAL_MOTOR_H

#include <stdint.h>

// Pines del motor en la IdeaBoard
constexpr uint8_t PIN_MOTOR_A = 12;
constexpr uint8_t PIN_MOTOR_B = 14;

// Configuracion PWM
constexpr uint32_t PWM_FRECUENCIA = 1000;
constexpr uint8_t PWM_RESOLUCION = 10;
constexpr uint16_t PWM_MAX = 1023;

// Funciones para controlar el motor
void motorInit();
void motorAvanzar(uint16_t velocidad);
void motorFrenar();
void motorLibre();
void motorStop();

#endif
