#ifndef INC_USBCDC_H_
#define INC_USBCDC_H_

// Includes
#include "system.h"

// Typedef
typedef struct
{
	volatile uint8_t data[256];
	volatile uint8_t read_index;
	volatile uint8_t write_index;

	volatile uint8_t read_state;
	uint8_t payload_length;
	uint8_t payload_init;
}usbcdc_buffer_read_t;

typedef struct
{
	volatile uint8_t data[256];
	volatile uint8_t read_index;
	volatile uint8_t write_index;
}usbcdc_buffer_write_t;

// Variables
usbcdc_buffer_read_t usbcdc_buffer_read;
usbcdc_buffer_write_t usbcdc_buffer_write;

// Funciones
void usbcdc_init(void);

void usbcdc_write_buffer_write(uint8_t *data, uint8_t length);
void usbcdc_write_buffer_read(uint8_t *data, uint8_t length);

void usbcdc_send_cmd(uint8_t cmd, uint8_t *payload, uint8_t length);

void usbcdc_read_pending(void);
void usbcdc_write_pending(void);

void usbcdc_timeout(void);

#endif /* INC_USBCDC_H_ */