#include "adc.h"

adc_buffer_t adc_buffer;

extern ADC_HandleTypeDef hadc1;

void adc_init(void)
{
	adc_buffer.send_esp = ADC_SEND_DATA_OFF;
	adc_buffer.data_index = 0;

	ticker_new(adc_start_ticker, 2, TICKER_LOW_PRIORITY);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{

}

void adc_start_ticker(void)
{
	if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)(&adc_buffer.data[adc_buffer.data_index]), 6) == HAL_OK)
	{
		adc_buffer.data_index++;

		if (adc_buffer.data_index >= ADC_BUFFER_LENGTH)
		{
			adc_buffer.data_index = 0;
		}
	}
}
