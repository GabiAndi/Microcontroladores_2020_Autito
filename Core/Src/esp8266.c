#include "esp8266.h"

/**********************************************************************************/
/********************************** Variables *************************************/
/**********************************************************************************/

/******************************** Datos en ram ************************************/
extern system_flash_data_t system_ram_user;
/**********************************************************************************/

/*********************************** Tickers **************************************/
ticker_t esp_ticker_read_time_out;
ticker_t esp_ticker_write_time_out;
ticker_t esp_ticker_connect_to_ap;
ticker_t esp_ticker_hard_reset_stop;
ticker_t esp_ticker_enable_usart_receibe;
ticker_t esp_ticker_send_adc_sensor_data;
/**********************************************************************************/

/******************************* UART de la HAL ***********************************/
extern UART_HandleTypeDef huart3;
/**********************************************************************************/

/***************************** Bufferes de datos **********************************/
system_ring_buffer_t esp_buffer_read;
system_ring_buffer_t esp_buffer_write;
system_ring_buffer_t esp_buffer_cmd_write;

extern system_ring_buffer_t usbcdc_buffer_write;
/**********************************************************************************/

/*************************** Manejador de comandos ********************************/
system_cmd_manager_t esp_cmd_manager;
/**********************************************************************************/

/***************************** Control de la ESP **********************************/
esp_manager_t esp_manager;
/**********************************************************************************/

/***************************** Depuracion via USB *********************************/
extern uint8_t system_usb_debug;
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/
void esp_init(void)
{
	/***********************************************************************************/
	/************************* Inicializacion de los bufferes **************************/
	/***********************************************************************************/

	/************************* Buffer de lectura de la ESP8266 *************************/
	esp_buffer_read.read_index = 0;
	esp_buffer_read.write_index = 0;
	/***********************************************************************************/

	/************************ Buffer de escritura de la ESP8266 ************************/
	esp_buffer_write.read_index = 0;
	esp_buffer_write.write_index = 0;
	/***********************************************************************************/

	/*********************** Buffer de escritura de comandos UDP ***********************/
	esp_buffer_cmd_write.read_index = 0;
	esp_buffer_cmd_write.write_index = 0;
	/***********************************************************************************/

	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	/***********************************************************************************/
	/************************* Inicializacion de los comandos **************************/
	/***********************************************************************************/
	esp_cmd_manager.buffer_read = &esp_buffer_read;

	esp_cmd_manager.read_state = 0;
	esp_cmd_manager.read_payload_init = 0;
	esp_cmd_manager.read_payload_length = 0;

	esp_cmd_manager.read_time_out = &esp_ticker_read_time_out;

	esp_cmd_manager.buffer_write = &esp_buffer_cmd_write;
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	/***********************************************************************************/
	/************************* Inicializacion del esp manager **************************/
	/***********************************************************************************/
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

	esp_manager.send_data_length = 0;

	esp_manager.auto_connection = 0;
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	/***********************************************************************************/
	/************************** Inicializacion de los tickers **************************/
	/***********************************************************************************/

	/***************************** Ticker de timeout read  *****************************/
	esp_ticker_read_time_out.ms_max = 150;
	esp_ticker_read_time_out.ms_count = 0;
	esp_ticker_read_time_out.calls = 0;
	esp_ticker_read_time_out.active = TICKER_NO_ACTIVE;
	esp_ticker_read_time_out.priority = TICKER_LOW_PRIORITY;
	esp_ticker_read_time_out.ticker_function = esp_timeout_read;

	ticker_new(&esp_ticker_read_time_out);
	/***********************************************************************************/

	/***************************** Ticker de timeout write  ****************************/
	esp_ticker_write_time_out.ms_max = 150;
	esp_ticker_write_time_out.ms_count = 0;
	esp_ticker_write_time_out.calls = 0;
	esp_ticker_write_time_out.active = TICKER_NO_ACTIVE;
	esp_ticker_write_time_out.priority = TICKER_LOW_PRIORITY;
	esp_ticker_write_time_out.ticker_function = esp_timeout_write;

	ticker_new(&esp_ticker_write_time_out);
	/***********************************************************************************/

	/**************************** Ticker de autoconectado  *****************************/
	esp_ticker_connect_to_ap.ms_count = 0;
	esp_ticker_connect_to_ap.ms_max = 200;
	esp_ticker_connect_to_ap.calls = 0;
	esp_ticker_connect_to_ap.priority = TICKER_LOW_PRIORITY;
	esp_ticker_connect_to_ap.ticker_function = esp_connect_to_ap;
	esp_ticker_connect_to_ap.active = TICKER_NO_ACTIVE;

	ticker_new(&esp_ticker_connect_to_ap);
	/***********************************************************************************/

	/****************************** Ticker de hard reset *******************************/
	esp_ticker_hard_reset_stop.ms_count = 0;
	esp_ticker_hard_reset_stop.ms_max = 200;
	esp_ticker_hard_reset_stop.calls = 0;
	esp_ticker_hard_reset_stop.priority = TICKER_LOW_PRIORITY;
	esp_ticker_hard_reset_stop.ticker_function = esp_hard_reset_stop;
	esp_ticker_hard_reset_stop.active = TICKER_NO_ACTIVE;

	ticker_new(&esp_ticker_hard_reset_stop);
	/***********************************************************************************/

	/************************** Ticker de habilitación USART ***************************/
	esp_ticker_enable_usart_receibe.ms_count = 0;
	esp_ticker_enable_usart_receibe.ms_max = 1500;
	esp_ticker_enable_usart_receibe.calls = 0;
	esp_ticker_enable_usart_receibe.priority = TICKER_LOW_PRIORITY;
	esp_ticker_enable_usart_receibe.ticker_function = esp_enable_usart_receibe;
	esp_ticker_enable_usart_receibe.active = TICKER_NO_ACTIVE;

	ticker_new(&esp_ticker_enable_usart_receibe);
	/***********************************************************************************/

	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	// Inicializamos la ESP
	esp_hard_reset();
}

