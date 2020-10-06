#include "esp8266.h"

// Variables
// Tickers
ticker_t ticker_esp_timeout_read;
ticker_t ticker_esp_timeout_send;

ticker_t ticker_esp_connect_to_ap;

ticker_t ticker_esp_hard_reset;

ticker_t ticker_esp_send_adc_data;

// Datos guardados
extern flash_data_t flash_user_ram;

// UART de la HAL
extern UART_HandleTypeDef huart3;

// Bufferes de datos
esp_buffer_read_t esp_buffer_read;
esp_buffer_write_t esp_buffer_write;
esp_buffer_write_t esp_buffer_cmd_write;

extern adc_buffer_t adc_buffer;

// Byte temporal de recepción de datos
volatile uint8_t byte_receibe_usart;

// Control de la ESP
esp_manager_t esp_manager;

// Flag de depuracion via USB
extern uint8_t esp_to_usb_debug;

// Variable de conversion de datos
extern byte_translate_u byte_translate;

// Variables auxiliares
uint8_t ack;

uint8_t i;
uint8_t j;

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
	esp_manager.status = ESP_STATUS_NO_INIT;
	esp_manager.station = ESP_STATION_NO_INIT;
	esp_manager.error = ESP_ERROR_OK;
	esp_manager.connected = ESP_DISCONNECTED;
	esp_manager.udp = ESP_UDP_NO_INIT;
	esp_manager.send = ESP_SEND_NO_INIT;

	esp_manager.read_state = 0;

	esp_manager.cmd = ESP_COMMAND_IDLE;

	esp_manager.cmd_init = 0;
	esp_manager.cmd_end = 0;

	esp_manager.cmd_index = 0;

	esp_manager.send_data_length = 0;

	esp_manager.auto_connection = 0;

	HAL_UART_Receive_IT(&huart3, (uint8_t *)(&byte_receibe_usart), 1);

	// Inicializacion del ticker de read timeout
	ticker_esp_timeout_read.ms_count = 0;
	ticker_esp_timeout_read.ms_max = 200;
	ticker_esp_timeout_read.calls = 0;
	ticker_esp_timeout_read.priority = TICKER_LOW_PRIORITY;
	ticker_esp_timeout_read.ticker_function = esp_timeout_read;
	ticker_esp_timeout_read.active = TICKER_DEACTIVATE;

	ticker_new(&ticker_esp_timeout_read);

	// Inicializacion del ticker de send timeout
	ticker_esp_timeout_send.ms_count = 0;
	ticker_esp_timeout_send.ms_max = 200;
	ticker_esp_timeout_send.calls = 0;
	ticker_esp_timeout_send.priority = TICKER_LOW_PRIORITY;
	ticker_esp_timeout_send.ticker_function = esp_timeout_send;
	ticker_esp_timeout_send.active = TICKER_DEACTIVATE;

	ticker_new(&ticker_esp_timeout_send);

	// Inicializacion del ticker de autoconectado
	ticker_esp_connect_to_ap.ms_count = 0;
	ticker_esp_connect_to_ap.ms_max = 200;
	ticker_esp_connect_to_ap.calls = 0;
	ticker_esp_connect_to_ap.priority = TICKER_LOW_PRIORITY;
	ticker_esp_connect_to_ap.ticker_function = esp_connect_to_ap;
	ticker_esp_connect_to_ap.active = TICKER_ACTIVE;

	ticker_new(&ticker_esp_connect_to_ap);

	// Inicializacion del ticker de hard reset
	ticker_esp_hard_reset.ms_count = 0;
	ticker_esp_hard_reset.ms_max = 200;
	ticker_esp_hard_reset.calls = 0;
	ticker_esp_hard_reset.priority = TICKER_LOW_PRIORITY;
	ticker_esp_hard_reset.ticker_function = esp_hard_reset;
	ticker_esp_hard_reset.active = TICKER_DEACTIVATE;

	ticker_new(&ticker_esp_hard_reset);

	// Inicializacion del ticker para envio de datos del adc
	ticker_esp_send_adc_data.ms_count = 0;
	ticker_esp_send_adc_data.ms_max = 500;
	ticker_esp_send_adc_data.calls = 0;
	ticker_esp_send_adc_data.priority = TICKER_LOW_PRIORITY;
	ticker_esp_send_adc_data.ticker_function = esp_send_adc_data;
	ticker_esp_send_adc_data.active = TICKER_DEACTIVATE;

	ticker_new(&ticker_esp_send_adc_data);
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
				if ((esp_buffer_read.data[esp_buffer_read.read_index] != '\r') &&
						(esp_buffer_read.data[esp_buffer_read.read_index] != '\n'))
				{
					ticker_esp_timeout_read.ms_count = 0;
					ticker_esp_timeout_read.active = TICKER_ACTIVE;

					esp_manager.cmd_init = esp_buffer_read.read_index;

					if ((esp_buffer_read.data[esp_buffer_read.read_index] == '>'))
					{
						esp_manager.read_state = 3;
					}

					else
					{
						esp_manager.read_state = 1;
					}
				}

				break;

			case 1:
				if ((esp_buffer_read.data[esp_buffer_read.read_index] == '\r') ||
						(esp_buffer_read.data[esp_buffer_read.read_index] == '\n'))
				{
					ticker_esp_timeout_read.active = TICKER_DEACTIVATE;

					esp_manager.cmd_end = esp_buffer_read.read_index - 1;

					esp_manager.read_state = 2;
				}

				break;

			case 2:
				esp_manager.read_state = 0;

				if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT"), 2))
				{
					esp_manager.cmd = ESP_COMMAND_AT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 13, (uint8_t *)("AT+CWMODE_CUR="), 14))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CWMODE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 12, (uint8_t *)("AT+CWJAP_CUR="), 13))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CWJAP;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("+CWJAP:1"), 8))
				{
					esp_manager.error = ESP_ERROR_CON_TIMEOUT;
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("+CWJAP:2"), 8))
				{
					esp_manager.error = ESP_ERROR_CON_PSW;
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("+CWJAP:3"), 8))
				{
					esp_manager.error = ESP_ERROR_CON_NO_AP;
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("+CWJAP:4"), 8))
				{
					esp_manager.error = ESP_ERROR_CON_FAIL;
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI CONNECTED"), 14))
				{
					esp_manager.connected = ESP_CONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI GOT IP"), 11))
				{
					esp_manager.connected = ESP_CONNECTED_GOT_IP;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI DISCONNECT"), 15))
				{
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
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
					esp_manager.connected = ESP_CONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("STATUS:3"), 8))
				{
					esp_manager.connected = ESP_CONNECTED;
					esp_manager.udp = ESP_UDP_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("STATUS:5"), 8))
				{
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT+CIPCLOSE"), 11))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPCLOSE;
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
							esp_manager.station = ESP_STATION_INIT;

							break;

						case ESP_COMMAND_AT_CIPSTA:
							esp_manager.connected = ESP_CONNECTED_SET_IP;

							break;

						case ESP_COMMAND_AT_CIPCLOSE:
							esp_manager.udp = ESP_UDP_NO_INIT;

							break;

						case ESP_COMMAND_AT_CIPSTART:
							esp_manager.udp = ESP_UDP_INIT;

							break;
					}

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("ERROR"), 5))
				{
					switch (esp_manager.cmd)
					{
						case ESP_COMMAND_AT:
							esp_manager.error = ESP_ERROR_INIT;

							break;

						case ESP_COMMAND_AT_CWMODE:
							esp_manager.error = ESP_ERROR_CWMODE;

							break;

						case ESP_COMMAND_AT_CIPSTA:
							esp_manager.error = ESP_ERROR_CIPSTA;

							break;

						case ESP_COMMAND_AT_CIPCLOSE:
							esp_manager.error = ESP_ERROR_UDP_CLOSE;
							esp_manager.udp = ESP_UDP_NO_INIT;

							break;

						case ESP_COMMAND_AT_CIPSTART:
							esp_manager.error = ESP_ERROR_UDP;
							esp_manager.udp = ESP_UDP_NO_INIT;

							break;

						case ESP_COMMAND_AT_CIPSEND:
							esp_manager.error = ESP_ERROR_CMD_SEND;
							esp_manager.send = ESP_SEND_NO_INIT;

							break;
					}

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("ALREADY CONNECTED"), 17))
				{
					switch (esp_manager.cmd)
					{
						case ESP_COMMAND_AT_CIPSTART:
							esp_manager.udp = ESP_UDP_INIT;

							break;
					}

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("SEND OK"), 7))
				{
					esp_manager.send = ESP_SEND_OK;

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("SEND FAIL"), 9))
				{
					esp_manager.send = ESP_SEND_NO_INIT;
					esp_manager.error = ESP_ERROR_SEND_DATA;

					esp_manager.cmd = ESP_COMMAND_IDLE;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 4, (uint8_t *)("+IPD,"), 5))
				{
					memset(esp_manager.len_char, '\0', 4);

					i = esp_manager.cmd_init + 5;
					j = 0;

					while ((esp_buffer_read.data[i] != ':') && (j < 3))
					{
						esp_manager.len_char[j] = esp_buffer_read.data[i];

						i++;
						j++;
					}

					esp_manager.len_uint = (uint8_t)(atoi(esp_manager.len_char));

					esp_manager.cmd_index = i + 1;
					esp_buffer_read.read_state = 0;

					while (esp_manager.cmd_index != (uint8_t)(i + esp_manager.len_uint - 1))
					{
						switch (esp_buffer_read.read_state)
						{
							case 0:	// Inicio de la abecera
								if (esp_buffer_read.data[esp_manager.cmd_index] == 'U')
								{
									esp_buffer_read.read_state = 1;

									ticker_esp_timeout_read.ms_count = 0;
									ticker_esp_timeout_read.active = TICKER_ACTIVE;
								}

								break;

							case 1:
								if (esp_buffer_read.data[esp_manager.cmd_index] == 'N')
								{
									esp_buffer_read.read_state = 2;
								}

								else
								{
									esp_buffer_read.read_state = 0;
								}

								break;

							case 2:
								if (esp_buffer_read.data[esp_manager.cmd_index] == 'E')
								{
									esp_buffer_read.read_state = 3;
								}

								else
								{
									esp_buffer_read.read_state = 0;
								}

								break;

							case 3:
								if (esp_buffer_read.data[esp_manager.cmd_index] == 'R')
								{
									esp_buffer_read.read_state = 4;
								}

								else
								{
									esp_buffer_read.read_state = 0;
								}

								break;

							case 4:	// Lee el tamaño del payload
								esp_buffer_read.payload_length = esp_buffer_read.data[esp_manager.cmd_index];

								esp_buffer_read.read_state = 5;

								break;

							case 5:	// Token
								if (esp_buffer_read.data[esp_manager.cmd_index] == ':')
								{
									esp_buffer_read.read_state = 6;
								}

								else
								{
									esp_buffer_read.read_state = 0;
								}

								break;

							case 6:	// Comando
								esp_buffer_read.payload_init = esp_manager.cmd_index + 1;

								esp_buffer_read.read_state = 7;

								break;

							case 7:	// Verificación de datos
								// Si se terminaron de recibir todos los datos
								if (esp_manager.cmd_index == (esp_buffer_read.payload_init + esp_buffer_read.payload_length))
								{
									// Se comprueba la integridad de datos
									if (xor(esp_buffer_read.data[esp_buffer_read.payload_init - 1], (uint8_t *)(esp_buffer_read.data),
											esp_buffer_read.payload_init, esp_buffer_read.payload_length)
											== esp_buffer_read.data[esp_manager.cmd_index])
									{
										// Analisis del comando recibido
										switch (esp_buffer_read.data[esp_buffer_read.payload_init - 1])
										{
											case 0xC0:	// Modo de envio de datos a la pc
												if (esp_buffer_read.data[esp_buffer_read.payload_init] == ADC_SEND_DATA_ON)
												{
													if (esp_buffer_read.data[esp_buffer_read.payload_init + 1] >= 50)
													{
														if (adc_buffer.send_esp == ADC_SEND_DATA_OFF)
														{
															adc_buffer.send_esp = ADC_SEND_DATA_ON;

															ticker_esp_send_adc_data.ms_max = esp_buffer_read.data[esp_buffer_read.payload_init + 1];
															ticker_esp_send_adc_data.active = TICKER_ACTIVE;
														}

														else if (adc_buffer.send_esp == ADC_SEND_DATA_ON)
														{
															ticker_esp_send_adc_data.ms_max = esp_buffer_read.data[esp_buffer_read.payload_init + 1];
														}
													}
												}

												else if (esp_buffer_read.data[esp_buffer_read.payload_init] == ADC_SEND_DATA_OFF)
												{
													adc_buffer.send_esp = ADC_SEND_DATA_OFF;

													ticker_esp_send_adc_data.active = TICKER_DEACTIVATE;
												}

												break;

											case 0xF0:  // ALIVE
												esp_send_cmd(0xF0, 0, 0x00);

												break;

											case 0xF2:	// Envio de comando AT
												for (uint8_t i = 0 ; i < esp_buffer_read.payload_length ; i++)
												{
													esp_write_buffer_write((uint8_t *)(&esp_buffer_read.data[esp_buffer_read.payload_init + i]), 1);
												}

												esp_write_buffer_write((uint8_t *)("\r\n"), 2);

												break;

											case 0xF3:	// Envio de datos
												for (uint8_t i = 0 ; i < esp_buffer_read.payload_length ; i++)
												{
													esp_write_buffer_write((uint8_t *)(&esp_buffer_read.data[esp_buffer_read.payload_init + i]), 1);
												}

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
									ticker_esp_timeout_read.active = TICKER_DEACTIVATE;

									esp_buffer_read.read_state = 0;
									esp_manager.read_state = 0;
								}

								break;
						}

						esp_manager.cmd_index++;
					}

					esp_manager.read_state = 0;
				}

				break;

			case 3:
				if ((esp_buffer_read.data[esp_buffer_read.read_index] == ' '))
				{
					ticker_esp_timeout_read.active = TICKER_DEACTIVATE;

					esp_manager.send = ESP_SEND_READY;

					esp_manager.read_state = 0;
				}

				break;
		}

		esp_buffer_read.read_index++;
	}
}

