/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "system.h"
#include "ticker.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
// Buffers de transmisión y recepción
extern read_ring_buffer_t read_buffer_USB;
extern read_ring_buffer_t read_buffer_UDP;
extern write_ring_buffer_t write_buffer_USB;
extern write_ring_buffer_t write_buffer_UDP;

// Manejo de la ESP8266
extern wifi_manager_t wifi;

char *ip_mcu = "192.168.0.100";
char *ip_pc = "192.168.0.17";

// Byte temporal de recepción de datos
volatile uint8_t byte_receibe_usart;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void write_buffer(write_ring_buffer_t *buffer, uint8_t *data, uint8_t length);

void write_cmd_USB(uint8_t cmd, uint8_t *payload, uint8_t length);
void write_cmd_UDP(uint8_t cmd, uint8_t *payload, uint8_t length);
void write_cmd_AT(uint8_t *payload, uint8_t length);

void read_data_USB(void);
void read_data_UDP(void);

void write_data_USB(void);
void write_data_UDP(void);

uint8_t xor(uint8_t cmd, uint8_t *payload, uint8_t payload_init, uint8_t payload_length);

uint8_t command_compare(char *str1, uint8_t init, uint8_t length, char *str2);

void timeout_USB(void);
void timeout_UDP(void);

void led_blink(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  init_ticker_core();

  new_ticker_ms(led_blink, 1000, LOW_PRIORITY);

  HAL_UART_Receive_IT(&huart3, (uint8_t *)(&byte_receibe_usart), 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  read_buffer_USB.read_index = 0;
  read_buffer_USB.write_index = 0;
  read_buffer_USB.read_state = 0;

  read_buffer_UDP.read_index = 0;
  read_buffer_UDP.write_index = 0;
  read_buffer_UDP.read_state = 0;

  write_buffer_USB.read_index = 0;
  write_buffer_USB.write_index = 0;

  write_buffer_UDP.read_index = 0;
  write_buffer_UDP.write_index = 0;

  wifi.status = WIFI_STATUS_NO_INIT;
  wifi.ip_mcu = ip_mcu;
  wifi.ip_pc = ip_pc;
  wifi.debug = WIFI_DEBUG_OFF;

  HAL_Delay(5000);
  write_cmd_AT((uint8_t *)("AT"), 2);
  write_cmd_AT((uint8_t *)("AT+CIPSTATUS"), 12);

  while (1)
  {
	  execute_ticker_pending();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  read_data_USB();
	  read_data_UDP();

	  write_data_USB();
	  write_data_UDP();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void write_buffer(write_ring_buffer_t *buffer, uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0 ; i < length ; i++)
	{
		buffer->data[buffer->write_index] = data[i];
		buffer->write_index++;
	}
}

void write_cmd_USB(uint8_t cmd, uint8_t *payload, uint8_t length)
{
	// Cabecera UNER
	write_buffer(&write_buffer_USB, (uint8_t *)("UNER"), 4);
	write_buffer(&write_buffer_USB, &length, 1);
	write_buffer(&write_buffer_USB, (uint8_t *)(":"), 1);
	write_buffer(&write_buffer_USB, &cmd, 1);
	write_buffer(&write_buffer_USB, payload, length);

	uint8_t checksum = xor(cmd, payload, 0, length);

	write_buffer(&write_buffer_USB, &checksum, 1);
}

void write_cmd_UDP(uint8_t cmd, uint8_t *payload, uint8_t length)
{
	// Cabecera AT
	write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CIPSEND="), 11);
	//write_buffer(&write_buffer_UDP, 2, 1);
	write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

	// Cabecera UNER
	write_buffer(&write_buffer_UDP, (uint8_t *)("UNER"), 4);
	write_buffer(&write_buffer_UDP, &length, 1);
	write_buffer(&write_buffer_UDP, (uint8_t *)(":"), 1);
	write_buffer(&write_buffer_UDP, &cmd, 1);
	write_buffer(&write_buffer_UDP, payload, length);

	uint8_t checksum = xor(cmd, payload, 0, length);

	write_buffer(&write_buffer_UDP, &checksum, 1);
}

void write_cmd_AT(uint8_t *payload, uint8_t length)
{
	write_buffer(&write_buffer_UDP, payload, length);
	write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);
}

void read_data_USB(void)
{
	if (read_buffer_USB.read_index != read_buffer_USB.write_index)
	{
		switch (read_buffer_USB.read_state)
		{
			// Inicio de la cabecera
			case 0:
				if (read_buffer_USB.data[read_buffer_USB.read_index] == 'U')
				{
					read_buffer_USB.read_state = 1;

					new_ticker_ms(timeout_USB, 200, LOW_PRIORITY);
				}

				break;

			case 1:
				if (read_buffer_USB.data[read_buffer_USB.read_index] == 'N')
				{
					read_buffer_USB.read_state = 2;
				}

				else
				{
					read_buffer_USB.read_state = 0;
				}

				break;

			case 2:
				if (read_buffer_USB.data[read_buffer_USB.read_index] == 'E')
				{
					read_buffer_USB.read_state = 3;
				}

				else
				{
					read_buffer_USB.read_state = 0;
				}

				break;

			case 3:
				if (read_buffer_USB.data[read_buffer_USB.read_index] == 'R')
				{
					read_buffer_USB.read_state = 4;
				}

				else
				{
					read_buffer_USB.read_state = 0;
				}

				break;

			case 4:
				read_buffer_USB.payload_length = read_buffer_USB.data[read_buffer_USB.read_index];

				read_buffer_USB.read_state = 5;

				break;

			case 5:
				if (read_buffer_USB.data[read_buffer_USB.read_index] == ':')
				{
					read_buffer_USB.read_state = 6;
				}

				else
				{
					read_buffer_USB.read_state = 0;
				}

				break;

			// Inicio de la parte de comando y control
			case 6:
				read_buffer_USB.payload_init = read_buffer_USB.read_index + 1;

				read_buffer_USB.read_state = 7;

				break;

			case 7:
				// Si se terminaron de recibir todos los datos
				if (read_buffer_USB.read_index == (read_buffer_USB.payload_init + read_buffer_USB.payload_length))
				{
					// Se comprueba la integridad de datos
					if (xor(read_buffer_USB.data[read_buffer_USB.payload_init - 1], (uint8_t *)(read_buffer_USB.data),
							read_buffer_USB.payload_init, read_buffer_USB.payload_length)
							== read_buffer_USB.data[read_buffer_USB.read_index])
					{
						// Analisis del comando recibido
						switch (read_buffer_USB.data[read_buffer_USB.payload_init - 1])
						{
							case 0xF0:  // ALIVE
								write_cmd_USB(0xF0, NULL, 0x00);

								break;

							case 0xF1:	// Modo debug
								if (read_buffer_USB.data[read_buffer_USB.payload_init] == 0xFF)
								{
									wifi.debug = WIFI_DEBUG_ON;

									uint8_t request = 0x00;

									write_cmd_USB(0xF1, &request, 0x01);
								}

								else if (read_buffer_USB.data[read_buffer_USB.payload_init] == 0x00)
								{
									wifi.debug = WIFI_DEBUG_OFF;

									uint8_t request = 0x00;

									write_cmd_USB(0xF1, &request, 0x01);
								}

								else
								{
									uint8_t request = 0xFF;

									write_cmd_USB(0xF1, &request, 0x01);
								}

								break;

							case 0xF2:	// Envio de comando AT
								for (uint8_t i = 0 ; i < read_buffer_USB.payload_length ; i++)
								{
									write_buffer_UDP.data[write_buffer_UDP.write_index] =
											read_buffer_USB.data[read_buffer_USB.payload_init + i];

									write_buffer_UDP.write_index++;
								}

								break;

							default:	// Comando no valido
								write_cmd_USB(0xFF, (uint8_t *)(&read_buffer_USB.data[read_buffer_USB.payload_init - 1]), 0x01);

								break;
						}
					}

					// Corrupcion de datos al recibir
					else
					{

					}

					// Detengo el timeout
					delete_ticker(timeout_USB);
					read_buffer_USB.read_state = 0;
				}

				break;
		}

		read_buffer_USB.read_index++;
	}
}

void read_data_UDP(void)
{
	if (read_buffer_UDP.read_index != read_buffer_UDP.write_index)
	{
		switch (read_buffer_UDP.read_state)
		{
			case 0:
				if (read_buffer_UDP.data[read_buffer_UDP.read_index] != 0x0D &&
						read_buffer_UDP.data[read_buffer_UDP.read_index] != 0x0A)
				{
					read_buffer_UDP.payload_init = read_buffer_UDP.read_index;

					read_buffer_UDP.read_state = 1;

					new_ticker_ms(timeout_UDP, 100, LOW_PRIORITY);
				}

				break;

			case 1:
				if (read_buffer_UDP.data[read_buffer_UDP.read_index] == 0x0D)
				{
					read_buffer_UDP.payload_length = read_buffer_UDP.read_index;

					read_buffer_UDP.read_state = 2;
				}

				break;

			case 2:
				if (command_compare((char *)(read_buffer_UDP.data), read_buffer_UDP.payload_init,
						read_buffer_UDP.payload_length, (char *)("AT")))
				{
					wifi.command = WIFI_COMMAND_AT;
				}

				else if (command_compare((char *)(read_buffer_UDP.data), read_buffer_UDP.payload_init,
						read_buffer_UDP.payload_length, (char *)("AT+CIPSTATUS")))
				{
					wifi.command = WIFI_COMMAND_CIPSTATUS;
				}

				else
				{
					wifi.command = WIFI_COMMAND_NO_FOUND;
				}

				read_buffer_UDP.read_state = 3;

				break;

			case 3:
				if (read_buffer_UDP.data[read_buffer_UDP.read_index] != 0x0D &&
						read_buffer_UDP.data[read_buffer_UDP.read_index] != 0x0A)
				{
					read_buffer_UDP.payload_init = read_buffer_UDP.read_index;

					read_buffer_UDP.read_state = 4;
				}

				break;

			case 4:
				if (read_buffer_UDP.data[read_buffer_UDP.read_index] == 0x0D)
				{
					read_buffer_UDP.payload_length = read_buffer_UDP.read_index;

					read_buffer_UDP.read_state = 5;
				}

				break;

			case 5:
				switch (wifi.command)
				{
					case WIFI_COMMAND_AT:
						if (command_compare((char *)(read_buffer_UDP.data), read_buffer_UDP.payload_init,
								read_buffer_UDP.payload_length, (char *)("OK")))
						{
							wifi.status = WIFI_STATUS_INIT;

							change_ticker_ms(led_blink, 500);
						}

						else if (command_compare((char *)(read_buffer_UDP.data), read_buffer_UDP.payload_init,
								read_buffer_UDP.payload_length, (char *)("ERROR")))
						{
							wifi.status = WIFI_STATUS_NO_INIT;
						}

						break;

					case WIFI_COMMAND_CIPSTATUS:
						if (command_compare((char *)(read_buffer_UDP.data), read_buffer_UDP.payload_init,
								read_buffer_UDP.payload_length, (char *)("STATUS:2")))
						{
							wifi.status = WIFI_STATUS_CONNECT;

							change_ticker_ms(led_blink, 250);
						}

						else if (command_compare((char *)(read_buffer_UDP.data), read_buffer_UDP.payload_init,
								read_buffer_UDP.payload_length, (char *)("STATUS:3")))
						{
							wifi.status = WIFI_STATUS_UDP_INIT;
						}

						else
						{
							wifi.status = WIFI_STATUS_DISCONNECT;
						}

						break;
				}

				read_buffer_UDP.read_state = 0;
				delete_ticker(timeout_UDP);

				break;
		}

		read_buffer_UDP.read_index++;
	}
}

void write_data_USB(void)
{
	if (write_buffer_USB.read_index != write_buffer_USB.write_index)
	{
		if (CDC_Transmit_FS((uint8_t *)(&write_buffer_USB.data[write_buffer_USB.read_index]), 1) == USBD_OK)
		{
			write_buffer_USB.read_index++;
		}
	}
}

void write_data_UDP(void)
{
	if (write_buffer_UDP.read_index != write_buffer_UDP.write_index)
	{
		if (HAL_UART_Transmit_IT(&huart3, (uint8_t *)(&write_buffer_UDP.data[write_buffer_UDP.read_index]), 1) == HAL_OK)
		{
			write_buffer_UDP.read_index++;
		}
	}
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

uint8_t command_compare(char *str1, uint8_t init, uint8_t length, char *str2)
{
	uint8_t i = init;

	while (i != length)
	{
		if (str1[i] != str2[i - init])
		{
			return 0;
		}

		i++;
	}

	return 1;
}

void timeout_USB(void)
{
	delete_ticker(timeout_USB);

	read_buffer_USB.read_state = 0;
}

void timeout_UDP(void)
{
	delete_ticker(timeout_UDP);

	read_buffer_UDP.read_state = 0;
}

void led_blink(void)
{
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	read_buffer_UDP.data[read_buffer_UDP.write_index] = byte_receibe_usart;
	read_buffer_UDP.write_index++;

	if (wifi.debug == WIFI_DEBUG_ON)
	{
		write_buffer_USB.data[write_buffer_USB.write_index] = byte_receibe_usart;
		write_buffer_USB.write_index++;
	}

	HAL_UART_Receive_IT(&huart3, (uint8_t *)(&byte_receibe_usart), 1);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
