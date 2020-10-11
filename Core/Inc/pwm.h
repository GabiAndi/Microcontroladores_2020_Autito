#ifndef INC_PWM_H_
#define INC_PWM_H_

#include "system.h"

void pwm_init(void);

void pwm_set_stop_motor(uint16_t ms);

void pwm_stop_motor(void);

void pwm_set_motor_der_speed(float vel);
void pwm_set_motor_izq_speed(float vel);

#endif /* INC_PWM_H_ */
