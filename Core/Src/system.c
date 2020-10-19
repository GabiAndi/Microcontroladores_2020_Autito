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
/**********************************************************************************/

/***************************** Bufferes de datos **********************************/
extern system_ring_buffer_t esp_buffer_write;	// Buffer de la ESP

extern adc_buffer_t adc_buffer;

system_ring_buffer_t *system_adc_buffer_send_data;
/**********************************************************************************/

/***************************** Depuracion via USB *********************************/
uint8_t system_usb_debug;
/**********************************************************************************/

/**************************** Conversion de datos *********************************/
system_byte_converter_u system_byte_converter;
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
	/************************* Inicializacion de los bufferes **************************/
	/***********************************************************************************/
	system_adc_buffer_send_data = 0;
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
	HAL_GPIO_TogglePin(SYSTEM_LED_GPIO_Port, SYSTEM_LED_Pin);
}

void system_led_set_status(uint16_t status)
{
	system_ticker_led_status.ms_max = status;
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
						 * Comando que setea el modo de envio de datos a la PC
						 *
						 */
						case 0xC0:
							if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == SYSTEM_ADC_SEND_DATA_ON)
							{
								system_ticker_adc_send_data.ms_max = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];

								if (system_ticker_adc_send_data.ms_max < 50)
								{
									system_ticker_adc_send_data.ms_max = 50;
								}

								system_ticker_adc_send_data.active = TICKER_ACTIVE;

								system_adc_buffer_send_data = cmd_manager->buffer_write;
							}

							else if (cmd_manager->buffer_read->data[cmd_manager->read_payload_init] == SYSTEM_ADC_SEND_DATA_OFF)
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
							system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init)];
							system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 1)];
							system_byte_converter.u8[2] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 2)];
							system_byte_converter.u8[3] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 3)];

							pwm_set_motor_der_speed(system_byte_converter.f);

							system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 4)];
							system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 5)];
							system_byte_converter.u8[2] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 6)];
							system_byte_converter.u8[3] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 7)];

							pwm_set_motor_izq_speed(system_byte_converter.f);

							system_byte_converter.u8[0] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 8)];
							system_byte_converter.u8[1] = cmd_manager->buffer_read->data[(uint8_t)(cmd_manager->read_payload_init + 9)];

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
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
