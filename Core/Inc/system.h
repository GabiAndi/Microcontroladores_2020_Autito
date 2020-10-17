#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

/**********************************************************************************/
/*********************************** Includes *************************************/
/**********************************************************************************/
#include <string.h>
#include <inttypes.h>

#include "main.h"

#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"

#include "usbd_cdc_if.h"

#include "ticker.h"
#include "usbcdc.h"
#include "esp8266.h"
#include "adc.h"
#include "pwm.h"
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/*********************************** Defines **************************************/
/**********************************************************************************/

/******************************** Led de estado ***********************************/
#define SYSTEM_LED_FAIL							50
#define SYSTEM_LED_INIT							500
#define SYSTEM_LED_OK							2000
/**********************************************************************************/

/******************************** Modo de debug ***********************************/
#define SYSTEM_USB_DEBUG_OFF					0x00
#define SYSTEM_USB_DEBUG_ON						0xFF
/**********************************************************************************/

/******************************* Envio de datos ***********************************/
#define SYSTEM_ADC_SEND_DATA_OFF				0x00
#define SYSTEM_ADC_SEND_DATA_ON					0xFF
/**********************************************************************************/

/***************************** Guardado en la flash *******************************/
#define SYSTEM_FLASH_SAVE_DATA_DISABLED			0x00
#define SYSTEM_FLASH_SAVE_DATA_ENABLED			0xFF
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/*********************************** Typedef **************************************/
/**********************************************************************************/

/****************************** Datos en la flash *********************************/
typedef struct
{
	uint8_t ssid[30];
	uint8_t ssid_length;

	uint8_t psw[30];
	uint8_t psw_length;

	uint8_t ip_mcu[20];
	uint8_t ip_mcu_length;

	uint8_t ip_pc[20];
	uint8_t ip_pc_length;

	uint8_t port[10];
	uint8_t port_length;

	uint8_t padding[1024 - 31 - 31 - 21 - 21 - 11 - 1];

	uint8_t checksum;
}__attribute__ ((packed)) system_flash_data_t;
/**********************************************************************************/

/******************************* Convertir datos **********************************/
typedef union
{
	volatile uint8_t u8[4];
	volatile uint16_t u16[2];
	volatile uint32_t u32;

	volatile int8_t i8[4];
	volatile int16_t i16[2];
	volatile int32_t i32;

	volatile float f;
}system_byte_converter_u;
/**********************************************************************************/

/*********************************** Bufferes *************************************/
typedef struct
{
	volatile uint8_t data[256];
	volatile uint8_t read_index;
	volatile uint8_t write_index;
}system_ring_buffer_t;
/**********************************************************************************/

/*********************************** Comandos *************************************/
typedef struct
{
	system_ring_buffer_t *buffer_read;

	uint8_t read_state;
	uint8_t read_payload_init;
	uint8_t read_payload_length;

	ticker_t read_time_out;

	system_ring_buffer_t *buffer_write;

	ticker_t write_time_out;

	system_byte_converter_u byte_converter;
}system_cmd_manager_t;
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/

/*
 * Funcion de inicio del sistema
 *
 * Esta funcion se encarga de iniciar todos los dispositivos y perifericos
 * del sistema. Tambien inicia los bufferes encargados de la comunicación, etc.
 *
 */
void system_init(void);

/*
 * Led de estado
 *
 * Da un indicador visual al usuario sobre el estado del autito.
 *
 */
void system_led_blink(void);

/*
 * Función que establece el estado del LED
 *
 */
void system_led_set_status(uint16_t status);

/*
 * Funcion que escribe en un buffer
 *
 * Escribe datos en un buffer sin necesidad de acceder a los indices
 * directamente.
 *
 */
void system_buffer_write(system_ring_buffer_t *buffer, uint8_t *data, uint8_t length);

/*
 * Funcion que escribe un comando en un buffer
 *
 * Escribe datos en un buffer sin necesidad de acceder a los indices
 * directamente.
 *
 */
void system_write_cmd(system_ring_buffer_t *buffer, uint8_t cmd, uint8_t *payload, uint8_t length);

/*
 * Funcion que se encarga de analizar un paquete
 *
 * Analiza y ejecuta los comandos recibidos por el paquete,
 * sin importar de que fuente provenga.
 *
 */
uint8_t system_data_package(system_cmd_manager_t *cmd_manager);

/*
 * Funcion que calcula el xor total
 *
 * Se calcula el xor total de un conjunto de datos de tamaño definido.
 *
 */
uint8_t system_check_xor(uint8_t *data, uint8_t init, uint8_t length);

/*
 * Guardado de datos en la flash
 *
 * Si se puede escribir datos en la flash y la suma de verificación
 * concuerda los datos son guardados.
 *
 */
HAL_StatusTypeDef system_flash_save_data(void);

/*
 * Verifica la integridad de datos de la flash
 *
 * Calcula el xor de los datos almacenados en la flash o ram.
 *
 */
uint8_t system_flash_check_integrity(system_flash_data_t *flash_data);

/*
 * Funcion que proteje la escritura de la flash
 *
 * No permite que la flash sea accedida para escritura en periodos
 * cortos de tiempo.
 *
 */
void system_flash_enable(void);
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

#endif
