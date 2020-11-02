#include "system.h"

/**********************************************************************************/
/********************************** Variables *************************************/
/**********************************************************************************/

/******************************* Datos en flash ***********************************/
__attribute__ ((__section__(".user_data_flash"))) system_flash_data_t system_flash_user;	// Datos en flash
system_flash_data_t system_ram_user;	// Datos en ram

uint8_t system_flash_enabled;	// Seguridad de escritura de la flash
/**********************************************************************************/

/*********************************** Tickers **************************************/
ticker_t system_ticker_flash_enable;
ticker_t system_ticker_led_status;
ticker_t system_ticker_adc_send_data;
ticker_t system_ticker_error_send_data;
ticker_t system_ticker_control_set;
ticker_t system_ticker_pid_control;
/**********************************************************************************/

/***************************** Bufferes de datos **********************************/
extern system_ring_buffer_t esp_buffer_write;	// Buffer de la ESP

extern adc_buffer_t adc_buffer;	// Buffer de captura del ADC

system_ring_buffer_t *system_adc_buffer_send_data;	// Buffer de envio de datos del ADC

system_ring_buffer_t *system_error_buffer_send_data;	// Buffer de envio de datos del error

system_ring_buffer_t *system_control_buffer_send_data;	// Buffer de envio de datos del control
/**********************************************************************************/

/***************************** Depuracion via USB *********************************/
uint8_t system_usb_debug;
/**********************************************************************************/

/**************************** Conversion de datos *********************************/
system_byte_converter_u system_byte_converter;
/**********************************************************************************/

/***************************** Control del autito *********************************/
system_pid_t system_pid;
/**********************************************************************************/

/**************************** Variables auxiliares ********************************/
uint8_t system_index_init;	// Sirve para guardar el indice de inicio para calcular el checksum

uint8_t system_aux_ack;	// Auxiliares de respuesta

