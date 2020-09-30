#include "esp8266.h"

// Variables
extern __attribute__ ((__section__(".user_data_flash"))) flash_data_t flash_user;
extern flash_data_t flash_user_ram;

esp_buffer_read_t esp_buffer_read;
esp_buffer_write_t esp_buffer_write;

esp_buffer_write_t esp_buffer_cmd_write;

esp_manager_t esp_manager;

uint8_t request;

uint8_t i;
uint8_t j;

// Byte temporal de recepción de datos
volatile uint8_t byte_receibe_usart;

void esp_init(void)
{
	// Inicializacion de los buffers
	esp_buffer_read.read_index = 0;
	esp_buffer_read.write_index = 0;
	esp_buffer_read.read_state = 0;

	esp_buffer_write.read_index = 0;
	esp_buffer_write.write_index = 0;

	esp_buffer_cmd_write.read_index = 0;
	esp_buffer_cmd_write.write_index = 0;

	// Inicializacion del esp manager
	esp_manager.read_state = 0;
	esp_manager.cmd = ESP_COMMAND_IDLE;
	esp_manager.status = ESP_STATUS_NO_INIT;

	ticker_new(esp_connect_to_ap, 200, TICKER_LOW_PRIORITY);
}

void esp_write_buffer_write(uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		esp_buffer_write.data[esp_buffer_write.write_index] = data[i];
		esp_buffer_write.write_index++;
	}
}

void esp_write_buffer_send_data_write(uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		esp_buffer_cmd_write.data[esp_buffer_cmd_write.write_index] = data[i];
		esp_buffer_cmd_write.write_index++;
	}
}

void esp_write_buffer_read(uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		esp_buffer_read.data[esp_buffer_read.write_index] = data[i];
		esp_buffer_read.write_index++;
	}
}

void esp_send_at(uint8_t *cmd, uint8_t length)
{
	// Cabecera AT
	esp_write_buffer_write(cmd, length);
	esp_write_buffer_write((uint8_t *)("\r\n"), 2);
}

void esp_send_cmd(uint8_t cmd, uint8_t *payload, uint8_t length)
{
	// Cabecera UNER
	esp_write_buffer_send_data_write((uint8_t *)("UNER"), 4);
	esp_write_buffer_send_data_write(&length, 1);
	esp_write_buffer_send_data_write((uint8_t *)(":"), 1);
	esp_write_buffer_send_data_write(&cmd, 1);
	esp_write_buffer_send_data_write(payload, length);

	uint8_t checksum = xor(cmd, payload, 0, length);

	esp_write_buffer_send_data_write(&checksum, 1);
}

