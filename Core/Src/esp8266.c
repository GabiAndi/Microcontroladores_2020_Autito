#include "esp8266.h"

// Variables
// Tickers
ticker_t ticker_esp_led_status;
ticker_t ticker_esp_write_send_data_pending;
ticker_t ticker_esp_connect_to_ap;
ticker_t ticker_esp_hard_reset_stop;
ticker_t ticker_esp_send_adc_sensor_data;
/*ticker_t ticker_esp_send_adc_batery_data;*/

// Datos guardados
extern flash_data_t flash_user_ram;

// UART de la HAL
extern UART_HandleTypeDef huart3;

// Bufferes de datos
system_ring_buffer_t esp_buffer_read;
system_ring_buffer_t esp_buffer_write;
system_ring_buffer_t esp_buffer_cmd_write;

// Manejador de comandos
system_cmd_manager_t esp_cmd_manager;

// Control de la ESP
esp_manager_t esp_manager;

// Flag de depuracion via USB
extern uint8_t system_esp_to_usb_debug;

void esp_init(void)
{
	/***********************************************************************************/
	/************************* Inicializacion de los bufferes **************************/
	/***********************************************************************************/

	/************************* Buffer de lectura de la ESP8266 *************************/
	memset((void *)(esp_buffer_read.data), 0, 256);
	esp_buffer_read.read_index = 0;
	esp_buffer_read.write_index = 0;
	/***********************************************************************************/

	/************************ Buffer de escritura de la ESP8266 ************************/
	memset((void *)(esp_buffer_write.data), 0, 256);
	esp_buffer_write.read_index = 0;
	esp_buffer_write.write_index = 0;
	/***********************************************************************************/

	/*********************** Buffer de escritura de comandos UDP ***********************/
	memset((void *)(esp_buffer_cmd_write.data), 0, 256);
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

	esp_cmd_manager.read_time_out.ms_max = 100;
	esp_cmd_manager.read_time_out.ms_count = 0;
	esp_cmd_manager.read_time_out.calls = 0;
	esp_cmd_manager.read_time_out.active = TICKER_NO_ACTIVE;
	esp_cmd_manager.read_time_out.priority = TICKER_LOW_PRIORITY;
	esp_cmd_manager.read_time_out.ticker_function = esp_timeout_read;

	ticker_new(&esp_cmd_manager.read_time_out);

	esp_cmd_manager.buffer_write = &esp_buffer_cmd_write;

	esp_cmd_manager.write_time_out.ms_max = 100;
	esp_cmd_manager.write_time_out.ms_count = 0;
	esp_cmd_manager.write_time_out.calls = 0;
	esp_cmd_manager.write_time_out.active = TICKER_NO_ACTIVE;
	esp_cmd_manager.write_time_out.priority = TICKER_LOW_PRIORITY;
	esp_cmd_manager.write_time_out.ticker_function = esp_timeout_write;

	ticker_new(&esp_cmd_manager.write_time_out);

	esp_cmd_manager.byte_converter.u32 = 0;
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

	esp_manager.read_state = ESP_DATA_WAIT_INIT;

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

	/*// Ticker para el led de estado
	ticker_esp_led_status.ms_count = 0;
	ticker_esp_led_status.ms_max = LED_FAIL;
	ticker_esp_led_status.calls = 0;
	ticker_esp_led_status.priority = TICKER_LOW_PRIORITY;
	ticker_esp_led_status.ticker_function = esp_led_status;
	ticker_esp_led_status.active = TICKER_ACTIVE;

	ticker_new(&ticker_esp_led_status);*/

	/*********************** Ticker para el envio de datos UDP  ************************/
	ticker_esp_write_send_data_pending.ms_count = 0;
	ticker_esp_write_send_data_pending.ms_max = 100;
	ticker_esp_write_send_data_pending.calls = 0;
	ticker_esp_write_send_data_pending.priority = TICKER_LOW_PRIORITY;
	ticker_esp_write_send_data_pending.ticker_function = esp_write_send_data_pending;
	ticker_esp_write_send_data_pending.active = TICKER_ACTIVE;

	ticker_new(&ticker_esp_write_send_data_pending);
	/***********************************************************************************/

	/**************************** Ticker de autoconectado  *****************************/
	ticker_esp_connect_to_ap.ms_count = 0;
	ticker_esp_connect_to_ap.ms_max = 100;
	ticker_esp_connect_to_ap.calls = 0;
	ticker_esp_connect_to_ap.priority = TICKER_LOW_PRIORITY;
	ticker_esp_connect_to_ap.ticker_function = esp_connect_to_ap;
	ticker_esp_connect_to_ap.active = TICKER_NO_ACTIVE;

	ticker_new(&ticker_esp_connect_to_ap);
	/***********************************************************************************/

	/****************************** Ticker de hard reset  ******************************/
	ticker_esp_hard_reset_stop.ms_count = 0;
	ticker_esp_hard_reset_stop.ms_max = 200;
	ticker_esp_hard_reset_stop.calls = 0;
	ticker_esp_hard_reset_stop.priority = TICKER_LOW_PRIORITY;
	ticker_esp_hard_reset_stop.ticker_function = esp_hard_reset_stop;
	ticker_esp_hard_reset_stop.active = TICKER_NO_ACTIVE;

	ticker_new(&ticker_esp_hard_reset_stop);
	/***********************************************************************************/

	/************************** Ticker envio de datos del adc  *************************/
	ticker_esp_send_adc_sensor_data.ms_count = 0;
	ticker_esp_send_adc_sensor_data.ms_max = 255;
	ticker_esp_send_adc_sensor_data.calls = 0;
	ticker_esp_send_adc_sensor_data.priority = TICKER_LOW_PRIORITY;
	ticker_esp_send_adc_sensor_data.ticker_function = esp_send_adc_sensor_data;
	ticker_esp_send_adc_sensor_data.active = TICKER_NO_ACTIVE;

	ticker_new(&ticker_esp_send_adc_sensor_data);
	/***********************************************************************************/

	/*ticker_esp_send_adc_batery_data.ms_count = 0;
	ticker_esp_send_adc_batery_data.ms_max = 10000;
	ticker_esp_send_adc_batery_data.calls = 0;
	ticker_esp_send_adc_batery_data.priority = TICKER_LOW_PRIORITY;
	ticker_esp_send_adc_batery_data.ticker_function = esp_send_adc_batery_data;
	ticker_esp_send_adc_batery_data.active = TICKER_DEACTIVATE;

	ticker_new(&ticker_esp_send_adc_batery_data);*/
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	// Inicia la captura de datos via USART
	HAL_UART_Receive_IT(&huart3, (uint8_t *)(&esp_manager.byte_receibe_usart), 1);

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
			case ESP_DATA_WAIT_INIT:
				// Si el byte actual no es ningun caracter de separación se esta ante un nuevo comando
				if ((esp_buffer_read.data[esp_buffer_read.read_index] != '\r') &&
						(esp_buffer_read.data[esp_buffer_read.read_index] != '\n'))
				{
					// Se activa el timeout para la lectura del paquete
					esp_cmd_manager.read_time_out.ms_count = 0;
					esp_cmd_manager.read_time_out.active = TICKER_ACTIVE;

					// Si se recibio un > es porque se va a enviar datos via UDP
					if (esp_buffer_read.data[esp_buffer_read.read_index] == '>')
					{
						esp_manager.read_state = ESP_DATA_WAIT_SEND;
					}

					// Si se recibe un + es porque se esta ante un comando de estado
					else if (esp_buffer_read.data[esp_buffer_read.read_index] == '+')
					{
						esp_manager.read_state = ESP_DATA_WAIT_STATE;
					}

					// Si paquete inicia de cualquier otra manera se esta ante un comando AT
					else
					{
						esp_manager.read_state = ESP_DATA_WAIT_END_AT;

						// Se guarda la posición de inicio del comando
						esp_manager.cmd_init = esp_buffer_read.read_index;
					}
				}

				break;

			// Fin de la recepción de un comando AT
			case ESP_DATA_WAIT_END_AT:
				if ((esp_buffer_read.data[esp_buffer_read.read_index] == '\r') ||
						(esp_buffer_read.data[esp_buffer_read.read_index] == '\n'))
				{
					esp_manager.read_state = ESP_DATA_AT_COMMAND;

					// Se guarda la posición de fin del comando
					esp_manager.cmd_end = esp_buffer_read.read_index - 1;
				}

				break;

			// Se analiza el comando que se recibio
			case ESP_DATA_AT_COMMAND:
				esp_cmd_manager.read_time_out.active = TICKER_NO_ACTIVE;

				esp_manager.read_state = ESP_DATA_WAIT_INIT;

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
			case ESP_DATA_WAIT_SEND:
				if (esp_buffer_read.data[esp_buffer_read.read_index] == ' ')
				{
					esp_cmd_manager.read_time_out.active = TICKER_NO_ACTIVE;
					esp_manager.read_state = ESP_DATA_WAIT_INIT;

					esp_manager.send = ESP_SEND_READY;
				}

				break;

			// Se comprueba el comando de estado que se recibio
			case ESP_DATA_WAIT_STATE:
				if (esp_buffer_read.data[esp_buffer_read.read_index] == 'I')
				{
					esp_manager.read_state = ESP_DATA_RECEIVE;
				}

				else
				{
					esp_manager.read_state = ESP_DATA_WAIT_END_STATE;
				}

				break;

			// Fin de la recepción de un comando AT
			case ESP_DATA_WAIT_END_STATE:
				if ((esp_buffer_read.data[esp_buffer_read.read_index] == '\r') ||
						(esp_buffer_read.data[esp_buffer_read.read_index] == '\n'))
				{
					esp_manager.read_state = ESP_DATA_STATE;

					// Se guarda la posición de fin del comando
					esp_manager.cmd_end = esp_buffer_read.read_index - 1;
				}

				break;

			// Se analiza el estado que se recibio
			case ESP_DATA_STATE:
				esp_cmd_manager.read_time_out.active = TICKER_NO_ACTIVE;
				esp_manager.read_state = ESP_DATA_WAIT_INIT;

				/*****************************************************************************************/
				/**************************************** Estados ****************************************/
				/*****************************************************************************************/

				/************************************ Estado: STATUS:2 ***********************************/
				if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
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

				/************************************ Estado: +CWJAP:1 ***********************************/
				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data),
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

				break;

			// Recepcion de datos via UDP
			case ESP_DATA_RECEIVE:
				// Si se termino de analizar el paquete se resetea el estado de la lectura
				if (system_data_package(&esp_cmd_manager))
				{
					esp_manager.read_state = ESP_DATA_WAIT_INIT;
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
					esp_manager.send_data_length = (uint8_t)(256 - (esp_buffer_cmd_write.read_index + esp_buffer_cmd_write.write_index));
				}

				memset(esp_manager.len_char, '\0', 4);

				esp_manager.len_uint = sprintf(esp_manager.len_char, "%u", esp_manager.send_data_length);

				system_buffer_write(&esp_buffer_write, (uint8_t *)("AT+CIPSEND="), 11);
				system_buffer_write(&esp_buffer_write, (uint8_t *)(esp_manager.len_char), esp_manager.len_uint);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\r\n"), 2);

				esp_cmd_manager.write_time_out.ms_count = 0;
				esp_cmd_manager.write_time_out.active = TICKER_ACTIVE;

				esp_manager.send = ESP_SEND_WAITING_OK;
			}

			break;

		case ESP_SEND_READY:
			while (esp_manager.send_data_length > 0)
			{
				esp_buffer_write.data[esp_buffer_cmd_write.write_index++] =
						esp_buffer_cmd_write.data[esp_buffer_cmd_write.read_index++];

				esp_manager.send_data_length--;
			}

			esp_manager.send = ESP_SEND_SENDING;

			break;

		case ESP_SEND_OK:
			esp_cmd_manager.write_time_out.active = TICKER_NO_ACTIVE;

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
	esp_cmd_manager.read_time_out.active = TICKER_NO_ACTIVE;

	esp_manager.read_state = 0;
	esp_cmd_manager.read_state = 0;
}

