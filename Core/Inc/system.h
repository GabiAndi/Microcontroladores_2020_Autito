#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#define LED_OK							1000
#define LED_FAIL						50

#define WIFI_DEBUG_OFF					0
#define WIFI_DEBUG_ON					1

#define WIFI_STATUS_NO_INIT				0
#define WIFI_STATUS_INIT				1
#define WIFI_STATUS_STATION				2
#define WIFI_STATUS_CONNECTED			3
#define WIFI_STATUS_DISCONNECTED		4
#define WIFI_STATUS_GOT_IP				5
#define WIFI_STATUS_SET_IP				6
#define WIFI_STATUS_BUSY				254

#define WIFI_COMMAND_ERROR				0
#define WIFI_COMMAND_AT					1
#define WIFI_COMMAND_CWMODE_CUR_1		2
#define WIFI_COMMAND_CWJAP_CUR			3
#define WIFI_COMMAND_CIPSTA_CUR			4

typedef struct write_ring_buffer
{
	volatile uint8_t data[256];
	volatile uint8_t read_index;
	volatile uint8_t write_index;
}write_ring_buffer_t;

typedef struct read_ring_buffer
{
	volatile uint8_t data[256];
	volatile uint8_t read_index;
	volatile uint8_t write_index;

	volatile uint8_t read_state;
	uint8_t payload_length;
	uint8_t payload_init;
}read_ring_buffer_t;

typedef struct wifi_manager
{
	uint8_t status;

	char *ip_mcu;
	char *ip_pc;

	uint8_t debug;

	uint8_t cmd;

	uint8_t cmd_init;
	uint8_t cmd_end;
}wifi_manager_t;

read_ring_buffer_t read_buffer_USB;
read_ring_buffer_t read_buffer_UDP;
write_ring_buffer_t write_buffer_USB;
write_ring_buffer_t write_buffer_UDP;

wifi_manager_t wifi;

#endif