void esp_read_pending(void)
{
	// Si hay algo en el buffer de recepción se lo lee
	if (esp_buffer_read.read_index != esp_buffer_read.write_index)
	{
		// Estado de la lectura
		switch (esp_manager.read_state)
		{
			// Estado en donde se identifica que tipo de información se esta recibiendo
			case 0:
				// Si el byte actual no es ningun caracter de separación se esta ante un nuevo comando
				if ((esp_buffer_read.data[esp_buffer_read.read_index] != '\r') &&
						(esp_buffer_read.data[esp_buffer_read.read_index] != '\n'))
				{
					// Se activa el timeout para la lectura del paquete
					esp_ticker_read_time_out.ms_count = 0;
					esp_ticker_read_time_out.active = TICKER_ACTIVE;

					// Si se recibio un > es porque se va a enviar datos via UDP
					if (esp_buffer_read.data[esp_buffer_read.read_index] == '>')
					{
						esp_manager.read_state = 3;
					}

					// Si se recibe un + es porque se esta ante un comando de estado
					else if (esp_buffer_read.data[esp_buffer_read.read_index] == '+')
					{
						esp_manager.read_state = 4;
					}

					// Si paquete inicia de cualquier otra manera se esta ante un comando AT
					else
					{
						esp_manager.read_state = 1;

						// Se guarda la posición de inicio del comando
						esp_manager.cmd_init = esp_buffer_read.read_index;
					}
				}

				break;

			// Fin de la recepción de un comando AT
			case 1:
				if ((esp_buffer_read.data[esp_buffer_read.read_index] == '\r') ||
						(esp_buffer_read.data[esp_buffer_read.read_index] == '\n'))
				{
					esp_manager.read_state = 2;

					// Se guarda la posición de fin del comando
					esp_manager.cmd_end = esp_buffer_read.read_index - 1;
				}

				break;

			// Se analiza el comando que se recibio
			case 2:
				esp_ticker_read_time_out.active = TICKER_NO_ACTIVE;

				esp_manager.read_state = 0;

				/*****************************************************************************************/
				/****************************************** Eco ******************************************/
				/*****************************************************************************************/

				/************************************** Comando: AT **************************************/
				if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("AT"), 2))
				{
					esp_manager.cmd = ESP_COMMAND_AT;
				}
				/*****************************************************************************************/

				/********************************* Comando: AT+CWMODE_CUR ********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_init + 13,
						(uint8_t *)("AT+CWMODE_CUR="), 14))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CWMODE;
				}
				/*****************************************************************************************/

				/********************************* Comando: AT+CWJAP_CUR *********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_init + 12,
						(uint8_t *)("AT+CWJAP_CUR="), 13))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CWJAP;
				}
				/*****************************************************************************************/

				/******************************** Comando: AT+CIPSTA_CUR *********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_init + 13,
						(uint8_t *)("AT+CIPSTA_CUR="), 14))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSTA;
				}
				/*****************************************************************************************/

				/********************************* Comando: AT+CIPSTATUS *********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("AT+CIPSTATUS"), 12))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSTATUS;
				}
				/*****************************************************************************************/

				/********************************** Comando: AT+CIPCLOSE *********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("AT+CIPCLOSE"), 11))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPCLOSE;
				}
				/*****************************************************************************************/

				/********************************** Comando: AT+CIPSTART *********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_init + 11,
						(uint8_t *)("AT+CIPSTART="), 12))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSTART;
				}
				/*****************************************************************************************/

				/********************************** Comando: AT+CIPSEND **********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_init + 10,
						(uint8_t *)("AT+CIPSEND="), 11))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSEND;
				}
				/*****************************************************************************************/

				/*****************************************************************************************/
				/*****************************************************************************************/
				/*****************************************************************************************/

				/*****************************************************************************************/
				/**************************************** Estados ****************************************/
				/*****************************************************************************************/

				/************************************ Estado: STATUS:2 ***********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("STATUS:2"), 8))
				{
					esp_manager.connected = ESP_CONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/************************************ Estado: STATUS:3 ***********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("STATUS:3"), 8))
				{
					esp_manager.connected = ESP_CONNECTED;
					esp_manager.udp = ESP_UDP_INIT;
				}
				/*****************************************************************************************/

				/************************************ Estado: STATUS:5 ***********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("STATUS:5"), 8))
				{
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/********************************* Estado: WIFI CONNECTED ********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("WIFI CONNECTED"), 14))
				{
					esp_manager.connected = ESP_CONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/*********************************** Estado: WIFI GOT IP *********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("WIFI GOT IP"), 11))
				{
					esp_manager.connected = ESP_CONNECTED_GOT_IP;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/********************************* Estado: WIFI DISCONNECT *******************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("WIFI DISCONNECT"), 15))
				{
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/*****************************************************************************************/
				/*****************************************************************************************/
				/*****************************************************************************************/

				/*****************************************************************************************/
				/************************************** Respuestas ***************************************/
				/*****************************************************************************************/

				/************************************** Respuesta: OK ************************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("OK"), 2))
				{
					// Se identifica de que comando viene la respuesta
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

					// El comando se pone en espera
					esp_manager.cmd = ESP_COMMAND_IDLE;
				}
				/*****************************************************************************************/

				/************************************ Respuesta: ERROR ***********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("ERROR"), 5))
				{
					// Se identifica de que comando viene la respuesta
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

					// El comando se pone en espera
					esp_manager.cmd = ESP_COMMAND_IDLE;
				}
				/*****************************************************************************************/

				/****************************** Respuesta: ALREADY CONNECTED *****************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("ALREADY CONNECTED"), 17))
				{
					// Se identifica de que comando viene la respuesta
					switch (esp_manager.cmd)
					{
						case ESP_COMMAND_AT_CIPSTART:
							esp_manager.udp = ESP_UDP_INIT;

							break;
					}

					// El comando se pone en espera
					esp_manager.cmd = ESP_COMMAND_IDLE;
				}
				/*****************************************************************************************/

				/*********************************** Respuesta: SEND OK **********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("SEND OK"), 7))
				{
					esp_manager.send = ESP_SEND_OK;

					// El comando se pone en espera
					esp_manager.cmd = ESP_COMMAND_IDLE;
				}
				/*****************************************************************************************/

				/********************************** Respuesta: SEND FAIL *********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("SEND FAIL"), 9))
				{
					esp_manager.send = ESP_SEND_NO_INIT;
					esp_manager.error = ESP_ERROR_SEND_DATA;

					// El comando se pone en espera
					esp_manager.cmd = ESP_COMMAND_IDLE;
				}
				/*****************************************************************************************/

				/****************************************************************************************/
				/****************************************************************************************/
				/****************************************************************************************/

				break;

			// Esperando para poder enviar
			case 3:
				if (esp_buffer_read.data[esp_buffer_read.read_index] == ' ')
				{
					esp_ticker_read_time_out.active = TICKER_NO_ACTIVE;
					esp_manager.read_state = 0;

					esp_manager.send = ESP_SEND_READY;
				}

				break;

			// Se comprueba el comando de estado que se recibio
			case 4:
				if (esp_buffer_read.data[esp_buffer_read.read_index] == 'I')
				{
					esp_manager.read_state = 7;
				}

				else
				{
					esp_manager.read_state = 5;
				}

				break;

			// Fin de la recepción de un estado AT
			case 5:
				if ((esp_buffer_read.data[esp_buffer_read.read_index] == '\r') ||
						(esp_buffer_read.data[esp_buffer_read.read_index] == '\n'))
				{
					esp_manager.read_state = 6;

					// Se guarda la posición de fin del comando
					esp_manager.cmd_end = esp_buffer_read.read_index - 1;
				}

				break;

			// Se analiza el estado que se recibio
			case 6:
				esp_ticker_read_time_out.active = TICKER_NO_ACTIVE;
				esp_manager.read_state = 0;

				/*****************************************************************************************/
				/**************************************** Estados ****************************************/
				/*****************************************************************************************/

				/************************************ Estado: +CWJAP:1 ***********************************/
				if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("+CWJAP:1"), 8))
				{
					esp_manager.error = ESP_ERROR_CON_TIMEOUT;
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/************************************ Estado: +CWJAP:2 ***********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("+CWJAP:2"), 8))
				{
					esp_manager.error = ESP_ERROR_CON_PSW;
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/************************************ Estado: +CWJAP:3 ***********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("+CWJAP:3"), 8))
				{
					esp_manager.error = ESP_ERROR_CON_NO_AP;
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/************************************ Estado: +CWJAP:4 ***********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
						esp_manager.cmd_init,
						esp_manager.cmd_end,
						(uint8_t *)("+CWJAP:4"), 8))
				{
					esp_manager.error = ESP_ERROR_CON_FAIL;
					esp_manager.connected = ESP_DISCONNECTED;
					esp_manager.udp = ESP_UDP_NO_INIT;
				}
				/*****************************************************************************************/

				/*****************************************************************************************/
				/*****************************************************************************************/
				/*****************************************************************************************/

				break;

			// Recepcion de datos via UDP
			case 7:
				// Si se termino de analizar el paquete se resetea el estado de la lectura
				if (system_data_package(&esp_cmd_manager))
				{
					esp_manager.read_state = 0;
				}

				break;
		}

		esp_buffer_read.read_index++;
	}
}

