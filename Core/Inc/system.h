#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

typedef struct write_ring_buffer
{
	uint8_t data[256];
	uint8_t read_index;
	uint8_t write_index;
}write_ring_buffer_t;

typedef struct read_ring_buffer
{
	uint8_t data[256];
	uint8_t read_index;
	uint8_t write_index;

	uint8_t read_state;
	uint8_t payload_length;
	uint8_t payload_init;
}read_ring_buffer_t;

read_ring_buffer_t read_buffer_USB;
read_ring_buffer_t read_buffer_UDP;
write_ring_buffer_t write_buffer_USB;
write_ring_buffer_t write_buffer_UDP;

#endif
