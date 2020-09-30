#include "system.h"

// Variables
__attribute__ ((__section__(".user_data_flash"))) flash_data_t flash_user;	// Datos en flash
flash_data_t flash_user_ram;	// Datos en ram

uint8_t debug;

uint8_t request;

uint8_t i;
uint8_t j;

void system_init(void)
{
	//flash_user_ram = flash_user;	// Cargo los datos almacenados en la flash

	flash_user_ram.ssid_length = 10;

	flash_user_ram.ssid[0] = 'T';
	flash_user_ram.ssid[1] = 'P';
	flash_user_ram.ssid[2] = 'L';
	flash_user_ram.ssid[3] = 'I';
	flash_user_ram.ssid[4] = 'N';
	flash_user_ram.ssid[5] = 'K';
	flash_user_ram.ssid[6] = '_';
	flash_user_ram.ssid[7] = '2';
	flash_user_ram.ssid[8] = '4';
	flash_user_ram.ssid[9] = 'G';

	flash_user_ram.psw_length = 14;

	flash_user_ram.psw[0] = 'B';
	flash_user_ram.psw[1] = 'a';
	flash_user_ram.psw[2] = 's';
	flash_user_ram.psw[3] = 'e';
	flash_user_ram.psw[4] = 'x';
	flash_user_ram.psw[5] = 'B';
	flash_user_ram.psw[6] = '1';
	flash_user_ram.psw[7] = 'A';
	flash_user_ram.psw[8] = 'u';
	flash_user_ram.psw[9] = '1';
	flash_user_ram.psw[10] = '9';
	flash_user_ram.psw[11] = '7';
	flash_user_ram.psw[12] = '4';
	flash_user_ram.psw[13] = '*';

	flash_user_ram.ip_mcu_length = 13;

	flash_user_ram.ip_mcu[0] = '1';
	flash_user_ram.ip_mcu[1] = '9';
	flash_user_ram.ip_mcu[2] = '2';
	flash_user_ram.ip_mcu[3] = '.';
	flash_user_ram.ip_mcu[4] = '1';
	flash_user_ram.ip_mcu[5] = '6';
	flash_user_ram.ip_mcu[6] = '8';
	flash_user_ram.ip_mcu[7] = '.';
	flash_user_ram.ip_mcu[8] = '0';
	flash_user_ram.ip_mcu[9] = '.';
	flash_user_ram.ip_mcu[10] = '1';
	flash_user_ram.ip_mcu[11] = '0';
	flash_user_ram.ip_mcu[12] = '0';

	flash_user_ram.ip_pc_length = 12;

	flash_user_ram.ip_pc[0] = '1';
	flash_user_ram.ip_pc[1] = '9';
	flash_user_ram.ip_pc[2] = '2';
	flash_user_ram.ip_pc[3] = '.';
	flash_user_ram.ip_pc[4] = '1';
	flash_user_ram.ip_pc[5] = '6';
	flash_user_ram.ip_pc[6] = '8';
	flash_user_ram.ip_pc[7] = '.';
	flash_user_ram.ip_pc[8] = '0';
	flash_user_ram.ip_pc[9] = '.';
	flash_user_ram.ip_pc[10] = '1';
	flash_user_ram.ip_pc[11] = '7';

	flash_user_ram.port_length = 5;

	flash_user_ram.port[0] = '5';
	flash_user_ram.port[1] = '0';
	flash_user_ram.port[2] = '0';
	flash_user_ram.port[3] = '0';
	flash_user_ram.port[4] = '0';

	ticker_init_core();	// Inicia la configuracion de los tickers

	usbcdc_init();	// Inicia la configuracion del USB
	esp_init();	// Inicia la configuracion del ESP
}

uint8_t xor(uint8_t cmd, uint8_t *payload, uint8_t payload_init, uint8_t payload_length)
{
	uint8_t xor = 0x00;

	xor ^= 'U';
	xor ^= 'N';
	xor ^= 'E';
	xor ^= 'R';
	xor ^= payload_length;
	xor ^= ':';

	xor ^= cmd;

	for (uint8_t i = payload_init ; i < payload_init + payload_length ; i++)
	{
		xor ^= payload[i];
	}

	return xor;
}

HAL_StatusTypeDef save_flash_data()
{
	uint32_t memory_address = (uint32_t)(&flash_user);
	uint32_t page_error = 0;

	FLASH_EraseInitTypeDef flash_erase;
	HAL_StatusTypeDef flash_status;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);

	flash_erase.TypeErase = FLASH_TYPEERASE_PAGES;
	flash_erase.PageAddress = memory_address;
	flash_erase.NbPages = 1;

	flash_status = HAL_FLASHEx_Erase(&flash_erase, &page_error);

	if (flash_status == HAL_OK)
	{
		for (uint16_t i = 0 ; i < 512 ; i++)
		{
			flash_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, memory_address,
					((uint16_t *)(&flash_user_ram))[i]);

			if (flash_status != HAL_OK)
			{
				break;
			}

			memory_address += 2;
		}
	}

	HAL_FLASH_Lock();

	return flash_status;
}