void esp_timeout_write(void)
{
	esp_cmd_manager.write_time_out.active = TICKER_NO_ACTIVE;

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
				system_buffer_write(&esp_buffer_write, flash_user_ram.ssid, flash_user_ram.ssid_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\",\""), 3);
				system_buffer_write(&esp_buffer_write, flash_user_ram.psw, flash_user_ram.psw_length);
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
				system_buffer_write(&esp_buffer_write, flash_user_ram.ip_mcu, flash_user_ram.ip_mcu_length);
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
				system_buffer_write(&esp_buffer_write, flash_user_ram.ip_pc, flash_user_ram.ip_pc_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\","), 2);
				system_buffer_write(&esp_buffer_write, flash_user_ram.port, flash_user_ram.port_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)(","), 1);
				system_buffer_write(&esp_buffer_write, flash_user_ram.port, flash_user_ram.port_length);
				system_buffer_write(&esp_buffer_write, (uint8_t *)("\r\n"), 2);

				esp_manager.auto_connection = 9;

				break;

			case 9:
				if (esp_manager.udp == ESP_UDP_INIT)
				{
					ticker_esp_connect_to_ap.active = TICKER_NO_ACTIVE;

					esp_manager.auto_connection = 0;
				}

				break;
		}
	}

	if (ticker_esp_connect_to_ap.calls > 100)
	{
		ticker_esp_connect_to_ap.active = TICKER_NO_ACTIVE;
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

	ticker_esp_hard_reset_stop.ms_count = 0;
	ticker_esp_hard_reset_stop.active = TICKER_ACTIVE;
}

