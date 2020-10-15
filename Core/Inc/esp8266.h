#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

// Includes
#include "system.h"

// Definiciones
// Estado de lectura de datos de la ESP
#define ESP_DATA_WAIT_INIT					0
#define ESP_DATA_WAIT_END_AT				1
#define ESP_DATA_AT_COMMAND					2
#define ESP_DATA_WAIT_SEND					3
#define ESP_DATA_WAIT_STATE					4
#define ESP_DATA_WAIT_END_STATE				5
#define ESP_DATA_STATE						6
#define ESP_DATA_RECEIVE					7
// Estados de inicio
#define ESP_STATUS_NO_INIT					0
#define ESP_STATUS_INIT						1
// Modo de operacion
#define ESP_STATION_NO_INIT					0
#define ESP_STATION_INIT					1
// Modo de conexion
#define ESP_DISCONNECTED					0
#define ESP_CONNECTED						1
#define ESP_CONNECTED_GOT_IP				2
#define ESP_CONNECTED_SET_IP				3
// Estado del socket UDP
#define ESP_UDP_NO_INIT						0
#define ESP_UDP_INIT						1
// Estado de envio
#define ESP_SEND_NO_INIT					0
#define ESP_SEND_WAITING_OK					1
#define ESP_SEND_READY						2
#define ESP_SEND_SENDING					3
#define ESP_SEND_OK							4
// Errores
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
// Comandos
#define ESP_COMMAND_IDLE					0
#define ESP_COMMAND_AT						1
#define ESP_COMMAND_AT_CWMODE				2
#define ESP_COMMAND_AT_CWJAP				3
#define ESP_COMMAND_AT_CIPSTA				4
#define ESP_COMMAND_AT_CIPSTATUS			5
#define ESP_COMMAND_AT_CIPCLOSE				6
#define ESP_COMMAND_AT_CIPSTART				7
#define ESP_COMMAND_AT_CIPSEND				8

// Tipos de datos
typedef struct
{
	// Flags de estados
	uint8_t status;
	uint8_t station;
	uint8_t error;
	uint8_t connected;
	uint8_t udp;
	uint8_t send;

	// Lectura de datos recibidos por UDP
	uint8_t read_state;

	uint8_t cmd;

	uint8_t cmd_init;
	uint8_t cmd_end;

	// Byte de recepcion
	volatile uint8_t byte_receibe_usart;

	// Envio de datos por UDP
	uint8_t send_data_length;

	// Autoconectado
	uint8_t auto_connection;

	// Variables de conversion de tamaños
	char len_char[4];
	uint8_t len_uint;
}esp_manager_t;

// Funciones
void esp_init(void);

void esp_send_at(uint8_t *cmd, uint8_t length);
void esp_send_cmd(uint8_t cmd, uint8_t *payload, uint8_t length);

void esp_read_pending(void);
void esp_write_pending(void);
void esp_write_send_data_pending(void);

uint8_t esp_at_cmp(uint8_t *at, uint8_t at_init, uint8_t at_end, uint8_t *at_cmp, uint8_t at_cmp_length);

void esp_timeout_read(void);
void esp_timeout_write(void);

void esp_connect_to_ap(void);

void esp_hard_reset(void);
void esp_hard_reset_stop(void);

void esp_guardian_status(void);

void esp_send_adc_sensor_data(void);
/*void esp_send_adc_batery_data(void);*/

#endif /* INC_ESP8266_H_ */