void esp_read_pending(void)
{
	if (esp_buffer_read.read_index != esp_buffer_read.write_index)
	{
		switch (esp_manager.read_state)
		{
			case 0:
				if (esp_buffer_read.data[esp_buffer_read.read_index] != '\r' &&
						esp_buffer_read.data[esp_buffer_read.read_index] != '\n')
				{
					ticker_new(esp_timeout, 200, TICKER_LOW_PRIORITY);

					esp_manager.cmd_init = esp_buffer_read.read_index;

					esp_manager.read_state = 1;
				}

				break;

			case 1:
				if (esp_buffer_read.data[esp_buffer_read.read_index] == '\r' ||
						esp_buffer_read.data[esp_buffer_read.read_index] == '\n')
				{
					ticker_delete(esp_timeout);

					esp_manager.cmd_end = esp_buffer_read.read_index - 1;

					esp_manager.read_state = 2;
				}

				break;

			case 2:
				if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT"), 2))
				{
					esp_manager.cmd = ESP_COMMAND_AT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT+CWMODE_CUR=1"), 15))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CWMODE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 12, (uint8_t *)("AT+CWJAP_CUR="), 13))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CWJAP;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("+CWJAP:1"), 8))
				{
					esp_manager.status = ESP_STATUS_ERROR_CON_TIMEOUT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("+CWJAP:2"), 8))
				{
					esp_manager.status = ESP_STATUS_ERROR_CON_PSW;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("+CWJAP:3"), 8))
				{
					esp_manager.status = ESP_STATUS_ERROR_CON_NO_AP;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("+CWJAP:4"), 8))
				{
					esp_manager.status = ESP_STATUS_ERROR_CON_FAIL;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI CONNECTED"), 14))
				{
					esp_manager.status = ESP_STATUS_CONNECTED;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI GOT IP"), 11))
				{
					esp_manager.status = ESP_STATUS_CONNECTED_GOT_IP;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI DISCONNECT"), 15))
				{
					esp_manager.status = ESP_STATUS_DISCONNECTED;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 13, (uint8_t *)("AT+CIPSTA_CUR="), 14))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSTA;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT+CIPSTATUS"), 12))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSTATUS;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("STATUS:2"), 8))
				{
					esp_manager.status = ESP_STATUS_CONNECTED_GOT_IP;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("STATUS:3"), 8))
				{
					esp_manager.status = ESP_STATUS_UDP_READY;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("STATUS:5"), 8))
				{
					esp_manager.status = ESP_STATUS_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT+CIPCLOSE"), 11))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPCLOSE;

					esp_manager.status = ESP_STATUS_SET_IP;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 11, (uint8_t *)("AT+CIPSTART="), 12))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSTART;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 10, (uint8_t *)("AT+CIPSEND="), 11))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSEND;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("OK"), 2))
				{
					switch (esp_manager.cmd)
					{
						case ESP_COMMAND_AT:
							esp_manager.status = ESP_STATUS_INIT;

							break;

						case ESP_COMMAND_AT_CWMODE:
							esp_manager.status = ESP_STATUS_STATION_OK;

							break;

						case ESP_COMMAND_AT_CWJAP:

							break;

						case ESP_COMMAND_AT_CIPSTA:
							esp_manager.status = ESP_STATUS_SET_IP;

							break;

						case ESP_COMMAND_AT_CIPSTATUS:

							break;

						case ESP_COMMAND_AT_CIPCLOSE:

							break;

						case ESP_COMMAND_AT_CIPSTART:
							esp_manager.status = ESP_STATUS_UDP_READY;

							break;

						case ESP_COMMAND_AT_CIPSEND:
							esp_manager.status = ESP_STATUS_READY_SEND;

							break;
					}

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("ERROR"), 5))
				{
					switch (esp_manager.cmd)
					{
						case ESP_COMMAND_AT:
							esp_manager.status = ESP_STATUS_ERROR_INIT;

							break;

						case ESP_COMMAND_AT_CWMODE:
							esp_manager.status = ESP_STATUS_ERROR_CWMODE;

							break;

						case ESP_COMMAND_AT_CIPSTA:
							esp_manager.status = ESP_STATUS_ERROR_CIPSTA;

							break;

						case ESP_COMMAND_AT_CIPSTATUS:

							break;

						case ESP_COMMAND_AT_CIPCLOSE:

							break;

						case ESP_COMMAND_AT_CIPSTART:
							esp_manager.status = ESP_STATUS_ERROR_UDP;

							break;

						case ESP_COMMAND_AT_CIPSEND:
							esp_manager.status = ESP_STATUS_ERROR_CMD_SEND;

							break;
					}

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("FAIL"), 4))
				{
					switch (esp_manager.cmd)
					{
						case ESP_COMMAND_AT_CWJAP:

							break;
					}

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("ALREADY CONNECTED"), 17))
				{
					switch (esp_manager.cmd)
					{
						case ESP_COMMAND_AT_CIPSTART:
							esp_manager.status = ESP_STATUS_UDP_READY;

							break;
					}

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("SEND OK"), 7))
				{
					esp_manager.status = ESP_STATUS_UDP_READY;

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("SEND FAIL"), 9))
				{
					esp_manager.status = ESP_STATUS_ERROR_SEND_DATA;

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 4, (uint8_t *)("+IPD,"), 5))
				{
					char len_char[4] = {'\0'};
					uint8_t len_uint;

					i = esp_manager.cmd_init + 5;
					j = 0;

					while ((esp_buffer_read.data[i] != ':') && (j < 3))
					{
						len_char[j] = esp_buffer_read.data[i];

						i++;
						j++;
					}

					len_uint = (uint8_t)(atoi(len_char));

					esp_buffer_read.scan_index = i + 1;
					esp_buffer_read.read_state = 0;

					while (esp_buffer_read.scan_index != (i + len_uint + 1))
					{
						switch (esp_buffer_read.read_state)
						{
							case 0:	// Inicio de la abecera
								if (esp_buffer_read.data[esp_buffer_read.scan_index] == 'U')
								{
									esp_buffer_read.read_state = 1;

									//ticker_new(usbcdc_timeout, 200, TICKER_HIGH_PRIORITY);
								}

								break;

							case 1:
								if (esp_buffer_read.data[esp_buffer_read.scan_index] == 'N')
								{
									esp_buffer_read.read_state = 2;
								}

								else
								{
									esp_buffer_read.read_state = 0;
								}

								break;

							case 2:
								if (esp_buffer_read.data[esp_buffer_read.scan_index] == 'E')
								{
									esp_buffer_read.read_state = 3;
								}

								else
								{
									esp_buffer_read.read_state = 0;
								}

								break;

							case 3:
								if (esp_buffer_read.data[esp_buffer_read.scan_index] == 'R')
								{
									esp_buffer_read.read_state = 4;
								}

								else
								{
									esp_buffer_read.read_state = 0;
								}

								break;

							case 4:	// Lee el tamaño del payload
								esp_buffer_read.payload_length = esp_buffer_read.data[esp_buffer_read.scan_index];

								esp_buffer_read.read_state = 5;

								break;

							case 5:	// Token
								if (esp_buffer_read.data[esp_buffer_read.scan_index] == ':')
								{
									esp_buffer_read.read_state = 6;
								}

								else
								{
									esp_buffer_read.read_state = 0;
								}

								break;

							case 6:	// Comando
								esp_buffer_read.payload_init = esp_buffer_read.scan_index + 1;

								esp_buffer_read.read_state = 7;

								break;

							case 7:	// Verificación de datos
								// Si se terminaron de recibir todos los datos
								if (esp_buffer_read.scan_index == (esp_buffer_read.payload_init + esp_buffer_read.payload_length))
								{
									// Se comprueba la integridad de datos
									if (xor(esp_buffer_read.data[esp_buffer_read.payload_init - 1], (uint8_t *)(esp_buffer_read.data),
											esp_buffer_read.payload_init, esp_buffer_read.payload_length)
											== esp_buffer_read.data[esp_buffer_read.scan_index])
									{
										// Analisis del comando recibido
										switch (esp_buffer_read.data[esp_buffer_read.payload_init - 1])
										{
											case 0xF0:  // ALIVE
												esp_send_cmd(0xF0, NULL, 0x00);

												break;

											default:	// Comando no valido
												esp_send_cmd(0xFF, (uint8_t *)(&esp_buffer_read.data[esp_buffer_read.payload_init - 1]), 0x01);

												break;
										}
									}

									// Corrupcion de datos al recibir
									else
									{

									}

									// Detengo el timeout
									//ticker_delete(usbcdc_timeout);

									esp_buffer_read.read_state = 0;
								}

								break;
						}

						esp_buffer_read.scan_index++;
					}
				}

				esp_manager.read_state = 0;

				break;
		}

		esp_buffer_read.read_index++;
	}
}

