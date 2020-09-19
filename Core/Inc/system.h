#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#define WIFI_DEBUG_OFF					0
#define WIFI_DEBUG_ON					1

#define WIFI_STATUS_DISCONNECT			0
#define WIFI_STATUS_CONNECT				1
#define WIFI_STATUS_INIT				3
#define WIFI_STATUS_NO_INIT				4
#define WIFI_STATUS_UDP_INIT			5
#define WIFI_STATUS_UDP_NO_INIT			6
#define WIFI_STATUS_FAIL				7

#define WIFI_COMMAND_NO_FOUND			0
#define WIFI_COMMAND_AT					1
#define WIFI_COMMAND_CIPSTATUS			2

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
	uint8_t command;
}wifi_manager_t;

read_ring_buffer_t read_buffer_USB;
read_ring_buffer_t read_buffer_UDP;
write_ring_buffer_t write_buffer_USB;
write_ring_buffer_t write_buffer_UDP;

wifi_manager_t wifi;

#endif
