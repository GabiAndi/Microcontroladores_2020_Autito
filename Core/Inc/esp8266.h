#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

/**********************************************************************************/
/*********************************** Includes *************************************/
/**********************************************************************************/
#include "system.h"
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/*********************************** Defines **************************************/
/**********************************************************************************/

/************************** Lectura de datos de la ESP ****************************/
#define ESP_DATA_WAIT_INIT					0
#define ESP_DATA_WAIT_END_AT				1
#define ESP_DATA_AT_COMMAND					2
#define ESP_DATA_WAIT_SEND					3
#define ESP_DATA_WAIT_STATE					4
#define ESP_DATA_WAIT_END_STATE				5
#define ESP_DATA_STATE						6
#define ESP_DATA_RECEIVE					7
/**********************************************************************************/

/****************************** Estados de inicio *********************************/
#define ESP_STATUS_NO_INIT					0
#define ESP_STATUS_INIT						1
/**********************************************************************************/

/****************************** Modo de operacion *********************************/
#define ESP_STATION_NO_INIT					0
#define ESP_STATION_INIT					1
/**********************************************************************************/

/******************************* Modo de conexion *********************************/
#define ESP_DISCONNECTED					0
#define ESP_CONNECTED						1
#define ESP_CONNECTED_GOT_IP				2
#define ESP_CONNECTED_SET_IP				3
/**********************************************************************************/

/**************************** Estado del socket UDP *******************************/
#define ESP_UDP_NO_INIT						0
#define ESP_UDP_INIT						1
/**********************************************************************************/

/******************************* Estado de envio **********************************/
#define ESP_SEND_NO_INIT					0
#define ESP_SEND_WAITING_OK					1
#define ESP_SEND_READY						2
#define ESP_SEND_SENDING					3
#define ESP_SEND_OK							4
/**********************************************************************************/

/*********************************** Errores **************************************/
#define ESP_ERROR_OK						0
#define ESP_ERROR_INIT						1
#define ESP_ERROR_CWMODE					2
#define ESP_ERROR_CIPSTA					3
#define ESP_ERROR_CON_TIMEOUT				4
#define ESP_ERROR_CON_PSW					5
#define ESP_ERROR_CON_NO_AP					6
#define ESP_ERROR_CON_FAIL					7
#define ESP_ERROR_UDP						8
#define ESP_ERROR_UDP_CLOSE					9
#define ESP_ERROR_CMD_SEND					10
#define ESP_ERROR_SEND_DATA					11
#define ESP_ERROR_CWJAP						12
/**********************************************************************************/

/*********************************** Comandos *************************************/
#define ESP_COMMAND_IDLE					0
#define ESP_COMMAND_AT						1
#define ESP_COMMAND_AT_CWMODE				2
#define ESP_COMMAND_AT_CWJAP				3
#define ESP_COMMAND_AT_CIPSTA				4
#define ESP_COMMAND_AT_CIPSTATUS			5
#define ESP_COMMAND_AT_CIPCLOSE				6
#define ESP_COMMAND_AT_CIPSTART				7
#define ESP_COMMAND_AT_CIPSEND				8
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/*********************************** Typedef **************************************/
/**********************************************************************************/

/***************************** Manejador de la esp ********************************/
typedef struct
{
	uint8_t status;
	uint8_t station;
	uint8_t error;
	uint8_t connected;
	uint8_t udp;
	uint8_t send;

	uint8_t read_state;

	uint8_t cmd;

	uint8_t cmd_init;
	uint8_t cmd_end;

	volatile uint8_t byte_receibe_usart;

	uint8_t send_data_length;

	uint8_t auto_connection;

	char len_char[4];
	uint8_t len_uint;
}esp_manager_t;
/**********************************************************************************/

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/

/*
 * Inicia y configura la ESP
 *
 * Manda un hard reset a la placa y activa el autoconectado.
 *
 */
void esp_init(void);

/*
 * Funcion de lectura de datos de la ESP
 *
 * Se encarga de verificar y analizar los datos que provengan de la esp.
 *
 */
void esp_read_pending(void);

/*
 * Funcion de envio de datos a la ESP
 *
 * Se encarga de veirificar si hay datos disponibles para enviar.
 *
 */
void esp_write_pending(void);

/*
 * Funcion de envio de comandos via UDP
 *
 * Esta funcion se encarga de preparar la ESP para enviar datos
 * a travez de UDP.
 *
 */
void esp_write_send_data_pending(void);

/*
 * Funcion de comparacion de comandos AT
 *
 * Verifica si el comando que llego de la ESP corresponde con alguno esperado.
 *
 */
uint8_t esp_at_cmp(uint8_t *at, uint8_t at_init, uint8_t at_end, uint8_t *at_cmp, uint8_t at_cmp_length);

/*
 * Funcion de timeout de lectura
 *
 * Esta funcion se activa si el paquete tardo mucho en recibirse completamente.
 *
 */
void esp_timeout_read(void);

/*
 * Funcion de timeout de escritura
 *
 * Esta funcion se activa si el paquete tardo mucho en enviarse.
 *
 */
void esp_timeout_write(void);

/*
 * Funcion de autoconectado
 *
 * Esta funcion se llama mediante un ticker para lograr la conexión con el AP
 * configurado.
 *
 */
void esp_connect_to_ap(void);

/*
 * Funcion que inicia un hard reset
 *
 * Esta funcion resetea la ESP mandando un pulso bajo durante ms configurables
 * en el ticker de llamada.
 *
 */
void esp_hard_reset(void);

/*
 * Funcion que detiene el hard reset
 *
 * Esta funcion se llama para volver a establecer el pin a alto de la ESP.
 *
 */
void esp_hard_reset_stop(void);

/*
 * Funcion de guardia de la ESP
 *
 * Esta funcion se encarga de capturar los eventos de la ESP y accionar a ellos.
 * Esto se puede deber a errores durante algun comando o envio de datos o
 * simplemente la ESP no respondio.
 *
 */
void esp_guardian_status(void);

/*
 * Funcion que envia los datos del ADC a la PC
 *
 * Cada un cierto periodo de tiempo seteado con un comando,
 * se envian los datos almacenados del ADC via UDP.
 *
 */
void esp_send_adc_sensor_data(void);
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

#endif /* INC_ESP8266_H_ */