uint8_t system_aux_i, system_aux_j;	// Auxiliares de iteración
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/
void system_init(void)
{
	/*
	 * Compruebo la integridad de los datos de la flash antes de cargar
	 *
	 * Si los datos estan bien son cargados a la flash, de otra forma
	 * los mismos son inicializados en 0.
	 *
	 */
	if (system_flash_user.checksum == system_flash_check_integrity(&system_flash_user))
	{
		memcpy(&system_ram_user, &system_flash_user, sizeof(system_flash_data_t));
	}

	else
	{
		memset(&system_ram_user, 0, sizeof(system_flash_data_t));
	}

	/***********************************************************************************/
	/*************************** Configuracion de prueba *******************************/
	/***********************************************************************************/
	system_ram_user.ssid_length = 10;

	system_ram_user.ssid[0] = 'T';
	system_ram_user.ssid[1] = 'P';
	system_ram_user.ssid[2] = 'L';
	system_ram_user.ssid[3] = 'I';
	system_ram_user.ssid[4] = 'N';
	system_ram_user.ssid[5] = 'K';
	system_ram_user.ssid[6] = '_';
	system_ram_user.ssid[7] = '2';
	system_ram_user.ssid[8] = '4';
	system_ram_user.ssid[9] = 'G';

	system_ram_user.psw_length = 14;

	system_ram_user.psw[0] = 'B';
	system_ram_user.psw[1] = 'a';
	system_ram_user.psw[2] = 's';
	system_ram_user.psw[3] = 'e';
	system_ram_user.psw[4] = 'x';
	system_ram_user.psw[5] = 'B';
	system_ram_user.psw[6] = '1';
	system_ram_user.psw[7] = 'A';
	system_ram_user.psw[8] = 'u';
	system_ram_user.psw[9] = '1';
	system_ram_user.psw[10] = '9';
	system_ram_user.psw[11] = '7';
	system_ram_user.psw[12] = '4';
	system_ram_user.psw[13] = '*';

	system_ram_user.ip_mcu_length = 13;

	system_ram_user.ip_mcu[0] = '1';
	system_ram_user.ip_mcu[1] = '9';
	system_ram_user.ip_mcu[2] = '2';
	system_ram_user.ip_mcu[3] = '.';
	system_ram_user.ip_mcu[4] = '1';
	system_ram_user.ip_mcu[5] = '6';
	system_ram_user.ip_mcu[6] = '8';
	system_ram_user.ip_mcu[7] = '.';
	system_ram_user.ip_mcu[8] = '0';
	system_ram_user.ip_mcu[9] = '.';
	system_ram_user.ip_mcu[10] = '1';
	system_ram_user.ip_mcu[11] = '0';
	system_ram_user.ip_mcu[12] = '0';

	system_ram_user.ip_pc_length = 12;

	system_ram_user.ip_pc[0] = '1';
	system_ram_user.ip_pc[1] = '9';
	system_ram_user.ip_pc[2] = '2';
	system_ram_user.ip_pc[3] = '.';
	system_ram_user.ip_pc[4] = '1';
	system_ram_user.ip_pc[5] = '6';
	system_ram_user.ip_pc[6] = '8';
	system_ram_user.ip_pc[7] = '.';
	system_ram_user.ip_pc[8] = '0';
	system_ram_user.ip_pc[9] = '.';
	system_ram_user.ip_pc[10] = '1';
	system_ram_user.ip_pc[11] = '7';

	system_ram_user.port_length = 5;

	system_ram_user.port[0] = '5';
	system_ram_user.port[1] = '0';
	system_ram_user.port[2] = '0';
	system_ram_user.port[3] = '0';
	system_ram_user.port[4] = '0';
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	system_flash_enabled = SYSTEM_FLASH_SAVE_DATA_DISABLED;	// Se habilita la flash para poder escribir

	system_usb_debug = SYSTEM_USB_DEBUG_OFF;	// La depuracion via USB inicia apagada

	/***********************************************************************************/
	/********************** Inicializacion de los valores del PID **********************/
	/***********************************************************************************/
	system_pid.state = SYSTEM_CONTROL_STATE_OFF;

	system_ram_user.kp = system_ram_user.kp;
	system_ram_user.kd = system_ram_user.kd;
	system_ram_user.ki = system_ram_user.ki;

	system_pid.vel_mot_der = 0;
	system_pid.vel_mot_izq = 0;
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	/***********************************************************************************/
	/************************* Inicializacion de los bufferes **************************/
	/***********************************************************************************/
	system_adc_buffer_send_data = 0;
	system_error_buffer_send_data = 0;
	system_control_buffer_send_data = 0;
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	ticker_init_core();	// Inicia la configuracion de los tickers

	/***********************************************************************************/
	/************************** Inicializacion de los tickers **************************/
	/***********************************************************************************/

	/********************** Ticker para el guardado de la flash ************************/
	system_ticker_flash_enable.ms_max = 10000;
	system_ticker_flash_enable.ms_count = 0;
	system_ticker_flash_enable.calls = 0;
	system_ticker_flash_enable.active = TICKER_ACTIVE;
	system_ticker_flash_enable.priority = TICKER_LOW_PRIORITY;
	system_ticker_flash_enable.ticker_function = system_flash_enable;

	ticker_new(&system_ticker_flash_enable);
	/***********************************************************************************/

	/************************** Ticker para el led de estado ***************************/
	system_ticker_led_status.ms_max = SYSTEM_LED_FAIL;
	system_ticker_led_status.ms_count = 0;
	system_ticker_led_status.calls = 0;
	system_ticker_led_status.active = TICKER_ACTIVE;
	system_ticker_led_status.priority = TICKER_LOW_PRIORITY;
	system_ticker_led_status.ticker_function = system_led_blink;

	ticker_new(&system_ticker_led_status);
	/***********************************************************************************/

	/************************** Ticker envio de datos del adc **************************/
	system_ticker_adc_send_data.ms_count = 0;
	system_ticker_adc_send_data.ms_max = 255;
	system_ticker_adc_send_data.calls = 0;
	system_ticker_adc_send_data.priority = TICKER_LOW_PRIORITY;
	system_ticker_adc_send_data.ticker_function = system_adc_send_data;
	system_ticker_adc_send_data.active = TICKER_NO_ACTIVE;

	ticker_new(&system_ticker_adc_send_data);
	/***********************************************************************************/

	/************************* Ticker envio de datos del error *************************/
	system_ticker_error_send_data.ms_count = 0;
	system_ticker_error_send_data.ms_max = 255;
	system_ticker_error_send_data.calls = 0;
	system_ticker_error_send_data.priority = TICKER_LOW_PRIORITY;
	system_ticker_error_send_data.ticker_function = system_error_send_data;
	system_ticker_error_send_data.active = TICKER_NO_ACTIVE;

	ticker_new(&system_ticker_error_send_data);
	/***********************************************************************************/

	/********************* Ticker para activar o desactivar el PID *********************/
	system_ticker_control_set.ms_count = 0;
	system_ticker_control_set.ms_max = 1;
	system_ticker_control_set.calls = 0;
	system_ticker_control_set.priority = TICKER_LOW_PRIORITY;
	system_ticker_control_set.ticker_function = system_control_state_button;
	system_ticker_control_set.active = TICKER_ACTIVE;

	ticker_new(&system_ticker_control_set);
	/***********************************************************************************/

	/********************************** Ticker del PID *********************************/
	system_ticker_pid_control.ms_count = 0;
	system_ticker_pid_control.ms_max = SYSTEM_CONTROL_RES_MS;
	system_ticker_pid_control.calls = 0;
	system_ticker_pid_control.priority = TICKER_LOW_PRIORITY;
	system_ticker_pid_control.ticker_function = system_pid_control;
	system_ticker_pid_control.active = TICKER_ACTIVE;

	ticker_new(&system_ticker_pid_control);
	/***********************************************************************************/

	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	/***********************************************************************************/
	/************************** Inicializacion de los modulos **************************/
	/***********************************************************************************/
	adc_init();	// Inicia la configuracion del ADC
	pwm_init();	// Inicia la configuracion del PWM
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/
}

