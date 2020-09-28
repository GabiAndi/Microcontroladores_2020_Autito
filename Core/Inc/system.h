#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#include <inttypes.h>
#include <stdio.h>

#include "usbcdc.h"
#include "esp8266.h"

#include "ticker.h"

#define LED_OK							1000
#define LED_FAIL						50

#define DEBUG_OFF						0
#define DEBUG_ON						1

// Estructuras de datos en la flash
typedef struct
{
	uint8_t ip_mcu[20];
	uint8_t ip_mcu_length;

	uint8_t ip_pc[20];
	uint8_t ip_pc_length;

	uint8_t ssid[20];
	uint8_t ssid_length;

	uint8_t psw[20];
	uint8_t psw_length;

	uint8_t port[10];
	uint8_t port_length;

	uint8_t padding[1024 - 21 - 21 - 21 - 21 - 11];
}__attribute__ ((packed)) flash_data_t;

void system_init(void);

uint8_t xor(uint8_t cmd, uint8_t *payload, uint8_t payload_init, uint8_t payload_length);

#endif