void esp_write_pending(void)
{
	// Si hay datos en el buffer para enviar enviamos
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
	// Estados de envio
	switch (esp_manager.send)
	{
		// Se puede enviar
		case ESP_SEND_NO_INIT:
			if ((esp_buffer_cmd_write.read_index != esp_buffer_cmd_write.write_index)
					&& (esp_manager.status == ESP_STATUS_INIT)
					&& (esp_manager.udp == ESP_UDP_INIT)
					&& (esp_manager.connected == ESP_CONNECTED_SET_IP)
					&& (esp_manager.error == ESP_ERROR_OK))
			{
				if (esp_buffer_cmd_write.read_index < esp_buffer_cmd_write.write_index)
				{
					esp_manager.send_data_length = esp_buffer_cmd_write.write_index - esp_buffer_cmd_write.read_index;
				}

				else
				{
					esp_manager.send_data_length = (uint8_t)(256 - esp_buffer_cmd_write.read_index + esp_buffer_cmd_write.write_index);
				}

				memset(esp_manager.len_char, '\0', 4);

				esp_manager.len_uint = sprintf(esp_manager.len_char, "%u", esp_manager.send_data_length);

				system_buffer_write(&esp_buffer_write, (uint8_t *)("AT+CIPSEND="), 11);
				system_buffer_write(&esp_buffer_write, (uint8_t *)(esp_manager.len_char), esp_manager.len_uint);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\r\n"), 2);

				esp_ticker_write_time_out.ms_count = 0;
				esp_ticker_write_time_out.active = TICKER_ACTIVE;

				esp_manager.send = ESP_SEND_WAITING_OK;
			}

			break;

		case ESP_SEND_READY:
			while (esp_manager.send_data_length > 0)
			{
				esp_buffer_write.data[esp_buffer_write.write_index] = esp_buffer_cmd_write.data[esp_buffer_cmd_write.read_index];

				esp_buffer_write.write_index++;
				esp_buffer_cmd_write.read_index++;

				esp_manager.send_data_length--;
			}

			esp_manager.send = ESP_SEND_SENDING;

			break;

		case ESP_SEND_OK:
			esp_ticker_write_time_out.active = TICKER_NO_ACTIVE;

			esp_manager.send = ESP_SEND_NO_INIT;

			break;
	}
}

