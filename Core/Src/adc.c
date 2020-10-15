#include "adc.h"

// Variables
// Buffers
adc_buffer_t adc_buffer;

// Tickers
ticker_t ticker_adc_capture;

// Estructura de control de la HAL
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

void adc_init(void)
{
	// Inicializacion del buffer
	adc_buffer.send_usb = ADC_SEND_DATA_OFF;
	adc_buffer.send_esp = ADC_SEND_DATA_OFF;

	adc_buffer.send_batery_usb = ADC_SEND_DATA_OFF;
	adc_buffer.send_batery_esp = ADC_SEND_DATA_OFF;

	adc_buffer.data_index = 0;

	// Ticker para la captura de datos
	ticker_adc_capture.ms_count = 0;
	ticker_adc_capture.ms_max = 2;
	ticker_adc_capture.calls = 0;
	ticker_adc_capture.priority = TICKER_LOW_PRIORITY;
	ticker_adc_capture.ticker_function = adc_capture;
	ticker_adc_capture.active = TICKER_ACTIVE;

	ticker_new(&ticker_adc_capture);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1)
	{
		adc_buffer.data_index++;

		if (adc_buffer.data_index >= ADC_BUFFER_LENGTH)
		{
			adc_buffer.data_index = 0;
		}
	}

	/*if (hadc->Instance == ADC2)
	{
		adc_buffer.batery = HAL_ADC_GetValue(&hadc2);
	}*/
}

void adc_capture(void)
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)(&adc_buffer.data[adc_buffer.data_index]), 6);

	/*HAL_ADC_Start_IT(&hadc2);*/
}
