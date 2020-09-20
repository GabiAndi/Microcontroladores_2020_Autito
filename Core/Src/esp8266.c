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

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI CONNECTED"), 14))
				{
					esp_manager.status = ESP_STATUS_CONNECTED;

					usbcdc_send_cmd(0x00, NULL, 0x00);
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI GOT IP"), 11))
				{
					esp_manager.status = ESP_STATUS_CONNECTED_GOT_IP;

					usbcdc_send_cmd(0x00, NULL, 0x00);
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("WIFI DISCONNECT"), 15))
				{
					esp_manager.status = ESP_STATUS_DISCONNECTED;

					usbcdc_send_cmd(0x00, NULL, 0x00);
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_init + 13, (uint8_t *)("AT+CIPSTA_CUR="), 14))
				{
					esp_manager.cmd = ESP_COMMAND_AT_CIPSTA;
				}

				else if (esp_at_cmp((uint8_t *)(esp_buffer_read.data), esp_manager.cmd_init, esp_manager.cmd_end, (uint8_t *)("OK"), 2))
				{
					switch (esp_manager.cmd)
					{
						case ESP_COMMAND_AT:
							esp_manager.status = ESP_STATUS_INIT;

							usbcdc_send_cmd(0x00, NULL, 0x00);

							break;

						case ESP_COMMAND_AT_CWMODE:
							esp_manager.status = ESP_STATUS_STATION_OK;

							usbcdc_send_cmd(0x00, NULL, 0x00);

							break;

						case ESP_COMMAND_AT_CWJAP:
							usbcdc_send_cmd(0x00, NULL, 0x00);

							break;

						case ESP_COMMAND_AT_CIPSTA:
							esp_manager.status = ESP_STATUS_SET_IP;

							usbcdc_send_cmd(0x00, NULL, 0x00);

							break;
					}

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
