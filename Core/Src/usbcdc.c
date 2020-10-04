#include "usbcdc.h"

// Variables
extern flash_data_t flash_user_ram;

usbcdc_buffer_read_t usbcdc_buffer_read;
usbcdc_buffer_write_t usbcdc_buffer_write;

extern uint8_t debug;

extern adc_buffer_t adc_buffer;

uint8_t request;

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

	uint8_t checksum = xor(cmd, payload, 0, length);

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

					ticker_new(usbcdc_timeout, 200, TICKER_HIGH_PRIORITY);
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
					if (xor(usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init - 1], (uint8_t *)(usbcdc_buffer_read.data),
							usbcdc_buffer_read.payload_init, usbcdc_buffer_read.payload_length)
							== usbcdc_buffer_read.data[usbcdc_buffer_read.read_index])
					{
						// Analisis del comando recibido
						switch (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init - 1])
						{
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

								request = 0x00;

								usbcdc_send_cmd(0xD0, &request, 0x01);

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

								request = 0x00;

								usbcdc_send_cmd(0xD1, &request, 0x01);

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

								request = 0x00;

								usbcdc_send_cmd(0xD2, &request, 0x01);

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

								request = 0x00;

								usbcdc_send_cmd(0xD3, &request, 0x01);

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

								request = 0x00;

								usbcdc_send_cmd(0xD4, &request, 0x01);

								break;

							case 0xD5:	// Graba los parametros en ram en la flash
								request = 0x00;

								if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init] == 0xFF)
								{
									request = save_flash_data();
								}

								if (request == HAL_OK)
								{
									request = 0x00;
								}

								else
								{
									request = 0xFF;
								}

								usbcdc_send_cmd(0xD5, &request, 0x01);

								break;

							case 0xF0:  // ALIVE
								usbcdc_send_cmd(0xF0, NULL, 0x00);

								break;

							case 0xF1:	// Modo debug
								if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init] == 0xFF)
								{
									debug = DEBUG_ON;

									request = 0x00;

									usbcdc_send_cmd(0xF1, &request, 0x01);
								}

								else if (usbcdc_buffer_read.data[usbcdc_buffer_read.payload_init] == 0x00)
								{
									debug = DEBUG_OFF;

									request = 0x00;

									usbcdc_send_cmd(0xF1, &request, 0x01);
								}

								else
								{
									request = 0xFF;

									usbcdc_send_cmd(0xF1, &request, 0x01);
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
					ticker_delete(usbcdc_timeout);

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

void usbcdc_timeout(void)
{
	ticker_delete(usbcdc_timeout);

	usbcdc_buffer_read.read_state = 0;
}
