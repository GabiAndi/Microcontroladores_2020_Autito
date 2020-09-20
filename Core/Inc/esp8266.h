#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#include "system.h"

// Definiciones
#define WIFI_STATUS_NO_INIT				0
#define WIFI_STATUS_INIT				1
#define WIFI_STATUS_STATION				2
#define WIFI_STATUS_CONNECTED			3
#define WIFI_STATUS_DISCONNECTED		4
#define WIFI_STATUS_GOT_IP				5
#define WIFI_STATUS_SET_IP				6
#define WIFI_STATUS_READY				7
#define WIFI_STATUS_BUSY				254

#define WIFI_COMMAND_ERROR				0
#define WIFI_COMMAND_AT					1
#define WIFI_COMMAND_CWMODE_CUR_1		2
#define WIFI_COMMAND_CWJAP_CUR			3
#define WIFI_COMMAND_CIPSTA_CUR			4
#define WIFI_COMMAND_CIPSTATUS			5

// Typedef
typedef struct
{
	volatile uint8_t data[256];
	volatile uint8_t read_index;
	volatile uint8_t write_index;

	volatile uint8_t read_state;
	uint8_t payload_length;
	uint8_t payload_init;
}esp_buffer_read_t;

typedef struct
{
	volatile uint8_t data[256];
	volatile uint8_t read_index;
	volatile uint8_t write_index;
}esp_buffer_write_t;

typedef struct
{
	uint8_t status;

	uint8_t *ip_mcu;
	uint8_t *ip_pc;

	uint8_t cmd;

	uint8_t cmd_init;
	uint8_t cmd_end;
}esp_manager_t;

// Varialbles
esp_buffer_read_t esp_buffer_read;
esp_buffer_write_t esp_buffer_write;

esp_manager_t esp_manager;

// Byte temporal de recepción de datos
volatile uint8_t byte_receibe_usart;

// Funciones
void esp_init(void);

void esp_write_buffer_write(uint8_t *data, uint8_t length);
void esp_write_buffer_read(uint8_t *data, uint8_t length);

void esp_send_at(uint8_t *cmd, uint8_t length);
void esp_send_cmd(uint8_t cmd, uint8_t *payload, uint8_t length);

void esp_read_pending(void);
void esp_write_pending(void);

uint8_t command_at(uint8_t *cmd, uint8_t init, uint8_t end, uint8_t *cmd_cmp);

void esp_timeout(void);

#endif /* INC_ESP8266_H_ */
