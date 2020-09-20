#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#include "system.h"

// Definiciones
#define ESP_STATUS_NO_INIT					0
#define ESP_STATUS_INIT						1
#define ESP_STATUS_STATION_OK				2
#define ESP_STATUS_CONNECTED				3
#define ESP_STATUS_DISCONNECTED				4
#define ESP_STATUS_CONNECTED_GOT_IP			5
#define ESP_STATUS_SET_IP					6

#define ESP_COMMAND_IDLE					0
#define ESP_COMMAND_AT						1
#define ESP_COMMAND_AT_CWMODE				2
#define ESP_COMMAND_AT_CWJAP				3
#define ESP_COMMAND_AT_CIPSTA				4

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
	uint8_t read_state;

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

uint8_t esp_at_cmp(uint8_t *at, uint8_t at_init, uint8_t at_end, uint8_t *at_cmp, uint8_t at_cmp_length);

void esp_timeout(void);

#endif /* INC_ESP8266_H_ */
