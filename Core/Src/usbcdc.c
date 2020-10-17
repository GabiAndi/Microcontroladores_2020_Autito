#include "usbcdc.h"

/**********************************************************************************/
/********************************** Variables *************************************/
/**********************************************************************************/

/*********************************** Tickers **************************************/
ticker_t usbcdc_ticker_send_adc_data;
/**********************************************************************************/

/***************************** Bufferes de datos **********************************/
system_ring_buffer_t usbcdc_buffer_read;
system_ring_buffer_t usbcdc_buffer_write;

extern adc_buffer_t adc_buffer;
/**********************************************************************************/

/*************************** Manejador de comandos ********************************/
system_cmd_manager_t usbcdc_cmd_manager;
/**********************************************************************************/

/***************************** Depuracion via USB *********************************/
extern uint8_t system_usb_debug;
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/
void usbcdc_init(void)
{
	/***********************************************************************************/
	/************************* Inicializacion de los bufferes **************************/
	/***********************************************************************************/

	/**************************** Buffer de lectura del USB ****************************/
	usbcdc_buffer_read.read_index = 0;
	usbcdc_buffer_read.write_index = 0;
	/***********************************************************************************/

	/*************************** Buffer de escritura del USB ***************************/
	usbcdc_buffer_write.read_index = 0;
	usbcdc_buffer_write.write_index = 0;
	/***********************************************************************************/

	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	/***********************************************************************************/
	/************************* Inicializacion de los comandos **************************/
	/***********************************************************************************/
	usbcdc_cmd_manager.buffer_read = &usbcdc_buffer_read;

	usbcdc_cmd_manager.read_state = 0;
	usbcdc_cmd_manager.read_payload_init = 0;
	usbcdc_cmd_manager.read_payload_length = 0;

	usbcdc_cmd_manager.read_time_out.ms_max = 100;
	usbcdc_cmd_manager.read_time_out.ms_count = 0;
	usbcdc_cmd_manager.read_time_out.calls = 0;
	usbcdc_cmd_manager.read_time_out.active = TICKER_NO_ACTIVE;
	usbcdc_cmd_manager.read_time_out.priority = TICKER_LOW_PRIORITY;
	usbcdc_cmd_manager.read_time_out.ticker_function = usbcdc_timeout_read;

	ticker_new(&usbcdc_cmd_manager.read_time_out);

	usbcdc_cmd_manager.buffer_write = &usbcdc_buffer_write;

	usbcdc_cmd_manager.byte_converter.u32 = 0;
	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/

	/***********************************************************************************/
	/************************** Inicializacion de los tickers **************************/
	/***********************************************************************************/

	/***************************** Ticker para envio de datos del adc ******************************/
	usbcdc_ticker_send_adc_data.ms_count = 0;
	usbcdc_ticker_send_adc_data.ms_max = 500;
	usbcdc_ticker_send_adc_data.calls = 0;
	usbcdc_ticker_send_adc_data.priority = TICKER_LOW_PRIORITY;
	usbcdc_ticker_send_adc_data.ticker_function = usbcdc_send_adc_data;
	usbcdc_ticker_send_adc_data.active = TICKER_NO_ACTIVE;

	ticker_new(&usbcdc_ticker_send_adc_data);
	/***********************************************************************************/

	/***********************************************************************************/
	/***********************************************************************************/
	/***********************************************************************************/
}

void usbcdc_read_pending(void)
{
	if ((usbcdc_buffer_read.read_index != usbcdc_buffer_read.write_index))
	{
		system_data_package(&usbcdc_cmd_manager);

		usbcdc_buffer_read.read_index++;
	}
}

void usbcdc_write_pending(void)
{
	if (usbcdc_buffer_write.read_index != usbcdc_buffer_write.write_index)
	{
		if (CDC_Transmit_FS((uint8_t *)(&usbcdc_buffer_write.data[usbcdc_buffer_write.read_index]), 1) == USBD_OK)
		{
			usbcdc_buffer_write.read_index++;
		}
	}
}

void usbcdc_timeout_read(void)
{
	usbcdc_cmd_manager.read_time_out.active = TICKER_NO_ACTIVE;

	usbcdc_cmd_manager.read_state = 0;
}

void usbcdc_send_adc_data(void)
{
	// Calculo la media de los datos almacenados en el buffer
	/*for (uint8_t i = 0 ; i < 6 ; i++)
	{
		adc_buffer.mean[i] = 0;

		for (uint8_t j = 0 ; j < ADC_BUFFER_LENGTH ; j += ADC_MEAN_STEP)
		{
			adc_buffer.mean[i] += adc_buffer.data[j][i];
		}

		adc_buffer.mean[i] = (uint16_t)(adc_buffer.mean[i] / (ADC_BUFFER_LENGTH / ADC_MEAN_STEP));
	}

	uint8_t init_index;

	usbcdc_write_buffer_write((uint8_t *)("UNER"), 4);

	ack = 13;
	usbcdc_write_buffer_write(&ack, 1);

	usbcdc_write_buffer_write((uint8_t *)(":"), 1);

	ack = 0xC0;
	usbcdc_write_buffer_write(&ack, 1);

	init_index = usbcdc_buffer_write.write_index;

	usbcdc_write_buffer_write((uint8_t *)(&adc_buffer.send_esp), 1);

	for (uint8_t i = 0 ; i < 6 ; i++)
	{
		byte_translate.u16[0] = adc_buffer.mean[i];

		usbcdc_write_buffer_write((uint8_t *)(&byte_translate.u8[0]), 1);
		usbcdc_write_buffer_write((uint8_t *)(&byte_translate.u8[1]), 1);
	}

	uint8_t checksum = check_xor(ack, (uint8_t *)(&usbcdc_buffer_write.data), init_index, 13);

	usbcdc_write_buffer_write(&checksum, 1);*/
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