void system_led_blink(void)
{
	if (system_pid.state == SYSTEM_CONTROL_STATE_ON)
	{
		system_ticker_led_status.ms_max = 50;
	}

	HAL_GPIO_TogglePin(SYSTEM_LED_GPIO_Port, SYSTEM_LED_Pin);
}

void system_led_set_status(uint16_t status)
{
	if (system_pid.state == SYSTEM_CONTROL_STATE_OFF)
	{
		system_ticker_led_status.ms_max = status;
	}
}

void system_buffer_write(system_ring_buffer_t *buffer, uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		buffer->data[buffer->write_index] = data[i];
		buffer->write_index++;
	}
}

void system_write_cmd(system_ring_buffer_t *buffer, uint8_t cmd, uint8_t *payload, uint8_t length)
{
	system_index_init = buffer->write_index;

	buffer->data[buffer->write_index] = 'U';
	buffer->write_index++;

	buffer->data[buffer->write_index] = 'N';
	buffer->write_index++;

	buffer->data[buffer->write_index] = 'E';
	buffer->write_index++;

	buffer->data[buffer->write_index] = 'R';
	buffer->write_index++;

	buffer->data[buffer->write_index] = length;
	buffer->write_index++;

	buffer->data[buffer->write_index] = ':';
	buffer->write_index++;

	buffer->data[buffer->write_index] = cmd;
	buffer->write_index++;

	for (uint8_t i = 0 ; i < length ; i++)
	{
		buffer->data[buffer->write_index] = payload[i];
		buffer->write_index++;
	}

	buffer->data[buffer->write_index] =
			system_check_xor((uint8_t *)(buffer->data),
					system_index_init, 7 + length);
	buffer->write_index++;
}