void esp_hard_reset_stop(void)
{
	HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_SET);

	ticker_esp_hard_reset_stop.active = TICKER_NO_ACTIVE;

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

void esp_send_adc_sensor_data(void)
{
	/*// Calculo la media de los datos almacenados en el buffer
	for (uint8_t i = 0 ; i < 6 ; i++)
	{
		adc_buffer.mean_aux = 0;

		for (uint8_t j = 0 ; j < ADC_BUFFER_LENGTH ; j += ADC_MEAN_STEP)
		{
			adc_buffer.mean_aux += adc_buffer.data[j][i];
		}

		adc_buffer.mean[i] = (uint16_t)(adc_buffer.mean_aux / (ADC_BUFFER_LENGTH / ADC_MEAN_STEP));
	}

	//esp_send_cmd(0xC0, (uint8_t *)(&adc_buffer.mean[0]), 12);

	uint8_t init_index;

	esp_write_buffer_send_data_write((uint8_t *)("UNER"), 4);

	ack = 13;
	esp_write_buffer_send_data_write(&ack, 1);

	esp_write_buffer_send_data_write((uint8_t *)(":"), 1);

	ack = 0xC0;
	esp_write_buffer_send_data_write(&ack, 1);

	init_index = esp_buffer_cmd_write.write_index;

	esp_write_buffer_send_data_write((uint8_t *)(&adc_buffer.send_esp), 1);

	for (uint8_t i = 0 ; i < 6 ; i++)
	{
		byte_translate.u16[0] = adc_buffer.mean[i];

		esp_write_buffer_send_data_write((uint8_t *)(&byte_translate.u8[0]), 1);
		esp_write_buffer_send_data_write((uint8_t *)(&byte_translate.u8[1]), 1);
	}

	uint8_t checksum = check_xor(ack, (uint8_t *)(&esp_buffer_cmd_write.data), init_index, 13);

	esp_write_buffer_send_data_write(&checksum, 1);*/
}

