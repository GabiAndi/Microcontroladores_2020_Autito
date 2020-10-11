#include "pwm.h"

// Tickers
ticker_t ticker_pwm_stop_motor;

// Estructura del pwm
extern TIM_HandleTypeDef htim4;

void pwm_init(void)
{
	// Inicio del PWM
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

	// Configuracion de los tickers
	ticker_pwm_stop_motor.ms_count = 0;
	ticker_pwm_stop_motor.ms_max = 0;
	ticker_pwm_stop_motor.calls = 0;
	ticker_pwm_stop_motor.priority = TICKER_LOW_PRIORITY;
	ticker_pwm_stop_motor.ticker_function = pwm_stop_motor;
	ticker_pwm_stop_motor.active = TICKER_DEACTIVATE;

	ticker_new(&ticker_pwm_stop_motor);
}

void pwm_set_stop_motor(uint16_t ms)
{
	ticker_pwm_stop_motor.ms_count = ms;
	ticker_pwm_stop_motor.active = TICKER_ACTIVE;
}

void pwm_stop_motor(void)
{
	ticker_pwm_stop_motor.active = TICKER_DEACTIVATE;

	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
}

void pwm_set_motor_der_speed(float vel)
{
	if ((vel > -0.01) && (vel < 0.01))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	}

	else if ((vel >= 0.01) && (vel <= 100.00))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (__HAL_TIM_GET_AUTORELOAD(&htim4) * vel));
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	}

	else if ((vel <= -0.01) && (vel >= -100.00))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel * -1.0)));
	}
}

void pwm_set_motor_izq_speed(float vel)
{
	if ((vel > -0.01) && (vel < 0.01))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
	}

	else if ((vel >= 0.01) && (vel <= 100.00))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, (__HAL_TIM_GET_AUTORELOAD(&htim4) * vel));
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
	}

	else if ((vel <= -0.01) && (vel >= -100.00))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel * -1.0)));
	}
}