void esp_write_pending(void)
{
	if (esp_buffer_write.read_index != esp_buffer_write.write_index)
	{
		if (HAL_UART_Transmit_IT(&huart3, (uint8_t *)(&esp_buffer_write.data[esp_buffer_write.read_index]), 1) == HAL_OK)
		{
			esp_buffer_write.read_index++;
		}
	}
}

void esp_write_send_data_pending(void)
{
	switch (esp_manager.send)
	{
		case ESP_SEND_NO_INIT:
			if ((esp_buffer_cmd_write.read_index != esp_buffer_cmd_write.write_index) && (esp_manager.status == ESP_STATUS_INIT)
					&& (esp_manager.udp == ESP_UDP_INIT) && (esp_manager.connected == ESP_CONNECTED_SET_IP)
					&& (esp_manager.error == ESP_ERROR_OK))
			{
				if (esp_buffer_cmd_write.read_index < esp_buffer_cmd_write.write_index)
				{
					esp_manager.send_data_length = esp_buffer_cmd_write.write_index - esp_buffer_cmd_write.read_index;
				}

				else
				{
					esp_manager.send_data_length = 256 - esp_buffer_cmd_write.read_index + esp_buffer_cmd_write.write_index;
				}

				esp_write_buffer_write((uint8_t *)("AT+CIPSEND="), 11);

				memset(esp_manager.len_char, '\0', 4);

				esp_manager.len_uint = sprintf(esp_manager.len_char, "%u", esp_manager.send_data_length);

				esp_write_buffer_write((uint8_t *)(esp_manager.len_char), esp_manager.len_uint);

				esp_write_buffer_write((uint8_t *)("\r\n"), 2);

				ticker_esp_timeout_send.ms_count = 0;
				ticker_esp_timeout_send.active = TICKER_ACTIVE;

				esp_manager.send = ESP_SEND_WAITING_OK;
			}

			break;

		case ESP_SEND_READY:
			while (esp_manager.send_data_length > 0)
			{
				esp_write_buffer_write((uint8_t *)(&esp_buffer_cmd_write.data[esp_buffer_cmd_write.read_index]), 1);
				esp_buffer_cmd_write.read_index++;
				esp_manager.send_data_length--;
			}

			esp_manager.send = ESP_SEND_SENDING;

			break;

		case ESP_SEND_OK:
			ticker_esp_timeout_send.active = TICKER_DEACTIVATE;

			esp_manager.send = ESP_SEND_NO_INIT;

			break;
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

void esp_timeout_read(void)
{
	ticker_esp_timeout_read.active = TICKER_DEACTIVATE;

	esp_manager.read_state = 0;
	esp_buffer_read.read_state = 0;
	esp_manager.cmd_index = 0;
}

void esp_timeout_send(void)
{
	ticker_esp_timeout_send.active = TICKER_DEACTIVATE;

	esp_manager.send = ESP_SEND_NO_INIT;
	esp_manager.error = ESP_ERROR_SEND_DATA;
}

void esp_connect_to_ap(void)
{
	if (esp_manager.cmd == ESP_COMMAND_IDLE)
	{
		switch (esp_manager.auto_connection)
		{
			case 0:
				esp_write_buffer_write((uint8_t *)("AT\r\n"), 4);

				esp_manager.auto_connection = 1;

				break;

			case 1:
				if (esp_manager.status == ESP_STATUS_INIT)
				{
					esp_manager.auto_connection = 2;
				}

				break;

			case 2:
				esp_write_buffer_write((uint8_t *)("AT+CWMODE_CUR=1\r\n"), 17);

				esp_manager.auto_connection = 3;

				break;

			case 3:
				if (esp_manager.station == ESP_STATION_INIT)
				{
					esp_manager.auto_connection = 4;
				}

				break;

			case 4:
				esp_write_buffer_write((uint8_t *)("AT+CWJAP_CUR=\""), 14);
				esp_write_buffer_write(flash_user_ram.ssid, flash_user_ram.ssid_length);
				esp_write_buffer_write((uint8_t *)("\",\""), 3);
				esp_write_buffer_write(flash_user_ram.psw, flash_user_ram.psw_length);
				esp_write_buffer_write((uint8_t *)("\""), 1);
				esp_write_buffer_write((uint8_t *)("\r\n"), 2);

				esp_manager.auto_connection = 5;

				break;

			case 5:
				if (esp_manager.connected == ESP_CONNECTED_GOT_IP)
				{
					esp_manager.auto_connection = 6;
				}

				break;

			case 6:
				esp_write_buffer_write((uint8_t *)("AT+CIPSTA_CUR=\""), 15);
				esp_write_buffer_write(flash_user_ram.ip_mcu, flash_user_ram.ip_mcu_length);
				esp_write_buffer_write((uint8_t *)("\""), 1);
				esp_write_buffer_write((uint8_t *)("\r\n"), 2);

				esp_manager.auto_connection = 7;

				break;

			case 7:
				if (esp_manager.connected == ESP_CONNECTED_SET_IP)
				{
					esp_manager.auto_connection = 8;
				}

				break;

			case 8:
				esp_write_buffer_write((uint8_t *)("AT+CIPSTART=\"UDP\",\""), 19);
				esp_write_buffer_write(flash_user_ram.ip_pc, flash_user_ram.ip_pc_length);
				esp_write_buffer_write((uint8_t *)("\","), 2);
				esp_write_buffer_write(flash_user_ram.port, flash_user_ram.port_length);
				esp_write_buffer_write((uint8_t *)(","), 1);
				esp_write_buffer_write(flash_user_ram.port, flash_user_ram.port_length);
				esp_write_buffer_write((uint8_t *)("\r\n"), 2);

				esp_manager.auto_connection = 9;

				break;

			case 9:
				if (esp_manager.udp == ESP_UDP_INIT)
				{
					ticker_esp_connect_to_ap.active = TICKER_DEACTIVATE;

					esp_manager.auto_connection = 0;
				}

				break;
		}
	}

	if (ticker_esp_connect_to_ap.calls > 100)
	{
		ticker_esp_connect_to_ap.active = TICKER_DEACTIVATE;
	}
}

void esp_hard_reset(void)
{
	HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_RESET);

	// Inicializacion del esp manager
	esp_manager.status = ESP_STATUS_NO_INIT;
	esp_manager.station = ESP_STATION_NO_INIT;
	esp_manager.error = ESP_ERROR_OK;
	esp_manager.connected = ESP_DISCONNECTED;
	esp_manager.udp = ESP_UDP_NO_INIT;
	esp_manager.send = ESP_SEND_NO_INIT;

	esp_manager.read_state = 0;

	esp_manager.cmd = ESP_COMMAND_IDLE;

	esp_manager.cmd_init = 0;
	esp_manager.cmd_end = 0;

	esp_manager.cmd_index = 0;

	esp_manager.send_data_length = 0;

	esp_manager.auto_connection = 0;

	ticker_esp_hard_reset.ms_count = 0;
	ticker_esp_hard_reset.active = TICKER_ACTIVE;
}

void esp_hard_reset_stop(void)
{
	HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_SET);

	ticker_esp_hard_reset.active = TICKER_DEACTIVATE;

	ticker_esp_connect_to_ap.ms_count = 0;
	ticker_esp_connect_to_ap.active = TICKER_ACTIVE;
}