/*void esp_send_adc_batery_data(void)
{
	uint8_t init_index;

	esp_write_buffer_send_data_write((uint8_t *)("UNER"), 4);

	ack = 3;
	esp_write_buffer_send_data_write(&ack, 1);

	esp_write_buffer_send_data_write((uint8_t *)(":"), 1);

	ack = 0xC3;
	esp_write_buffer_send_data_write(&ack, 1);

	init_index = esp_buffer_cmd_write.write_index;

	esp_write_buffer_send_data_write((uint8_t *)(&adc_buffer.send_batery_esp), 1);

	byte_translate.u16[0] = adc_buffer.batery;

	esp_write_buffer_send_data_write((uint8_t *)(byte_translate.u8), 2);

	uint8_t checksum = check_xor(ack, (uint8_t *)(&esp_buffer_cmd_write.data), init_index, 3);

	esp_write_buffer_send_data_write(&checksum, 1);
}*/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		esp_buffer_read.data[esp_buffer_read.write_index++] = esp_manager.byte_receibe_usart;

		if (system_esp_to_usb_debug == DEBUG_ON)
		{
			//usbcdc_buffer_read.data[usbcdc_buffer_read.write_index++] = esp_manager.byte_receibe_usart;
		}

		HAL_UART_Receive_IT(&huart3, (uint8_t *)(&esp_manager.byte_receibe_usart), 1);
	}
}