uint8_t esp_at_cmp(uint8_t *at, uint8_t at_init, uint8_t at_end, uint8_t *at_cmp, uint8_t at_cmp_length)
{
	uint8_t i = at_init;
	uint8_t j = 0;

	if (((at_init > at_end) && (at_cmp_length < (uint8_t)(256 - at_init + at_end + 1))) || ((at_cmp_length < (uint8_t)(at_end + 1 - at_init))))
	{
		return 0;
	}

	while (i != (uint8_t)(at_end + 1))
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
	esp_ticker_read_time_out.active = TICKER_NO_ACTIVE;

	esp_manager.read_state = 0;
	esp_cmd_manager.read_state = 0;
}

void esp_timeout_write(void)
{
	esp_ticker_write_time_out.active = TICKER_NO_ACTIVE;

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
				system_buffer_write(&esp_buffer_write, (uint8_t *)("AT\r\n"), 4);

				esp_manager.auto_connection = 1;

				break;

			case 1:
				if (esp_manager.status == ESP_STATUS_INIT)
				{
					esp_manager.auto_connection = 2;
				}

				break;

			case 2:
				system_buffer_write(&esp_buffer_write, (uint8_t *)("AT+CWMODE_CUR=1\r\n"), 17);

				esp_manager.auto_connection = 3;

				break;

			case 3:
				if (esp_manager.station == ESP_STATION_INIT)
				{
					esp_manager.auto_connection = 4;
				}

				break;

			case 4:
				system_buffer_write(&esp_buffer_write, (uint8_t *)("AT+CWJAP_CUR=\""), 14);
				system_buffer_write(&esp_buffer_write, system_ram_user.ssid, system_ram_user.ssid_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\",\""), 3);
				system_buffer_write(&esp_buffer_write, system_ram_user.psw, system_ram_user.psw_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\"\r\n"), 3);

				esp_manager.auto_connection = 5;

				break;

			case 5:
				if (esp_manager.connected == ESP_CONNECTED_GOT_IP)
				{
					esp_manager.auto_connection = 6;
				}

				break;

			case 6:
				system_buffer_write(&esp_buffer_write, (uint8_t *)("AT+CIPSTA_CUR=\""), 15);
				system_buffer_write(&esp_buffer_write, system_ram_user.ip_mcu, system_ram_user.ip_mcu_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\"\r\n"), 3);

				esp_manager.auto_connection = 7;

				break;

			case 7:
				if (esp_manager.connected == ESP_CONNECTED_SET_IP)
				{
					esp_manager.auto_connection = 8;
				}

				break;

			case 8:
				system_buffer_write(&esp_buffer_write, (uint8_t *)("AT+CIPSTART=\"UDP\",\""), 19);
				system_buffer_write(&esp_buffer_write, system_ram_user.ip_pc, system_ram_user.ip_pc_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\","), 2);
				system_buffer_write(&esp_buffer_write, system_ram_user.port, system_ram_user.port_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)(","), 1);
				system_buffer_write(&esp_buffer_write, system_ram_user.port, system_ram_user.port_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\r\n"), 2);

				esp_manager.auto_connection = 9;

				break;

			case 9:
				if (esp_manager.udp == ESP_UDP_INIT)
				{
					esp_ticker_connect_to_ap.active = TICKER_NO_ACTIVE;

					esp_manager.auto_connection = 0;
				}

				break;
		}
	}

	if (esp_ticker_connect_to_ap.calls > 100)
	{
		esp_ticker_connect_to_ap.active = TICKER_NO_ACTIVE;
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

	esp_manager.send_data_length = 0;

	esp_manager.auto_connection = 0;

	esp_ticker_hard_reset_stop.ms_count = 0;
	esp_ticker_hard_reset_stop.active = TICKER_ACTIVE;

	// Se detiene la captura de datos
	HAL_UART_Abort_IT(&huart3);
}

