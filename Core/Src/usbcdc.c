#include "usbcdc.h"

// Variables
// Tickers
ticker_t ticker_usbcdc_read_timeout;

ticker_t ticker_usbcdc_send_adc_data;

// Datos guardados
extern flash_data_t flash_user_ram;

// Bufferes de datos
usbcdc_buffer_read_t usbcdc_buffer_read;
usbcdc_buffer_write_t usbcdc_buffer_write;

extern adc_buffer_t adc_buffer;

// Flag de depuracion via USB
extern uint8_t esp_to_usb_debug;

// Variable de conversion de datos
extern byte_translate_u byte_translate;

// Variables auxiliares
uint8_t ack;

uint8_t i;
uint8_t j;

void usbcdc_init(void)
{
	// Inicializacion de los buffers
	usbcdc_buffer_read.read_index = 0;
	usbcdc_buffer_read.write_index = 0;
	usbcdc_buffer_read.read_state = 0;

	usbcdc_buffer_write.read_index = 0;
	usbcdc_buffer_write.write_index = 0;

	// Inicializacion del ticker de read timeout
	ticker_usbcdc_read_timeout.ms_count = 0;
	ticker_usbcdc_read_timeout.ms_max = 100;
	ticker_usbcdc_read_timeout.calls = 0;
	ticker_usbcdc_read_timeout.priority = TICKER_LOW_PRIORITY;
	ticker_usbcdc_read_timeout.ticker_function = usbcdc_read_timeout;
	ticker_usbcdc_read_timeout.active = TICKER_DEACTIVATE;

	ticker_new(&ticker_usbcdc_read_timeout);

	// Inicializacion del ticker para envio de datos del adc
	ticker_usbcdc_send_adc_data.ms_count = 0;
	ticker_usbcdc_send_adc_data.ms_max = 500;
	ticker_usbcdc_send_adc_data.calls = 0;
	ticker_usbcdc_send_adc_data.priority = TICKER_LOW_PRIORITY;
	ticker_usbcdc_send_adc_data.ticker_function = usbcdc_send_adc_data;
	ticker_usbcdc_send_adc_data.active = TICKER_DEACTIVATE;

	ticker_new(&ticker_usbcdc_send_adc_data);
}

void usbcdc_write_buffer_write(uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		usbcdc_buffer_write.data[usbcdc_buffer_write.write_index] = data[i];
		usbcdc_buffer_write.write_index++;
	}
}

void usbcdc_write_buffer_read(uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		usbcdc_buffer_read.data[usbcdc_buffer_read.write_index] = data[i];
		usbcdc_buffer_read.write_index++;
	}
}

void usbcdc_send_cmd(uint8_t cmd, uint8_t *payload, uint8_t length)
{
	// Cabecera UNER
	usbcdc_write_buffer_write((uint8_t *)("UNER"), 4);
	usbcdc_write_buffer_write(&length, 1);
	usbcdc_write_buffer_write((uint8_t *)(":"), 1);
	usbcdc_write_buffer_write(&cmd, 1);
	usbcdc_write_buffer_write(payload, length);

	uint8_t checksum = check_xor(cmd, payload, 0, length);

	usbcdc_write_buffer_write(&checksum, 1);
}

