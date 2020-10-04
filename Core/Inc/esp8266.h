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
#define ESP_STATUS_UDP_READY				7
#define ESP_STATUS_READY_SEND				8
#define ESP_STATUS_WAIT_SENDING				9
#define ESP_STATUS_SENDING					10
#define ESP_STATUS_SEND_OK					11

#define ESP_STATUS_ERROR_INIT				100
#define ESP_STATUS_ERROR_CWMODE				101
#define ESP_STATUS_ERROR_CIPSTA				102
#define ESP_STATUS_ERROR_CON_TIMEOUT		103
#define ESP_STATUS_ERROR_CON_PSW			104
#define ESP_STATUS_ERROR_CON_NO_AP			105
#define ESP_STATUS_ERROR_CON_FAIL			106
#define ESP_STATUS_ERROR_UDP				107
#define ESP_STATUS_ERROR_CMD_SEND			108
#define ESP_STATUS_ERROR_SEND_DATA			109

#define ESP_STATUS_FAIL_CWJAP				200

#define ESP_COMMAND_IDLE					0
#define ESP_COMMAND_AT						1
#define ESP_COMMAND_AT_CWMODE				2
#define ESP_COMMAND_AT_CWJAP				3
#define ESP_COMMAND_AT_CIPSTA				4
#define ESP_COMMAND_AT_CIPSTATUS			5
#define ESP_COMMAND_AT_CIPCLOSE				6
#define ESP_COMMAND_AT_CIPSTART				7
#define ESP_COMMAND_AT_CIPSEND				8

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

	uint8_t read_cmd_index;

	uint8_t cmd;

	uint8_t cmd_init;
	uint8_t cmd_end;

	uint8_t send_data_length;

	char len_char[4];
	uint8_t len_uint;
}esp_manager_t;

// Funciones
void esp_init(void);

void esp_write_buffer_write(uint8_t *data, uint8_t length);
void esp_write_buffer_send_data_write(uint8_t *data, uint8_t length);
void esp_write_buffer_read(uint8_t *data, uint8_t length);

void esp_send_at(uint8_t *cmd, uint8_t length);
void esp_send_cmd(uint8_t cmd, uint8_t *payload, uint8_t length);

void esp_read_pending(void);
void esp_write_pending(void);
void esp_write_send_data_pending(void);

uint8_t esp_at_cmp(uint8_t *at, uint8_t at_init, uint8_t at_end, uint8_t *at_cmp, uint8_t at_cmp_length);

void esp_timeout_read(void);
void esp_timeout_send(void);

void esp_connect_to_ap(void);

void esp_hard_reset(void);
void esp_hard_reset_stop(void);

void esp_guardian_status(void);

void send_adc_data_esp(void);

#endif /* INC_ESP8266_H_ */
