#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

/**********************************************************************************/
/*********************************** Includes *************************************/
/**********************************************************************************/
#include <inttypes.h>
#include <string.h>

#include "stm32f1xx_hal.h"

#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"

#include "datatypes.h"

#include "ticker.h"
#include "adc.h"
#include "pwm.h"
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/*********************************** Defines **************************************/
/**********************************************************************************/

/******************************** Pines del LED ***********************************/
#define SYSTEM_LED_Pin 							GPIO_PIN_13
#define SYSTEM_LED_GPIO_Port					GPIOC
/**********************************************************************************/

/******************************** Pines del PID ***********************************/
#define SYSTEM_PID_Pin 							GPIO_PIN_8
#define SYSTEM_PID_GPIO_Port					GPIOA
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

/***************************** Guardado en la flash *******************************/
#define SYSTEM_FLASH_SAVE_DATA_DISABLED			0x00
#define SYSTEM_FLASH_SAVE_DATA_ENABLED			0xFF
/**********************************************************************************/

/****************************** Estado del control ********************************/
#define SYSTEM_CONTROL_STATE_OFF				0x00
#define SYSTEM_CONTROL_STATE_ON					0xFF
/**********************************************************************************/

/************************************* PID ****************************************/
#define SYSTEM_CONTROL_ERROR_MAX				3400

#define SYSTEM_CONTROL_BASE_SPEED				30
#define SYSTEM_CONTROL_MAX_SPEED				70

#define SYSTEM_CONTROL_RES_MS					50
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/*********************************** Typedef **************************************/
/**********************************************************************************/

/***************************** Estructura de control ******************************/
typedef struct
{
	uint8_t state;

	int8_t vel_mot_der;
	int8_t vel_mot_izq;

	int16_t error;
	int16_t error_vel;

	uint16_t p;
	uint16_t i;
	uint16_t d;
}system_control_t;
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
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

/*
 * Funcion que envia los datos del ADC a la PC
 *
 * Cada un cierto periodo de tiempo seteado con un comando,
 * se envian los datos almacenados del ADC.
 *
 */
void system_adc_send_data(void);

/*
 * Función encargada de resolver el recorrido de la pista
 *
 * Esta funcion se encarga de reaccionar al recorrido de los
 * motores y sensores de auto para poder corregir su trayectoria.
 *
 */
void system_pid_control(void);

/*
 * Funcion que envia los datos del error a la PC
 *
 * Cada un cierto periodo de tiempo seteado con un comando,
 * se envian los datos almacenados del error del PID.
 *
 */
void system_error_send_data(void);

/*
 * Funcion que activa o desactida el PID
 *
 */
void system_control_state_button(void);
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

#endif
