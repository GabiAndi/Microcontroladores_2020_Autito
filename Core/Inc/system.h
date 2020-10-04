#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

/* 1) ¡AGREGAR! (Preguntar a German)
 *
 * Es necesario un control de integridad para verificar si los datos
 * contenidos en la flash son valores aleatorios o iniciales
 *
 */

/* 2) ¡MEJORAR!
 *
 * Atoi en la lectura de datos
 *
 */

/* 3) ¡VER MEJORA!
 *
 * En la copia de datos del buffer de comandos de esp al buffer de envio
 * ver la posibilidad de mejorar el while de copia
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"

#include "usbd_cdc_if.h"

#include "ticker.h"

#include "usbcdc.h"
#include "esp8266.h"
#include "adc.h"

#define LED_OK							1000
#define LED_FAIL						50

#define DEBUG_OFF						0
#define DEBUG_ON						1

#define ADC_SEND_DATA_OFF				0x00
#define ADC_SEND_DATA_ON				0xFF

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

	uint8_t checksum;

	uint8_t padding[1024 - 31 - 31 - 21 - 21 - 11 - 1];
}__attribute__ ((packed)) flash_data_t;

typedef union
{
	volatile uint8_t u8[4];
	volatile uint16_t u16[2];
	volatile uint32_t u32;

	volatile int8_t i8[4];
	volatile int16_t i16[2];
	volatile int32_t i32;
}byte_translate_u;

void system_init(void);

void system_led_status(void);

uint8_t xor(uint8_t cmd, uint8_t *payload, uint8_t payload_init, uint8_t payload_length);

HAL_StatusTypeDef save_flash_data(void);
uint8_t check_flash_data_integrity(flash_data_t *flash_data);

#endif