void usbcdc_read_pending(void)
{
	if (usbcdc_buffer_read.read_index != usbcdc_buffer_read.write_index)
	{
		switch (usbcdc_buffer_read.read_state)
		{
			case 0:	// Inicio de la abecera
				if (usbcdc_buffer_read.data[usbcdc_buffer_read.read_index] == 'U')
				{
					usbcdc_buffer_read.read_state = 1;

					ticker_usbcdc_read_timeout.ms_count = 0;
					ticker_usbcdc_read_timeout.active = TICKER_ACTIVE;
				}

				break;

			case 1:
				if (usbcdc_buffer_read.data[usbcdc_buffer_read.read_index] == 'N')
				{
					usbcdc_buffer_read.read_state = 2;
				}

				else
				{
					usbcdc_buffer_read.read_state = 0;
				}

				break;

			case 2:
				if (usbcdc_buffer_read.data[usbcdc_buffer_read.read_index] == 'E')
				{
					usbcdc_buffer_read.read_state = 3;
				}

				else
				{
					usbcdc_buffer_read.read_state = 0;
				}

				break;

			case 3:
				if (usbcdc_buffer_read.data[usbcdc_buffer_read.read_index] == 'R')
				{
					usbcdc_buffer_read.read_state = 4;
				}

				else
				{
					usbcdc_buffer_read.read_state = 0;
				}

				break;

			case 4:	// Lee el tamaño del payload
				usbcdc_buffer_read.payload_length = usbcdc_buffer_read.data[usbcdc_buffer_read.read_index];

				usbcdc_buffer_read.read_state = 5;

				break;

			case 5:	// Token
				if (usbcdc_buffer_read.data[usbcdc_buffer_read.read_index] == ':')
				{
					usbcdc_buffer_read.read_state = 6;
				}

				else
				{
					usbcdc_buffer_read.read_state = 0;
				}

				break;

			case 6:	// Comando
				usbcdc_buffer_read.payload_init = usbcdc_buffer_read.read_index + 1;

				usbcdc_buffer_read.read_state = 7;

				break;

			case 7:	// Verificación de datos
				// Si se terminaron de recibir todos los datos
				if (usbcdc_buffer_read.read_index == (usbcdc_buffer_read.payload_init + usbcdc_buffer_read.payload_length))
				{
					// Se comprueba la integridad de datos
					if (check_xor(usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init - 1], (uint8_t *)(usbcdc_buffer_read.data),
							usbcdc_buffer_read.payload_init, usbcdc_buffer_read.payload_length)
							== usbcdc_buffer_read.data[usbcdc_buffer_read.read_index])
					{
						// Analisis del comando recibido
						switch (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init - 1])
						{
							case 0xC0:	// Modo de envio de datos a la pc
								if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init] == ADC_SEND_DATA_ON)
								{
									if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init + 1] >= 50)
									{
										if (adc_buffer.send_usb == ADC_SEND_DATA_OFF)
										{
											adc_buffer.send_usb = ADC_SEND_DATA_ON;

											ticker_usbcdc_send_adc_data.ms_max = usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init + 1];
											ticker_usbcdc_send_adc_data.active = TICKER_ACTIVE;
										}

										else if (adc_buffer.send_usb == ADC_SEND_DATA_ON)
										{
											ticker_usbcdc_send_adc_data.ms_max = usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init + 1];
										}
									}
								}

								else if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init] == ADC_SEND_DATA_OFF)
								{
									adc_buffer.send_usb = ADC_SEND_DATA_OFF;

									ticker_usbcdc_send_adc_data.active = TICKER_DEACTIVATE;
								}

								break;

							case 0xD0:	// Seteo de ssid
								flash_user_ram.ssid_length = usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init];

								i = 0;
								j = usbcdc_buffer_read.payload_init + 1;

								while (i < flash_user_ram.ssid_length)
								{
									flash_user_ram.ssid[i] = usbcdc_buffer_read.data[j];

									i++;
									j++;
								}

								ack = 0x00;

								usbcdc_send_cmd(0xD0, &ack, 0x01);

								break;

							case 0xD1:	// Seteo de psw
								flash_user_ram.psw_length = usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init];

								i = 0;
								j = usbcdc_buffer_read.payload_init + 1;

								while (i < flash_user_ram.psw_length)
								{
									flash_user_ram.psw[i] = usbcdc_buffer_read.data[j];

									i++;
									j++;
								}

								ack = 0x00;

								usbcdc_send_cmd(0xD1, &ack, 0x01);

								break;

							case 0xD2:	// Seteo de la ip del micro
								flash_user_ram.ip_mcu_length = usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init];

								i = 0;
								j = usbcdc_buffer_read.payload_init + 1;

								while (i < flash_user_ram.ip_mcu_length)
								{
									flash_user_ram.ip_mcu[i] = usbcdc_buffer_read.data[j];

									i++;
									j++;
								}

								ack = 0x00;

								usbcdc_send_cmd(0xD2, &ack, 0x01);

								break;

							case 0xD3:	// Seteo de la ip del pc
								flash_user_ram.ip_pc_length = usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init];

								i = 0;
								j = usbcdc_buffer_read.payload_init + 1;

								while (i < flash_user_ram.ip_pc_length)
								{
									flash_user_ram.ip_pc[i] = usbcdc_buffer_read.data[j];

									i++;
									j++;
								}

								ack = 0x00;

								usbcdc_send_cmd(0xD3, &ack, 0x01);

								break;

							case 0xD4:	// Seteo del puerto UDP
								flash_user_ram.port_length = usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init];

								i = 0;
								j = usbcdc_buffer_read.payload_init + 1;

								while (i < flash_user_ram.port_length)
								{
									flash_user_ram.port[i] = usbcdc_buffer_read.data[j];

									i++;
									j++;
								}

								ack = 0x00;

								usbcdc_send_cmd(0xD4, &ack, 0x01);

								break;

							case 0xD5:	// Graba los parametros en ram en la flash
								ack = 0xFF;

								if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init] == 0xFF)
								{
									if (save_flash_data() == HAL_OK)
									{
										ack = 0x00;
									}
								}

								usbcdc_send_cmd(0xD5, &ack, 0x01);

								break;

							case 0xF0:  // ALIVE
								usbcdc_send_cmd(0xF0, 0, 0x00);

								break;

							case 0xF1:	// Modo debug
								if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init] == 0xFF)
								{
									esp_to_usb_debug = DEBUG_ON;

									ack = 0x00;

									usbcdc_send_cmd(0xF1, &ack, 0x01);
								}

								else if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init] == 0x00)
								{
									esp_to_usb_debug = DEBUG_OFF;

									ack = 0x00;

									usbcdc_send_cmd(0xF1, &ack, 0x01);
								}

								else
								{
									ack = 0xFF;

									usbcdc_send_cmd(0xF1, &ack, 0x01);
								}

								break;

							case 0xF2:	// Envio de comando AT
								for (uint8_t i = 0 ; i < usbcdc_buffer_read.payload_length ; i++)
								{
									esp_write_buffer_write((uint8_t *)(&usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init + i]), 1);
								}

								esp_write_buffer_write((uint8_t *)("\r\n"), 2);

								break;

							case 0xF3:	// Envio de datos
								for (uint8_t i = 0 ; i < usbcdc_buffer_read.payload_length ; i++)
								{
									esp_write_buffer_write((uint8_t *)(&usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init + i]), 1);
								}

								break;

							default:	// Comando no valido
								usbcdc_send_cmd(0xFF, (uint8_t *)(&usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init - 1]), 0x01);

								break;
						}
					}

					// Corrupcion de datos al recibir
					else
					{

					}

					// Detengo el timeout
					ticker_usbcdc_read_timeout.active = TICKER_DEACTIVATE;

					usbcdc_buffer_read.read_state = 0;
				}

				break;
		}

		usbcdc_buffer_read.read_index++;
	}
}

