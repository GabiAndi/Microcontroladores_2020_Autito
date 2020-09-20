#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#include <inttypes.h>

#include "usbcdc.h"
#include "esp8266.h"

#include "ticker.h"

#define LED_OK							1000
#define LED_FAIL						50

#define DEBUG_OFF						0
#define DEBUG_ON						1

uint8_t debug;

void system_init(void);

uint8_t xor(uint8_t cmd, uint8_t *payload, uint8_t payload_init, uint8_t payload_length);

#endif