uint8_t system_data_package(system_cmd_manager_t *cmd_manager)
{
	uint8_t result = 0;	// Indica si se termino de analizar el paquete o no

	switch (cmd_manager->read_state)
	{
		// Inicio de la cabecera
		case 0:
			if (cmd_manager->buffer_read->data[cmd_manager->buffer_read->read_index] == 'U')
			{
				cmd_manager->read_time_out->ms_count = 0;
				cmd_manager->read_time_out->active = TICKER_ACTIVE;

				cmd_manager->read_state = 1;
			}

			break;

		case 1:
			if (cmd_manager->buffer_read->data[cmd_manager->buffer_read->read_index] == 'N')
			{
				cmd_manager->read_state = 2;
			}

			else
			{
				cmd_manager->read_state = 0;
			}

			break;

		case 2:
			if (cmd_manager->buffer_read->data[cmd_manager->buffer_read->read_index] == 'E')
			{
				cmd_manager->read_state = 3;
			}

			else
			{
				cmd_manager->read_state = 0;
			}

			break;

		case 3:
			if (cmd_manager->buffer_read->data[cmd_manager->buffer_read->read_index] == 'R')
			{
				cmd_manager->read_state = 4;
			}

			else
			{
				cmd_manager->read_state = 0;
			}

			break;

		// Tamaño del payload
		case 4:
			cmd_manager->read_payload_length = cmd_manager->buffer_read->data[cmd_manager->buffer_read->read_index];

			cmd_manager->read_state = 5;

			break;

		// Byte de token
		case 5:
			if (cmd_manager->buffer_read->data[cmd_manager->buffer_read->read_index] == ':')
			{
				cmd_manager->read_state = 6;
			}

			else
			{
				cmd_manager->read_state = 0;
			}

			break;

		// Comando
		case 6:
			cmd_manager->read_payload_init = cmd_manager->buffer_read->read_index + 1;

			cmd_manager->read_state = 7;

			break;

		// Verificación de datos
		case 7:
			// Se espera que se termine de recibir todos los datos
			if (cmd_manager->buffer_read->read_index == (uint8_t)(cmd_manager->read_payload_init + cmd_manager->read_payload_length))
			{
				// Se comprueba la integridad de datos
				if (system_check_xor((uint8_t *)(cmd_manager->buffer_read->data),
						(uint8_t)(cmd_manager->read_payload_init - 7),
						(uint8_t)(cmd_manager->read_payload_length + 7))
						== cmd_manager->buffer_read->data[cmd_manager->buffer_read->read_index])
				{
					// Detengo el timeout
					cmd_manager->read_time_out->active = TICKER_NO_ACTIVE;

					// El estado se resetea
					cmd_manager->read_state = 0;

					// Se indica que se termino de analizar el paquete
					result = 1;

					// Analisis del comando recibido
					switch (cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init - 1)])
					{
						/*
						 * Comando para leer o setear la constante KP
						 *
						 */
						case 0xA0:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];
								system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 2)];

								system_ram_user.kp = system_byte_converter.u16[0];

								system_aux_ack = 0xFF;

								system_write_cmd(cmd_manager->buffer_write, 0xA0, &system_aux_ack, 1);
							}

							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0x00)
							{
								system_byte_converter.u8[1] = 0x00;

								system_byte_converter.u16[1] = system_ram_user.kp;

								system_write_cmd(cmd_manager->buffer_write, 0xA0, &system_byte_converter.u8[1], 3);
							}

							break;

						/*
						 * Comando para leer o setear la constante KD
						 *
						 */
						case 0xA1:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];
								system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 2)];

								system_ram_user.kd = system_byte_converter.u16[0];

								system_aux_ack = 0xFF;

								system_write_cmd(cmd_manager->buffer_write, 0xA1, &system_aux_ack, 1);
							}

							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0x00)
							{
								system_byte_converter.u8[1] = 0x00;

								system_byte_converter.u16[1] = system_ram_user.kd;

								system_write_cmd(cmd_manager->buffer_write, 0xA1, &system_byte_converter.u8[1], 3);
							}

							break;

						/*
						 * Comando para leer o setear la constante KI
						 *
						 */
						case 0xA2:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];
								system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 2)];

								system_ram_user.ki = system_byte_converter.u16[0];

								system_aux_ack = 0xFF;

								system_write_cmd(cmd_manager->buffer_write, 0xA2, &system_aux_ack, 1);
							}

							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0x00)
							{
								system_byte_converter.u8[1] = 0x00;

								system_byte_converter.u16[1] = system_ram_user.ki;

								system_write_cmd(cmd_manager->buffer_write, 0xA2, &system_byte_converter.u8[1], 3);
							}

							break;

						/*
						 * Comando que setea el modo de envio de datos del error a la PC
						 *
						 */
						case 0xA3:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								system_ticker_error_send_data.ms_max = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];

								if (system_ticker_error_send_data.ms_max < 150)
								{
									system_ticker_error_send_data.ms_max = 150;
								}

								system_ticker_error_send_data.active = TICKER_ACTIVE;

								system_error_buffer_send_data = cmd_manager->buffer_write;
							}

							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0x00)
							{
								system_ticker_error_send_data.active = TICKER_NO_ACTIVE;

								system_error_buffer_send_data = 0;
							}

							break;

						/*
						 * Comando que setea o lee los pesos de la ponderacion
						 *
						 */
						case 0xA4:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								system_ram_user.p0 = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];
								system_ram_user.p1 = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 2)];
								system_ram_user.p2 = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 3)];
								system_ram_user.p3 = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 4)];
								system_ram_user.p4 = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 5)];
								system_ram_user.p5 = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 6)];
							}

							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0x00)
							{
								system_write_cmd(cmd_manager->buffer_write, 0xA4, (uint8_t *)(&system_ram_user.p0), 6);
							}

							break;

						/*
						 * Comando que activa o desactiva el modo de control automatico
						 *
						 */
						case 0xAA:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								system_pid.state = SYSTEM_CONTROL_STATE_ON;

								system_control_buffer_send_data = cmd_manager->buffer_write;

								system_pid.p = 0;
								system_pid.d = 0;
								system_pid.i = 0;

								system_pid.vel_mot_der = 0;
								system_pid.vel_mot_izq = 0;
							}

							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0x00)
							{
								system_pid.state = SYSTEM_CONTROL_STATE_OFF;

								system_control_buffer_send_data = 0;
							}

							system_write_cmd(cmd_manager->buffer_write, 0xAA, (uint8_t *)(&cmd_manager->buffer_read->data[cmd_manager->read_payload_init]), 1);

							break;

						/*
						 * Comando que setea el modo de envio de datos del puerto ADC a la PC
						 *
						 */
						case 0xC0:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								system_ticker_adc_send_data.ms_max = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];

								if (system_ticker_adc_send_data.ms_max < 150)
								{
									system_ticker_adc_send_data.ms_max = 150;
								}

								system_ticker_adc_send_data.active = TICKER_ACTIVE;

								system_adc_buffer_send_data = cmd_manager->buffer_write;
							}

							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0x00)
							{
								system_ticker_adc_send_data.active = TICKER_NO_ACTIVE;

								system_adc_buffer_send_data = 0;
							}

							break;

						/*
						 * Comando que asigna el duty cycle al PWM
						 *
						 */
						case 0xC1:
							system_byte_converter.u8[0] = cmd_manager->buffer_read->data[cmd_manager->read_payload_init];
							system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];

							pwm_set_motor_der_speed(system_byte_converter.i16[0]);

							system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 2)];
							system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 3)];

							pwm_set_motor_izq_speed(system_byte_converter.i16[0]);

							system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 4)];
							system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 5)];

							if (system_byte_converter.u16[0] != 0)
							{
								pwm_set_stop_motor(system_byte_converter.u16[0]);
							}

							system_aux_ack = 0x00;

							system_write_cmd(cmd_manager->buffer_write, 0xC1, &system_aux_ack, 1);

							break;

						/*
						 * Comando que asigna la frecuencia al PWM
						 *
						 */
						case 0xC2:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];
								system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 2)];

								pwm_change_freq(system_byte_converter.u16[0]);
							}

							system_aux_ack = 0x00;

							system_write_cmd(cmd_manager->buffer_write, 0xC2, &system_aux_ack, 1);

							break;

						/*
						 * Comando que asigna el SSID
						 *
						 */
						case 0xD0:
							system_aux_ack = 0xFF;

							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] <= 30)
							{
								system_ram_user.ssid_length = cmd_manager->buffer_read->data[cmd_manager->read_payload_init];

								system_aux_i = 0;
								system_aux_j = cmd_manager->read_payload_init + 1;

								while (system_aux_i < system_ram_user.ssid_length)
								{
									system_ram_user.ssid[system_aux_i] = cmd_manager->buffer_read->data[system_aux_j];

									system_aux_i++;
									system_aux_j++;
								}

								system_aux_ack = 0x00;
							}

							system_write_cmd(cmd_manager->buffer_write, 0xD0, &system_aux_ack, 1);

							break;

						/*
						 * Comando que asigna el PSW
						 *
						 */
						case 0xD1:
							system_aux_ack = 0xFF;

							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] <= 30)
							{
								system_ram_user.psw_length = cmd_manager->buffer_read->data[cmd_manager->read_payload_init];

								system_aux_i = 0;
								system_aux_j = cmd_manager->read_payload_init + 1;

								while (system_aux_i < system_ram_user.psw_length)
								{
									system_ram_user.psw[system_aux_i] = cmd_manager->buffer_read->data[system_aux_j];

									system_aux_i++;
									system_aux_j++;
								}

								system_aux_ack = 0x00;
							}

							system_write_cmd(cmd_manager->buffer_write, 0xD1, &system_aux_ack, 1);

							break;

						/*
						 * Comando que asigna la IP del micro
						 *
						 */
						case 0xD2:
							system_aux_ack = 0xFF;

							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] <= 20)
							{
								system_ram_user.ip_mcu_length = cmd_manager->buffer_read->data[cmd_manager->read_payload_init];

								system_aux_i = 0;
								system_aux_j = cmd_manager->read_payload_init + 1;

								while (system_aux_i < system_ram_user.ip_mcu_length)
								{
									system_ram_user.ip_mcu[system_aux_i] = cmd_manager->buffer_read->data[system_aux_j];

									system_aux_i++;
									system_aux_j++;
								}

								system_aux_ack = 0x00;
							}

							system_write_cmd(cmd_manager->buffer_write, 0xD2, &system_aux_ack, 1);

							break;

						/*
						 * Comando que asigna la IP de la PC
						 *
						 */
						case 0xD3:	// Seteo de la ip del pc
							system_aux_ack = 0xFF;

							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] <= 20)
							{
								system_ram_user.ip_pc_length = cmd_manager->buffer_read->data[cmd_manager->read_payload_init];

								system_aux_i = 0;
								system_aux_j = cmd_manager->read_payload_init + 1;

								while (system_aux_i < system_ram_user.ip_pc_length)
								{
									system_ram_user.ip_pc[system_aux_i] = cmd_manager->buffer_read->data[system_aux_j];

									system_aux_i++;
									system_aux_j++;
								}

								system_aux_ack = 0x00;
							}

							system_write_cmd(cmd_manager->buffer_write, 0xD3, &system_aux_ack, 1);

							break;

						/*
						 * Comando que asigna el puerto UDP para la comunicacion
						 *
						 */
						case 0xD4:
							system_aux_ack = 0xFF;

							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] <= 10)
							{
								system_ram_user.port_length = cmd_manager->buffer_read->data[cmd_manager->read_payload_init];

								system_aux_i = 0;
								system_aux_j = cmd_manager->read_payload_init + 1;

								while (system_aux_i < system_ram_user.port_length)
								{
									system_ram_user.port[system_aux_i] = cmd_manager->buffer_read->data[system_aux_j];

									system_aux_i++;
									system_aux_j++;
								}

								system_aux_ack = 0x00;
							}

							system_write_cmd(cmd_manager->buffer_write, 0xD4, &system_aux_ack, 1);

							break;

						/*
						 * ¡¡CUIDADO ESTE COMANDO GRABA LOS DATOS DE LA RAM EN LA FLASH!!
						 *
						 */
						case 0xD5:
							system_aux_ack = 0xFF;

							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == 0xFF)
							{
								if (system_flash_save_data() == HAL_OK)
								{
									system_aux_ack = 0x00;
								}
							}

							system_write_cmd(cmd_manager->buffer_write, 0xD5, &system_aux_ack, 1);

							break;

						/*
						 * ALIVE
						 *
						 * Comando para verificar la conexión
						 *
						 */
						case 0xF0:
							system_write_cmd(cmd_manager->buffer_write, 0xF0, 0, 0);

							break;

						/*
						 * Comando que activa la depuración via USB
						 *
						 */
						case 0xF1:
							system_aux_ack = 0xFF;

							// Activo el modo de depuracion
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == SYSTEM_USB_DEBUG_ON)
							{
								system_usb_debug = SYSTEM_USB_DEBUG_ON;

								system_aux_ack = 0x00;
							}

							// Desactivo el modo de depuracion
							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == SYSTEM_USB_DEBUG_OFF)
							{
								system_usb_debug = SYSTEM_USB_DEBUG_OFF;

								system_aux_ack = 0x00;
							}

							system_write_cmd(cmd_manager->buffer_write, 0xF1, &system_aux_ack, 1);

							break;

						/*
						 * Comando para enviar comandos AT a la ESP
						 *
						 */
						case 0xF2:
							for (uint8_t i = 0 ; i < cmd_manager->read_payload_length ; i++)
							{
								system_buffer_write(&esp_buffer_write,
										(uint8_t *)(&cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + i)]), 1);
							}

							system_buffer_write(&esp_buffer_write, (uint8_t *)("\r\n"), 2);

							break;

						/*
						 * Comando para enviar datos a la ESP
						 *
						 */
						case 0xF3:
							for (uint8_t i = 0 ; i < cmd_manager->read_payload_length ; i++)
							{
								system_buffer_write(&esp_buffer_write,
										(uint8_t *)(&cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + i)]), 1);
							}

							break;

						/*
						 * Se recibio un comando que no es identificable
						 *
						 */
						default:
							system_aux_ack = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init - 1)];

							system_write_cmd(cmd_manager->buffer_write, 0xFF, &system_aux_ack, 1);
							break;
					}
				}

				// Corrupcion de datos al recibir
				else
				{

				}
			}

			break;
	}

	return result;
}