__attribute__((weak)) void esp_write_pending(void)
{

}

void esp_write_send_data_pending(void)
{
	if (esp_buffer_cmd_write.read_index != esp_buffer_cmd_write.write_index)
	{
		switch (esp_manager.status)
		{
			case ESP_STATUS_UDP_READY:
				if (esp_buffer_cmd_write.read_index < esp_buffer_cmd_write.write_index)
				{
					esp_manager.send_data_length = esp_buffer_cmd_write.write_index - esp_buffer_cmd_write.read_index;
				}

				else
				{
					esp_manager.send_data_length = 256 - esp_buffer_cmd_write.read_index + esp_buffer_cmd_write.write_index + 1;
				}

				esp_write_buffer_write((uint8_t *)("AT+CIPSEND="), 11);

				char len_char[4];

				uint8_t len_uint = sprintf(len_char, "%u", esp_manager.send_data_length);

				esp_write_buffer_write((uint8_t *)(len_char), len_uint);

				esp_write_buffer_write((uint8_t *)("\r\n"), 2);

				esp_manager.status = ESP_STATUS_WAIT_SENDING;
				break;

			case ESP_STATUS_READY_SEND:
				while ((esp_manager.send_data_length > 0) && (esp_buffer_cmd_write.read_index != esp_buffer_cmd_write.write_index))
				{
					esp_write_buffer_write((uint8_t *)(&esp_buffer_cmd_write.data[esp_buffer_cmd_write.read_index]), 1);
					esp_buffer_cmd_write.read_index++;
					esp_manager.send_data_length--;
				}

				esp_manager.status = ESP_STATUS_WAIT_SENDING;

				break;
		}
	}
}