void esp_hard_reset_stop(void)
{
	HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_SET);

	esp_ticker_hard_reset_stop.active = TICKER_NO_ACTIVE;

	esp_ticker_enable_usart_receibe.ms_count = 0;
	esp_ticker_enable_usart_receibe.active = TICKER_ACTIVE;
}

void esp_enable_usart_receibe(void)
{
	esp_ticker_enable_usart_receibe.active = TICKER_NO_ACTIVE;

	esp_ticker_connect_to_ap.ms_count = 0;
	esp_ticker_connect_to_ap.active = TICKER_ACTIVE;

	// Inicia la captura de datos via USART
	HAL_UART_Receive_IT(&huart3, (uint8_t *)(&esp_manager.byte_receibe_usart), 1);
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

	// Parpadeo del LED
	if (esp_manager.status == ESP_STATUS_NO_INIT)
	{
		system_led_set_status(SYSTEM_LED_FAIL);
	}

	else if (esp_manager.udp != ESP_UDP_INIT)
	{
		system_led_set_status(SYSTEM_LED_INIT);
	}

	else if ((esp_manager.error == ESP_ERROR_OK) && (esp_manager.udp == ESP_UDP_INIT))
	{
		system_led_set_status(SYSTEM_LED_OK);
	}
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Callbacks *************************************/
/**********************************************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		esp_buffer_read.data[esp_buffer_read.write_index] = esp_manager.byte_receibe_usart;
		esp_buffer_read.write_index++;

		if (system_usb_debug == SYSTEM_USB_DEBUG_ON)
		{
			usbcdc_buffer_write.data[usbcdc_buffer_write.write_index] = esp_manager.byte_receibe_usart;
			usbcdc_buffer_write.write_index++;
		}

		HAL_UART_Receive_IT(&huart3, (uint8_t *)(&esp_manager.byte_receibe_usart), 1);
	}
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
