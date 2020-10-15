#ifndef INC_ADC_H_
#define INC_ADC_H_

// Includes
#include "system.h"

#define ADC_BUFFER_LENGTH			100

#define ADC_MEAN_STEP				5

// typedef
typedef struct
{
	volatile uint16_t data[ADC_BUFFER_LENGTH][6];

	volatile uint16_t mean[6];

	volatile uint16_t batery;

	volatile uint32_t mean_aux;

	volatile uint8_t data_index;

	volatile uint8_t send_esp;
	volatile uint8_t send_usb;

	volatile uint8_t send_batery_esp;
	volatile uint8_t send_batery_usb;
}adc_buffer_t;

// Funciones
void adc_init(void);

void adc_capture(void);

#endif /* INC_ADC_H_ */