uint8_t esp_at_cmp(uint8_t *at, uint8_t at_init, uint8_t at_end, uint8_t *at_cmp, uint8_t at_cmp_length)
{
	uint8_t i = at_init;
	uint8_t j = 0;

	if (((at_init > at_end) && (at_cmp_length < (256 - at_init + at_end + 1))) || ((at_cmp_length < (at_end + 1 - at_init))))
	{
		return 0;
	}

	while (i != (at_end + 1))
	{
		if (at[i] != at_cmp[j])
		{
			return 0;
		}

		i++;
		j++;
	}

	return 1;
}

void esp_timeout(void)
{
	ticker_delete(esp_timeout);

	esp_manager.read_state = 0;
}

void esp_connect_to_ap(void)
{
	if (esp_manager.cmd == ESP_COMMAND_IDLE)
	{
		switch (esp_manager.status)
		{
			case ESP_STATUS_NO_INIT:
				esp_send_at((uint8_t *)("AT"), 2);

				break;

			case ESP_STATUS_INIT:
				esp_send_at((uint8_t *)("AT+CWMODE_CUR=1"), 15);

				break;

			case ESP_STATUS_STATION_OK:
				esp_write_buffer_write((uint8_t *)("AT+CWJAP_CUR=\""), 14);
				esp_write_buffer_write(flash_user_ram.ssid, flash_user_ram.ssid_length);
				esp_write_buffer_write((uint8_t *)("\",\""), 3);
				esp_write_buffer_write(flash_user_ram.psw, flash_user_ram.psw_length);
				esp_write_buffer_write((uint8_t *)("\""), 1);
				esp_write_buffer_write((uint8_t *)("\r\n"), 2);

				break;

			case ESP_STATUS_CONNECTED:

				break;

			case ESP_STATUS_CONNECTED_GOT_IP:
				esp_write_buffer_write((uint8_t *)("AT+CIPSTA_CUR=\""), 15);
				esp_write_buffer_write(flash_user_ram.ip_mcu, flash_user_ram.ip_mcu_length);
				esp_write_buffer_write((uint8_t *)("\""), 1);
				esp_write_buffer_write((uint8_t *)("\r\n"), 2);

				break;

			case ESP_STATUS_SET_IP:
				esp_write_buffer_write((uint8_t *)("AT+CIPSTART=\"UDP\",\""), 19);
				esp_write_buffer_write(flash_user_ram.ip_pc, flash_user_ram.ip_pc_length);
				esp_write_buffer_write((uint8_t *)("\","), 2);
				esp_write_buffer_write(flash_user_ram.port, flash_user_ram.port_length);
				esp_write_buffer_write((uint8_t *)(","), 1);
				esp_write_buffer_write(flash_user_ram.port, flash_user_ram.port_length);
				esp_write_buffer_write((uint8_t *)("\r\n"), 2);

				break;

			case ESP_STATUS_UDP_READY:
				ticker_delete(esp_connect_to_ap);

				break;
		}
	}

	if (ticker_calls(esp_connect_to_ap) > 60)
	{
		ticker_delete(esp_connect_to_ap);
	}
}
