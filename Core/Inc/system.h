#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

// Includes
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

// Definiciones
// Led de estado
#define LED_OK							1000
#define LED_FAIL						50
// Modo de debug
#define DEBUG_OFF						0
#define DEBUG_ON						1
// Fuente del paquete recibido
#define DEVICE_USB						0
#define DEVICE_UDP						1
// Envio de datos
#define ADC_SEND_DATA_OFF				0x00
#define ADC_SEND_DATA_ON				0xFF
// Guardado en la flash
#define FLASH_SAVE_DATA_DISABLED		0x00
#define FLASH_SAVE_DATA_ENABLED			0xFF

// Estructuras de datos en la flash
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
}__attribute__ ((packed)) flash_data_t;

// Union para convertir datos
typedef union
{
	volatile uint8_t u8[4];
	volatile uint16_t u16[2];
	volatile uint32_t u32;

	volatile int8_t i8[4];
	volatile int16_t i16[2];
	volatile int32_t i32;

	volatile float f;
}byte_converter_u;

// Buffers
typedef struct
{
	volatile uint8_t data[256];
	volatile uint8_t read_index;
	volatile uint8_t write_index;
}system_ring_buffer_t;

// Manejador de comandos
typedef struct
{
	// Lectura
	system_ring_buffer_t *buffer_read;

	uint8_t read_state;
	uint8_t read_payload_init;
	uint8_t read_payload_length;

	ticker_t read_time_out;

	// Escritura
	system_ring_buffer_t *buffer_write;

	ticker_t write_time_out;

	// Conversion de datos
	byte_converter_u byte_converter;
}system_cmd_manager_t;

void system_init(void);

void system_led_status(void);

void system_buffer_write(system_ring_buffer_t *buffer, uint8_t *data, uint8_t length);

uint8_t system_data_package(system_cmd_manager_t *cmd_manager);

uint8_t check_xor(uint8_t *data, uint8_t init, uint8_t length);

HAL_StatusTypeDef save_flash_data(void);
uint8_t check_flash_data_integrity(flash_data_t *flash_data);

void system_guardian_flash(void);

#endif