void usbcdc_write_pending(void)
{
	if (usbcdc_buffer_write.read_index != usbcdc_buffer_write.write_index)
	{
		if (CDC_Transmit_FS((uint8_t *)(&usbcdc_buffer_write.data[usbcdc_buffer_write.read_index]), 1) == USBD_OK)
		{
			usbcdc_buffer_write.read_index++;
		}
	}
}

void usbcdc_read_timeout(void)
{
	ticker_usbcdc_read_timeout.active = TICKER_DEACTIVATE;

	usbcdc_buffer_read.read_state = 0;
}

void usbcdc_send_adc_data(void)
{
	// Calculo la media de los datos almacenados en el buffer
	for (uint8_t i = 0 ; i < 6 ; i++)
	{
		adc_buffer.mean[i] = 0;

		for (uint8_t j = 0 ; j < ADC_BUFFER_LENGTH ; j += ADC_MEAN_STEP)
		{
			adc_buffer.mean[i] += adc_buffer.data[j][i];
		}

		adc_buffer.mean[i] = (uint16_t)(adc_buffer.mean[i] / (ADC_BUFFER_LENGTH / ADC_MEAN_STEP));
	}

	uint8_t init_index;

	usbcdc_write_buffer_write((uint8_t *)("UNER"), 4);

	ack = 13;
	usbcdc_write_buffer_write(&ack, 1);

	usbcdc_write_buffer_write((uint8_t *)(":"), 1);

	ack = 0xC0;
	usbcdc_write_buffer_write(&ack, 1);

	init_index = usbcdc_buffer_write.write_index;

	usbcdc_write_buffer_write(&adc_buffer.send_esp, 1);

	for (uint8_t i = 0 ; i < 6 ; i++)
	{
		byte_translate.u16[0] = adc_buffer.mean[i];

		usbcdc_write_buffer_write((uint8_t *)(&byte_translate.u8[0]), 1);
		usbcdc_write_buffer_write((uint8_t *)(&byte_translate.u8[1]), 1);
	}

	uint8_t checksum = check_xor(ack, (uint8_t *)(&usbcdc_buffer_write.data), init_index, 13);

	usbcdc_write_buffer_write(&checksum, 1);
}
