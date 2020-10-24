#include "pwm.h"

/**********************************************************************************/
/********************************** Variables *************************************/
/**********************************************************************************/

/*********************************** Tickers **************************************/
ticker_t ticker_pwm_stop_motor;
/**********************************************************************************/

/******************************* PWM de la HAL ************************************/
extern TIM_HandleTypeDef htim4;
/**********************************************************************************/

/************************* Velocidad de los motores *******************************/
float vel_mot_der;
float vel_mot_izq;
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/
void pwm_init(void)
{
	/***********************************************************************************/
	/**************** Inicializacion de las velocidades de los motores *****************/
	/***********************************************************************************/
	vel_mot_der = 0.0;
	vel_mot_izq = 0.0;
	/***********************************************************************************/

	/***********************************************************************************/
	/********************************* Inicio del PWM **********************************/
	/***********************************************************************************/
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
	/***********************************************************************************/

	/***********************************************************************************/
	/************************** Inicializacion de los tickers **************************/
	/***********************************************************************************/

	/********************* Ticker para la detencion de los motores *********************/
	ticker_pwm_stop_motor.ms_count = 0;
	ticker_pwm_stop_motor.ms_max = 0;
	ticker_pwm_stop_motor.calls = 0;
	ticker_pwm_stop_motor.priority = TICKER_LOW_PRIORITY;
	ticker_pwm_stop_motor.ticker_function = pwm_stop_motor;
	ticker_pwm_stop_motor.active = TICKER_NO_ACTIVE;

	ticker_new(&ticker_pwm_stop_motor);
	/***********************************************************************************/

	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/
}

void pwm_change_freq(uint16_t freq)
{
	__HAL_TIM_SET_AUTORELOAD(&htim4, (72000000.0 / (4.0 * freq)));

	pwm_set_motor_der_speed(vel_mot_der);
	pwm_set_motor_izq_speed(vel_mot_izq);
}

void pwm_set_stop_motor(uint16_t ms)
{
	ticker_pwm_stop_motor.ms_count = 0;
	ticker_pwm_stop_motor.ms_max = ms;
	ticker_pwm_stop_motor.active = TICKER_ACTIVE;
}

void pwm_stop_motor(void)
{
	ticker_pwm_stop_motor.active = TICKER_NO_ACTIVE;

	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);

	vel_mot_der = 0;
	vel_mot_izq = 0;
}

void pwm_set_motor_der_speed(float vel)
{
	vel_mot_der = vel;

	if ((vel_mot_der > -0.01) && (vel_mot_der < 0.01))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	}

	else if ((vel_mot_der >= 0.01) && (vel_mot_der <= 100.00))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel_mot_der / 100.0)));
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	}

	else if (vel_mot_der > 100.00)
	{
		vel_mot_der = 100.00;

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel_mot_der / 100.0)));
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	}

	else if ((vel_mot_der <= -0.01) && (vel_mot_der >= -100.00))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel_mot_der / 100.0) * -1.0));
	}

	else if (vel_mot_der < -100.00)
	{
		vel_mot_der = -100.00;

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel_mot_der / 100.0) * -1.0));
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	}
}

void pwm_set_motor_izq_speed(float vel)
{
	vel_mot_izq = vel;

	if ((vel_mot_izq > -0.01) && (vel_mot_izq < 0.01))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
	}

	else if ((vel_mot_izq >= 0.01) && (vel_mot_izq <= 100.00))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel_mot_izq / 100.0)));
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
	}

	else if (vel_mot_izq > 100.00)
	{
		vel_mot_izq = 100.00;

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel_mot_izq / 100.0)));
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
	}

	else if ((vel_mot_izq <= -0.01) && (vel_mot_izq >= -100.00))
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel_mot_izq / 100.0) * -1.0));
	}

	else if (vel_mot_izq < -100.00)
	{
		vel_mot_izq = -100.00;

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, (__HAL_TIM_GET_AUTORELOAD(&htim4) * (vel_mot_izq / 100.0) * -1.0));
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
	}
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
