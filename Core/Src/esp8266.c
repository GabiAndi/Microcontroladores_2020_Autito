#include "esp8266.h"

void esp_init(void)
{
	// Inicializacion de los buffers
	esp_buffer_read.read_index = 0;
	esp_buffer_read.write_index = 0;
	esp_buffer_read.read_state = 0;

	esp_buffer_write.read_index = 0;
	esp_buffer_write.write_index = 0;

	// Inicializacion del esp manager
	esp_manager.read_state = 0;
	esp_manager.cmd = ESP_COMMAND_IDLE;
	esp_manager.status = ESP_STATUS_NO_INIT;

	//esp_send_at((uint8_t *)("AT+CIPSTATUS"), 12);

	//ticker_new(esp_connect_to_ap, 200, TICKER_LOW_PRIORITY);
}

void esp_write_buffer_write(uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		esp_buffer_write.data[esp_buffer_write.write_index] = data[i];
		esp_buffer_write.write_index++;
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
	// Cabecera AT
	//write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CIPSEND="), 11);
	//write_buffer(&write_buffer_UDP, 2, 1);
	//write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

	// Cabecera UNER
	//write_buffer(&write_buffer_UDP, (uint8_t *)("UNER"), 4);
	//write_buffer(&write_buffer_UDP, &length, 1);
	//write_buffer(&write_buffer_UDP, (uint8_t *)(":"), 1);
	//write_buffer(&write_buffer_UDP, &cmd, 1);
	//write_buffer(&write_buffer_UDP, payload, length);

	//uint8_t checksum = xor(cmd, payload, 0, length);

	//write_buffer(&write_buffer_UDP, &checksum, 1);
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

				esp_manager.read_state = 0;

				break;
		}

		esp_buffer_read.read_index++;
	}
}

__attribute__((weak)) void esp_write_pending(void)
{

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
	/*if (esp_manager.cmd == ESP_COMMAND_IDLE)
	{
		switch (wifi.status)
		{
			case WIFI_STATUS_NO_INIT:
				write_buffer(&write_buffer_UDP, (uint8_t *)("AT"), 2);
				write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

				wifi.status = WIFI_STATUS_BUSY;

				break;

			case WIFI_STATUS_INIT:
				write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CWMODE_CUR=1"), 15);
				write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

				wifi.status = WIFI_STATUS_BUSY;

				break;

			case WIFI_STATUS_STATION:
				write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CWJAP_CUR=\"Gabi-RED\",\"GabiAndi26040102.\""), 43);
				write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

				wifi.status = WIFI_STATUS_BUSY;

				break;

			case WIFI_STATUS_CONNECTED:
				wifi.status = WIFI_STATUS_BUSY;

				break;

			case WIFI_STATUS_GOT_IP:
				write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CIPSTA_CUR=\"10.0.0.10\""), 25);
				write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

				wifi.status = WIFI_STATUS_BUSY;

				break;

			case WIFI_STATUS_SET_IP:
				write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CIPSTATUS"), 12);
				write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

				wifi.status = WIFI_STATUS_BUSY;

				break;

			case WIFI_STATUS_READY:
				change_ticker_ms(led_blink, LED_OK);

				delete_ticker(esp_init);

				break;
		}
	}*/
}
