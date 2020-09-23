#include "system.h"

// Variables
uint8_t debug;

void system_init(void)
{
	ticker_init_core();	// Inicia la configuracion de los tickers

	usbcdc_init();	// Inicia la configuracion del USB
	esp_init();	// Inicia la configuracion del ESP
}

uint8_t xor(uint8_t cmd, uint8_t *payload, uint8_t payload_init, uint8_t payload_length)
{
	uint8_t xor = 0x00;

	xor ^= 'U';
	xor ^= 'N';
	xor ^= 'E';
	xor ^= 'R';
	xor ^= payload_length;
	xor ^= ':';

	xor ^= cmd;

	for (uint8_t i = payload_init ; i < payload_init + payload_length ; i++)
	{
		xor ^= payload[i];
	}

	return xor;
}
