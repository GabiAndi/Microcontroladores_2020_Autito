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
		switch (esp_buffer_read.read_state)
		{
			case 0:
				if (esp_buffer_read.data[esp_buffer_read.read_index] != '\r' &&
						esp_buffer_read.data[esp_buffer_read.read_index] != '\n')
				{
					ticker_new(esp_timeout, 100, TICKER_LOW_PRIORITY);

					esp_manager.cmd_init = esp_buffer_read.read_index;

					esp_buffer_read.read_state = 1;
				}

				break;

			case 1:
				if (esp_buffer_read.data[esp_buffer_read.read_index] == '\r' ||
						esp_buffer_read.data[esp_buffer_read.read_index] == '\n')
				{
					ticker_delete(esp_timeout);

					esp_manager.cmd_end = esp_buffer_read.read_index - 1;

					esp_buffer_read.read_state = 2;
				}

				break;

			case 2:
				esp_buffer_read.read_state = 0;

				if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT")))
				{
					esp_manager.cmd = WIFI_COMMAND_AT;

					esp_buffer_read.read_state = 3;
				}

				else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT+CWMODE_CUR=1")))
				{
					esp_manager.cmd = WIFI_COMMAND_CWMODE_CUR_1;

					esp_buffer_read.read_state = 3;
				}

				else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 12, (uint8_t *)("AT+CWJAP_CUR=")))
				{
					esp_manager.cmd = WIFI_COMMAND_CWJAP_CUR;

					esp_buffer_read.read_state = 0;
				}

				else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI CONNECTED")))
				{
					esp_manager.status = WIFI_STATUS_CONNECTED;

					esp_buffer_read.read_state = 0;
				}

				else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI GOT IP")))
				{
					esp_manager.status = WIFI_STATUS_GOT_IP;

					esp_buffer_read.read_state = 0;
				}

				else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 13, (uint8_t *)("AT+CIPSTA_CUR=")))
				{
					esp_manager.cmd = WIFI_COMMAND_CIPSTA_CUR;

					esp_buffer_read.read_state = 3;
				}

				else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("AT+CIPSTATUS")))
				{
					esp_manager.cmd = WIFI_COMMAND_CIPSTATUS;

					esp_buffer_read.read_state = 3;
				}

				break;

			case 3:
				if (esp_buffer_read.data[esp_buffer_read.read_index] != '\r' &&
						esp_buffer_read.data[esp_buffer_read.read_index] != '\n')
				{
					ticker_new(esp_timeout, 100, TICKER_LOW_PRIORITY);

					esp_manager.cmd_init = esp_buffer_read.read_index;

					esp_buffer_read.read_state = 4;
				}

				break;

			case 4:
				if (esp_buffer_read.data[esp_buffer_read.read_index] == '\r' ||
						esp_buffer_read.data[esp_buffer_read.read_index] == '\n')
				{
					ticker_delete(esp_timeout);

					esp_manager.cmd_end = esp_buffer_read.read_index - 1;

					esp_buffer_read.read_state = 5;
				}

				break;

			case 5:
				switch (esp_manager.cmd)
				{
					case WIFI_COMMAND_AT:
						if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("OK")))
						{
							esp_manager.status = WIFI_STATUS_INIT;
						}

						else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("ERROR")))
						{
							esp_manager.status = WIFI_STATUS_NO_INIT;
						}

						esp_buffer_read.read_state = 0;

						break;

					case WIFI_COMMAND_CWMODE_CUR_1:
						if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("OK")))
						{
							esp_manager.status = WIFI_STATUS_STATION;
						}

						else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("ERROR")))
						{
							esp_manager.status = WIFI_STATUS_NO_INIT;
						}

						esp_buffer_read.read_state = 0;

						break;

					case WIFI_COMMAND_CIPSTA_CUR:
						if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("OK")))
						{
							esp_manager.status = WIFI_STATUS_SET_IP;
						}

						else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("ERROR")))
						{
							esp_manager.status = WIFI_STATUS_NO_INIT;
						}

						esp_buffer_read.read_state = 0;

						break;

					case WIFI_COMMAND_CIPSTATUS:
						if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("STATUS:2")))
						{
							esp_manager.status = WIFI_STATUS_READY;
						}

						else if (command_at((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("STATUS:5")))
						{
							esp_manager.status = WIFI_STATUS_NO_INIT;
						}

						esp_buffer_read.read_state = 0;

						break;
				}

				break;
		}

		esp_buffer_read.read_index++;
	}
}

__attribute__((weak)) void esp_write_pending(void)
{

}

uint8_t command_at(uint8_t *cmd, uint8_t init, uint8_t end, uint8_t *cmd_cmp)
{
	/*uint8_t i = init;
	uint8_t j = 0;

	while (i != (end + 1))
	{
		if (cmd[i] != cmd_cmp[j])
		{
			return 0;
		}

		i++;
		j++;
	}*/

	return 1;
}

void esp_timeout(void)
{
	ticker_delete(esp_timeout);

	esp_buffer_read.read_state = 0;
}
