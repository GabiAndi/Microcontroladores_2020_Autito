#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

/* 1) ¡BUG!
 *
 * Cuando recibimos un comando AT como dato por parte del comando 0xF2,
 * debido a la condicion de corte en el case 1 de la recepción del UDP,
 * el paquete se corta antes de tiempo debido al \r \n
 *
 */

/* 2) ¡AGREGAR!
 *
 * Si se desconecta la esp se deberia iniciar otra vez el autonectado
 *
 */

/* 3) ¡AGREGAR!
 *
 * Es necesario un control de integridad para verificar si los datos
 * contenidos en la flash son valores aleatorios o iniciales
 *
 */

/* 4) ¡MEJORAR!
 *
 * Atoi en la lectura de datos
 *
 */

/* 5) ¡VER MEJORA!
 *
 * En la copia de datos del buffer de comandos de esp al buffer de envio
 * ver la posibilidad de mejorar el while de copia
 *
 */

#include <inttypes.h>
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

#define LED_OK							1000
#define LED_FAIL						50

#define DEBUG_OFF						0
#define DEBUG_ON						1

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

void system_init(void);

uint8_t xor(uint8_t cmd, uint8_t *payload, uint8_t payload_init, uint8_t payload_length);

HAL_StatusTypeDef save_flash_data();

#endif
