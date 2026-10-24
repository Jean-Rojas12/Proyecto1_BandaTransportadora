
#include <Arduino.h>
#include "hal_motor.h"
#include "driver/ledc.h"

// Canales PWM del motor
#define CANAL_MOTOR_A LEDC_CHANNEL_0
#define CANAL_MOTOR_B LEDC_CHANNEL_1

// Configuracion del temporizador PWM
void motorInit() {

    ledc_timer_config_t timer = {};
    timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.timer_num = LEDC_TIMER_0;
    timer.freq_hz = PWM_FRECUENCIA;
    timer.clk_cfg = LEDC_AUTO_CLK;

    ledc_timer_config(&timer);

    ledc_channel_config_t canalA = {};
    canalA.gpio_num = PIN_MOTOR_A;
    canalA.speed_mode = LEDC_HIGH_SPEED_MODE;
    canalA.channel = CANAL_MOTOR_A;
    canalA.intr_type = LEDC_INTR_DISABLE;
    canalA.timer_sel = LEDC_TIMER_0;
    canalA.duty = 0;
    canalA.hpoint = 0;

    ledc_channel_config(&canalA);

    ledc_channel_config_t canalB = canalA;
    canalB.gpio_num = PIN_MOTOR_B;
    canalB.channel = CANAL_MOTOR_B;

    ledc_channel_config(&canalB);

    motorLibre();
}

// Funcion para actualizar el PWM
static void actualizarPWM(
    ledc_channel_t canal,
    uint16_t duty
) {
    ledc_set_duty(
        LEDC_HIGH_SPEED_MODE,
        canal,
        duty
    );

    ledc_update_duty(
        LEDC_HIGH_SPEED_MODE,
        canal
    );
}

// Motor hacia adelante
void motorAvanzar(uint16_t velocidad) {

    if (velocidad > PWM_MAX) {
        velocidad = PWM_MAX;
    }

    actualizarPWM(CANAL_MOTOR_B, 0);
    actualizarPWM(CANAL_MOTOR_A, velocidad);
}

// Frenado dinamico
void motorFrenar() {
    actualizarPWM(CANAL_MOTOR_A, PWM_MAX);
    actualizarPWM(CANAL_MOTOR_B, PWM_MAX);
}

// Motor detenido por inercia
void motorLibre() {
    actualizarPWM(CANAL_MOTOR_A, 0);
    actualizarPWM(CANAL_MOTOR_B, 0);
}

// Detencion del motor
void motorStop() {
    motorFrenar();
}
