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
/**********************************************************************************/

/***************************** Depuracion via USB *********************************/
uint8_t system_usb_debug;
/**********************************************************************************/

/**************************** Variables auxiliares ********************************/
uint8_t system_index_init;	// Sirve para guardar el indice de inicio para calcular el checksum
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

	/*************************** Configuracion de prueba *******************************/
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

	system_flash_enabled = SYSTEM_FLASH_SAVE_DATA_ENABLED;	// Se habilita la flash para poder escribir

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

	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	/***********************************************************************************/
	/************************** Inicializacion de los modulos **************************/
	/***********************************************************************************/
	esp_init();	// Inicia la configuracion del ESP
	usbcdc_init();	// Inicia la configuracion del USB
	adc_init();	// Inicia la configuracion del ADC
	pwm_init();	// Inicia la configuracion del PWM
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/
}

void system_led_status(void)
{
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

void system_buffer_write(system_ring_buffer_t *buffer, uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		buffer->data[buffer->write_index] = data[i];
		buffer->write_index++;
	}
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
			if (cmd_manager->buffer_read->read_index == (cmd_manager->read_payload_init + cmd_manager->read_payload_length))
			{
				// Se comprueba la integridad de datos
				if (system_check_xor((uint8_t *)(cmd_manager->buffer_read->data),
						(uint8_t)(cmd_manager->read_payload_init - 7),
						(uint8_t)(cmd_manager->read_payload_length + 8))
						== cmd_manager->buffer_read->data[cmd_manager->buffer_read->read_index])
				{
					// Analisis del comando recibido
					switch (cmd_manager->buffer_read->data[cmd_manager->read_payload_init - 1])
					{
						/*case 0xC0:	// Modo de envio de datos a la pc
							if (esp_buffer_read.buffer.data[esp_buffer_read.payload_init] == ADC_SEND_DATA_ON)
							{
								ticker_esp_send_adc_sensor_data.ms_max = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 1];

								if (ticker_esp_send_adc_sensor_data.ms_max < 150)
								{
									ticker_esp_send_adc_sensor_data.ms_max = 150;
								}

								if (adc_buffer.send_esp == ADC_SEND_DATA_OFF)
								{
									adc_buffer.send_esp = ADC_SEND_DATA_ON;

									ticker_esp_send_adc_sensor_data.active = TICKER_ACTIVE;
								}
							}

							else if (esp_buffer_read.buffer.data[esp_buffer_read.payload_init] == ADC_SEND_DATA_OFF)
							{
								adc_buffer.send_esp = ADC_SEND_DATA_OFF;

								ticker_esp_send_adc_sensor_data.active = TICKER_NO_ACTIVE;
							}

							break;

						case 0xC1:
							byte_translate.u8[0] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init];
							byte_translate.u8[1] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 1];
							byte_translate.u8[2] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 2];
							byte_translate.u8[3] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 3];

							pwm_set_motor_der_speed(byte_translate.f);

							byte_translate.u8[0] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 4];
							byte_translate.u8[1] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 5];
							byte_translate.u8[2] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 6];
							byte_translate.u8[3] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 7];

							pwm_set_motor_izq_speed(byte_translate.f);

							byte_translate.u8[0] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 8];
							byte_translate.u8[1] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 9];

							if (byte_translate.u16[0] != 0)
							{
								pwm_set_stop_motor(byte_translate.u16[0]);
							}

							ack = 0x00;

							esp_send_cmd(0xC2, &ack, 1);

							break;

						case 0xC2:
							if (esp_buffer_read.buffer.data[esp_buffer_read.payload_init] == 0xFF)
							{
								byte_translate.u8[0] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 1];
								byte_translate.u8[1] = esp_buffer_read.buffer.data[esp_buffer_read.payload_init + 2];

								pwm_change_freq(byte_translate.u16[0]);
							}

							esp_send_cmd(0xC2, (uint8_t *)(&esp_buffer_read.buffer.data[esp_buffer_read.payload_init]), 3);

							break;

						case 0xC3:	// Continuar nivel de bateria
							if (esp_buffer_read.data[esp_buffer_read.payload_init] == ADC_SEND_DATA_ON)
							{
								ticker_esp_send_adc_batery_data.ms_max = esp_buffer_read.data[esp_buffer_read.payload_init + 1];

								if (ticker_esp_send_adc_batery_data.ms_max < 2000)
								{
									ticker_esp_send_adc_batery_data.ms_max = 2000;
								}

								if (adc_buffer.send_batery_esp == ADC_SEND_DATA_OFF)
								{
									adc_buffer.send_batery_esp = ADC_SEND_DATA_ON;

									ticker_esp_send_adc_batery_data.active = TICKER_ACTIVE;
								}
							}

							else if (esp_buffer_read.data[esp_buffer_read.payload_init] == ADC_SEND_DATA_OFF)
							{
								adc_buffer.send_batery_esp = ADC_SEND_DATA_OFF;

								ticker_esp_send_adc_batery_data.active = TICKER_DEACTIVATE;
							}

							break;

						case 0xD0:	// Seteo de ssid
							flash_user_ram.ssid_length = esp_buffer_read.buffer.data[esp_buffer_read.payload_init];

							i = 0;
							j = esp_buffer_read.payload_init + 1;

							while (i < flash_user_ram.ssid_length)
							{
								flash_user_ram.ssid[i] = esp_buffer_read.buffer.data[j];

								i++;
								j++;
							}

							ack = 0x00;

							esp_send_cmd(0xD0, &ack, 0x01);

							break;

						case 0xD1:	// Seteo de psw
							flash_user_ram.psw_length = esp_buffer_read.buffer.data[esp_buffer_read.payload_init];

							i = 0;
							j = esp_buffer_read.payload_init + 1;

							while (i < flash_user_ram.psw_length)
							{
								flash_user_ram.psw[i] = esp_buffer_read.buffer.data[j];

								i++;
								j++;
							}

							ack = 0x00;

							esp_send_cmd(0xD1, &ack, 0x01);

							break;

						case 0xD2:	// Seteo de la ip del micro
							flash_user_ram.ip_mcu_length = esp_buffer_read.buffer.data[esp_buffer_read.payload_init];

							i = 0;
							j = esp_buffer_read.payload_init + 1;

							while (i < flash_user_ram.ip_mcu_length)
							{
								flash_user_ram.ip_mcu[i] = esp_buffer_read.buffer.data[j];

								i++;
								j++;
							}

							ack = 0x00;

							esp_send_cmd(0xD2, &ack, 0x01);

							break;

						case 0xD3:	// Seteo de la ip del pc
							flash_user_ram.ip_pc_length = esp_buffer_read.buffer.data[esp_buffer_read.payload_init];

							i = 0;
							j = esp_buffer_read.payload_init + 1;

							while (i < flash_user_ram.ip_pc_length)
							{
								flash_user_ram.ip_pc[i] = esp_buffer_read.buffer.data[j];

								i++;
								j++;
							}

							ack = 0x00;

							esp_send_cmd(0xD3, &ack, 0x01);

							break;

						case 0xD4:	// Seteo del puerto UDP
							flash_user_ram.port_length = esp_buffer_read.buffer.data[esp_buffer_read.payload_init];

							i = 0;
							j = esp_buffer_read.payload_init + 1;

							while (i < flash_user_ram.port_length)
							{
								flash_user_ram.port[i] = esp_buffer_read.buffer.data[j];

								i++;
								j++;
							}

							ack = 0x00;

							esp_send_cmd(0xD4, &ack, 0x01);

							break;

						case 0xD5:	// Graba los parametros en ram en la flash
							ack = 0xFF;

							if (esp_buffer_read.buffer.data[esp_buffer_read.payload_init] == 0xFF)
							{
								if (save_flash_data() == HAL_OK)
								{
									ack = 0x00;
								}
							}

							esp_send_cmd(0xD5, &ack, 0x01);

							break;*/

						/*
						 * ALIVE
						 *
						 * Comando para verificar la conexión
						 *
						 */
						case 0xF0:
							system_index_init = cmd_manager->buffer_write->write_index;

							cmd_manager->buffer_write->data[cmd_manager->buffer_write->write_index++] = 'U';
							cmd_manager->buffer_write->data[cmd_manager->buffer_write->write_index++] = 'N';
							cmd_manager->buffer_write->data[cmd_manager->buffer_write->write_index++] = 'E';
							cmd_manager->buffer_write->data[cmd_manager->buffer_write->write_index++] = 'R';
							cmd_manager->buffer_write->data[cmd_manager->buffer_write->write_index++] = 0;
							cmd_manager->buffer_write->data[cmd_manager->buffer_write->write_index++] = ':';
							cmd_manager->buffer_write->data[cmd_manager->buffer_write->write_index++] = 0xF0;
							cmd_manager->buffer_write->data[cmd_manager->buffer_write->write_index++] =
									system_check_xor((uint8_t *)(cmd_manager->buffer_write->data), system_index_init, 7);

							break;

						/*case 0xF2:	// Envio de comando AT
							for (uint8_t i = 0 ; i < esp_buffer_read.payload_length ; i++)
							{
								esp_write_buffer_write((uint8_t *)(&esp_buffer_read.buffer.data[esp_buffer_read.payload_init + i]), 1);
							}

							esp_write_buffer_write((uint8_t *)("\r\n"), 2);

							break;

						case 0xF3:	// Envio de datos
							for (uint8_t i = 0 ; i < esp_buffer_read.payload_length ; i++)
							{
								esp_write_buffer_write((uint8_t *)(&esp_buffer_read.buffer.data[esp_buffer_read.payload_init + i]), 1);
							}

							break;

						default:	// Comando no valido
							esp_send_cmd(0xFF, (uint8_t *)(&esp_buffer_read.buffer.data[esp_buffer_read.payload_init - 1]), 0x01);

							break;*/
					}
				}

				// Corrupcion de datos al recibir
				else
				{

				}

				// Detengo el timeout
				cmd_manager->read_time_out.active = TICKER_NO_ACTIVE;

				// El estado se resetea
				cmd_manager->read_state = 0;

				// Se indica que se termino de analizar el paquete
				result = 1;
			}

			break;
	}

	return result;
}

uint8_t system_check_xor(uint8_t *data, uint8_t init, uint8_t length)
{
	uint8_t val_xor = 0x00;

	for (uint8_t i = init ; i < (uint8_t)(init + length) ; i++)
	{
		val_xor ^= data[i];
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
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