void esp_guardian_status(void)
{
	// Manejo de errores
	switch (esp_manager.error)
	{
		case ESP_ERROR_SEND_DATA:	// Error al enviar un dato
			esp_manager.error = ESP_ERROR_OK;

			break;
	}
}

void esp_send_adc_data(void)
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

	uint8_t cmd_index;

	esp_write_buffer_send_data_write((uint8_t *)("UNER"), 4);

	ack = 13;
	esp_write_buffer_send_data_write(&ack, 1);

	esp_write_buffer_send_data_write((uint8_t *)(":"), 1);

	cmd_index = esp_buffer_cmd_write.write_index;

	ack = 0xC0;
	esp_write_buffer_send_data_write(&ack, 1);

	esp_write_buffer_send_data_write(&adc_buffer.send_esp, 1);

	for (uint8_t i = 0 ; i < 6 ; i++)
	{
		byte_translate.u16[0] = adc_buffer.mean[i];

		esp_write_buffer_send_data_write((uint8_t *)(&byte_translate.u8[0]), 1);
		esp_write_buffer_send_data_write((uint8_t *)(&byte_translate.u8[1]), 1);
	}

	uint8_t checksum = xor(ack, (uint8_t *)(&esp_buffer_cmd_write.data), cmd_index, 13);

	esp_write_buffer_send_data_write(&checksum, 1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		esp_write_buffer_read((uint8_t *)(&byte_receibe_usart), 1);

		if (esp_to_usb_debug == DEBUG_ON)
		{
			usbcdc_write_buffer_write((uint8_t *)(&byte_receibe_usart), 1);
		}

		HAL_UART_Receive_IT(&huart3, (uint8_t *)(&byte_receibe_usart), 1);
	}
}
