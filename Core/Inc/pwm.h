#ifndef INC_PWM_H_
#define INC_PWM_H_

/**********************************************************************************/
/*********************************** Includes *************************************/
/**********************************************************************************/
#include <inttypes.h>

#include "stm32f1xx_hal.h"

#include "ticker.h"
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/

/*
 * Funcion que inicia la configuración del PWM
 *
 */
void pwm_init(void);

/*
 * Funcion que cambia la frecuencia del PWM
 *
 */
void pwm_change_freq(uint16_t freq);

/*
 * Funcion que establece el tiempo de parada para el PWM
 *
 */
void pwm_set_stop_motor(uint16_t ms);

/*
 * Funcion que establece el PWM en 0
 *
 */
void pwm_stop_motor(void);

/*
 * Funcion que establece la velocidad del motor de la derecha en %
 *
 */
void pwm_set_motor_der_speed(int16_t vel);

/*
 * Funcion que establece la velocidad del motor de la izquierda en %
 *
 */
void pwm_set_motor_izq_speed(int16_t vel);

/*
 * Funcion que devuelve la velocidad actual del motor derecho
 *
 */
int16_t pwm_get_motor_der_speed(void);

/*
 * Funcion que devuelve la velocidad actual del motor izquierdo
 *
 */
int16_t pwm_get_motor_izq_speed(void);
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

#endif /* INC_PWM_H_ */
