#include "system.h"

// Variables
// Tickers
ticker_t ticker_system_led_status;

// Datos guardados
__attribute__ ((__section__(".user_data_flash"))) flash_data_t flash_user;	// Datos en flash
flash_data_t flash_user_ram;	// Datos en ram

extern esp_manager_t esp_manager;

// Flag de depuracion via USB
uint8_t esp_to_usb_debug;

// Variable de conversion de datos
byte_translate_u byte_translate;

void system_init(void)
{
	HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_SET);	// Inicio la ESP

	HAL_Delay(1000);	// Espero 1 segundo para eliminar los datos basura

	// Compruebo la integridad de los datos
	if (flash_user.checksum == check_flash_data_integrity(&flash_user))
	{
		memcpy(&flash_user_ram, &flash_user, sizeof(flash_data_t));	// Cargo los datos almacenados en la flash
	}

	else
	{
		memset(&flash_user_ram, 0, sizeof(flash_data_t));	// Cargo los datos con valor 0
	}

	// Carga manual de los datos de conexion (Pruebas)
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

	// Ticker para el led de estado
	ticker_system_led_status.ms_count = 0;
	ticker_system_led_status.ms_max = LED_FAIL;
	ticker_system_led_status.calls = 0;
	ticker_system_led_status.priority = TICKER_LOW_PRIORITY;
	ticker_system_led_status.ticker_function = system_led_status;
	ticker_system_led_status.active = TICKER_ACTIVE;

	ticker_new(&ticker_system_led_status);

	// Inicializacion de los modulos
	usbcdc_init();	// Inicia la configuracion del USB
	esp_init();	// Inicia la configuracion del ESP
	adc_init();	// Inicia la configuracion del ADC
}

void system_led_status(void)
{
	if (esp_manager.udp == ESP_UDP_INIT)
	{
		ticker_system_led_status.ms_max = LED_OK;
	}

	else
	{
		ticker_system_led_status.ms_max = LED_FAIL;
	}

	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

uint8_t check_xor(uint8_t cmd, uint8_t *payload, uint8_t payload_init, uint8_t payload_length)
{
	uint8_t val_xor = 0x00;

	val_xor ^= 'U';
	val_xor ^= 'N';
	val_xor ^= 'E';
	val_xor ^= 'R';
	val_xor ^= payload_length;
	val_xor ^= ':';

	val_xor ^= cmd;

	for (uint8_t i = payload_init ; i < (uint8_t)(payload_init + payload_length) ; i++)
	{
		val_xor ^= payload[i];
	}

	return val_xor;
}

HAL_StatusTypeDef save_flash_data(void)
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

	flash_user_ram.checksum = check_flash_data_integrity(&flash_user_ram);	// Calculo el checksum de los datos en RAM

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

uint8_t check_flash_data_integrity(flash_data_t *flash_data)
{
	uint8_t checksum = 0;

	for (uint16_t i = 0 ; i < 1023 ; i++)
	{
		checksum ^= ((uint8_t *)(flash_data))[i];
	}

	return checksum;
}