uint8_t system_check_xor(uint8_t *data, uint8_t init, uint8_t length)
{
	uint8_t val_xor = 0x00;

	for (uint8_t i = 0 ; i < length ; i++)
	{
		val_xor ^= data[(uint8_t)(init + i)];
	}

	return val_xor;
}

HAL_StatusTypeDef system_flash_save_data(void)
{
	HAL_StatusTypeDef flash_status = HAL_ERROR;

	// Verifica si la flash se encuentra bloqueada o no
	if (system_flash_enabled == SYSTEM_FLASH_SAVE_DATA_ENABLED)
	{
		system_flash_enabled = SYSTEM_FLASH_SAVE_DATA_DISABLED;	// Indica que se escribio en la flash y la desactiva
		system_ticker_flash_enable.ms_count = 0;	// Reinicia timer

		uint32_t memory_address = (uint32_t)(&system_flash_user);
		uint32_t page_error = 0;

		FLASH_EraseInitTypeDef flash_erase;

		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);

		flash_erase.TypeErase = FLASH_TYPEERASE_PAGES;
		flash_erase.PageAddress = memory_address;
		flash_erase.NbPages = 1;

		flash_status = HAL_FLASHEx_Erase(&flash_erase, &page_error);

		system_ram_user.checksum = system_flash_check_integrity(&system_ram_user);	// Calculo el checksum de los datos en RAM

		if (flash_status == HAL_OK)
		{
			for (uint16_t i = 0 ; i < 512 ; i++)
			{
				flash_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, memory_address,
						((uint16_t *)(&system_ram_user))[i]);

				if (flash_status != HAL_OK)
				{
					break;
				}

				memory_address += 2;
			}
		}

		HAL_FLASH_Lock();
	}

	return flash_status;
}

