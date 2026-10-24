
#include <Arduino.h>
#include "hal_motor.h"
#include "driver/ledc.h"

// Canales PWM del puente H
constexpr ledc_channel_t CANAL_A = LEDC_CHANNEL_0;
constexpr ledc_channel_t CANAL_B = LEDC_CHANNEL_1;

// Modo y temporizador del ESP32
constexpr ledc_mode_t MODO_PWM = LEDC_HIGH_SPEED_MODE;
constexpr ledc_timer_t TIMER_PWM = LEDC_TIMER_0;

// Actualizar el ciclo de trabajo
static void actualizarPWM(ledc_channel_t canal,
                          uint16_t velocidad) {

    if (velocidad > PWM_MAX) {
        velocidad = PWM_MAX;
    }

    ledc_set_duty(MODO_PWM, canal, velocidad);
    ledc_update_duty(MODO_PWM, canal);
}

// Inicializar el control del motor
void motorInit() {

    // Configuracion del temporizador
    ledc_timer_config_t timer = {};

    timer.speed_mode = MODO_PWM;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.timer_num = TIMER_PWM;
    timer.freq_hz = PWM_FRECUENCIA;
    timer.clk_cfg = LEDC_AUTO_CLK;

    ledc_timer_config(&timer);

    // Configuracion del canal A
    ledc_channel_config_t canalA = {};

    canalA.gpio_num = PIN_MOTOR_A;
    canalA.speed_mode = MODO_PWM;
    canalA.channel = CANAL_A;
    canalA.intr_type = LEDC_INTR_DISABLE;
    canalA.timer_sel = TIMER_PWM;
    canalA.duty = 0;
    canalA.hpoint = 0;

    ledc_channel_config(&canalA);

    // Configuracion del canal B
    ledc_channel_config_t canalB = canalA;

    canalB.gpio_num = PIN_MOTOR_B;
    canalB.channel = CANAL_B;

    ledc_channel_config(&canalB);

    // Motor apagado al iniciar
    motorLibre();
}

// Motor hacia adelante
void motorAvanzar(uint16_t velocidad) {

    // Apagar primero el canal contrario
    actualizarPWM(CANAL_B, 0);

    // Aplicar velocidad al motor
    actualizarPWM(CANAL_A, velocidad);
}

// Frenado dinamico
void motorFrenar() {

    actualizarPWM(CANAL_A, PWM_MAX);
    actualizarPWM(CANAL_B, PWM_MAX);
}

// Detencion por inercia
void motorLibre() {

    actualizarPWM(CANAL_A, 0);
    actualizarPWM(CANAL_B, 0);
}

// Paro del motor
void motorStop() {

    motorFrenar();
}