uint8_t system_flash_check_integrity(system_flash_data_t *flash_data)
{
	uint8_t checksum = 0;

	for (uint16_t i = 0 ; i < 1023 ; i++)
	{
		checksum ^= ((uint8_t *)(flash_data))[i];
	}

	return checksum;
}

void system_flash_enable(void)
{
	system_flash_enabled = SYSTEM_FLASH_SAVE_DATA_ENABLED;	// Habilita la escritura de la flash
}

void system_adc_send_data(void)
{
	if (system_adc_buffer_send_data != 0)
	{
		system_write_cmd(system_adc_buffer_send_data, 0xC0, (uint8_t *)(adc_buffer.mean), 12);
	}
}

void system_pid_control(void)
{
	/*
	 * Distancia optima a la pared es el centro, es decir la suma ponderada tiene valor 0
	 *
	 * Para el error se realiza un suma ponderada de los valores de los sensores
	 *
	 */
	system_pid.error_vel = system_pid.error;
	system_pid.error = adc_buffer.mean[0] * system_ram_user.p0 + adc_buffer.mean[1] * system_ram_user.p1 + adc_buffer.mean[2] * system_ram_user.p2
			+ adc_buffer.mean[3] * system_ram_user.p3 + adc_buffer.mean[4] * system_ram_user.p4 + adc_buffer.mean[5] * system_ram_user.p5;
	system_pid.error_vel = (system_pid.error - system_pid.error_vel) / 2;

	// Algoritmo de PID los saltos de las constantes con k = 1 son de saltos de a 100 como maximo
	// Limite proporcional
	system_byte_converter.i32 = (system_pid.error / 50) * system_ram_user.kp;

	if (system_byte_converter.i32 > 7000)
	{
		system_pid.p = 7000;
	}

	else if (system_byte_converter.i32 < -7000)
	{
		system_pid.p = -7000;
	}

	else
	{
		system_pid.p = (int16_t)(system_byte_converter.i32);
	}

	// Limite derivativo
	system_byte_converter.i32 = (system_pid.error_vel / 50) * system_ram_user.kd;

	if (system_byte_converter.i32 > 5000)
	{
		system_pid.d = 5000;
	}

	else if (system_byte_converter.i32 < -5000)
	{
		system_pid.d = -5000;
	}

	else
	{
		system_pid.d = (int16_t)(system_byte_converter.i32);
	}

	// Limite integral
	system_byte_converter.i32 = system_pid.i + (system_pid.error / 100) * system_ram_user.ki;

	if (system_byte_converter.i32 > 3000)
	{
		system_pid.i = 3000;
	}

	else if (system_byte_converter.i32 < -3000)
	{
		system_pid.i = -3000;
	}

	else
	{
		system_pid.i = (int16_t)(system_byte_converter.i32);
	}

	if (system_pid.state == SYSTEM_CONTROL_STATE_ON)
	{
		// Motor de la derecha
		system_byte_converter.i32 = SYSTEM_CONTROL_BASE_SPEED + system_pid.p + system_pid.i + system_pid.d;

		if (system_byte_converter.i32 > SYSTEM_CONTROL_MAX_SPEED)
		{
			system_pid.vel_mot_der = SYSTEM_CONTROL_MAX_SPEED;
		}

		else if (system_byte_converter.i32 < -SYSTEM_CONTROL_MAX_SPEED)
		{
			system_pid.vel_mot_der = -SYSTEM_CONTROL_MAX_SPEED;
		}

		else
		{
			system_pid.vel_mot_der = (int16_t)(system_byte_converter.i32);
		}

		// Motor de la izquierda
		system_byte_converter.i32 = -SYSTEM_CONTROL_BASE_SPEED + system_pid.p + system_pid.i + system_pid.d;

		if (system_byte_converter.i32 > SYSTEM_CONTROL_MAX_SPEED)
		{
			system_pid.vel_mot_izq = SYSTEM_CONTROL_MAX_SPEED;
		}

		else if (system_byte_converter.i32 < -SYSTEM_CONTROL_MAX_SPEED)
		{
			system_pid.vel_mot_izq = -SYSTEM_CONTROL_MAX_SPEED;
		}

		else
		{
			system_pid.vel_mot_izq = (int16_t)(system_byte_converter.i32);
		}

		// Si se detecta una pared se detiene
		if (adc_buffer.mean[1] >= 1500 && adc_buffer.mean[4] >= 1500)
		{
			system_pid.vel_mot_der = 0;
			system_pid.vel_mot_izq = 0;
		}

		// Se le da la velocidad a los motores
		pwm_set_motor_der_speed(system_pid.vel_mot_der);
		pwm_set_motor_izq_speed(system_pid.vel_mot_izq);

		// Se da un timeout por si paso algo
		pwm_set_stop_motor(200);
	}

	else
	{
		system_pid.vel_mot_der = pwm_get_motor_der_speed();
		system_pid.vel_mot_izq = pwm_get_motor_izq_speed();
	}
}

void system_error_send_data(void)
{
	if (system_error_buffer_send_data != 0)
	{
		system_write_cmd(system_error_buffer_send_data, 0xA3, (uint8_t *)(&system_pid), sizeof(system_pid_t));
	}
}

void system_control_state_button(void)
{
	system_ticker_control_set.ms_max = 1;

	if (HAL_GPIO_ReadPin(SYSTEM_PID_GPIO_Port, SYSTEM_PID_Pin) == GPIO_PIN_RESET)
	{
		system_ticker_control_set.ms_max = 500;	// Antibounce

		if (system_pid.state == SYSTEM_CONTROL_STATE_OFF)
		{
			system_pid.state = SYSTEM_CONTROL_STATE_ON;

			system_pid.p = 0;
			system_pid.d = 0;
			system_pid.i = 0;

			system_pid.vel_mot_der = 0;
			system_pid.vel_mot_izq = 0;
		}

		else
		{
			system_pid.state = SYSTEM_CONTROL_STATE_OFF;
		}

		if (system_control_buffer_send_data != 0)
		{
			system_write_cmd(system_control_buffer_send_data, 0xAA, &system_pid.state, 1);
		}
	}
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
