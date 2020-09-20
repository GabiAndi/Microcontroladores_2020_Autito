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
#include "system.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
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
  system_init();	// Inicia la configuración del sistema

  ticker_new(led_blink, LED_FAIL, TICKER_LOW_PRIORITY);	// Ticker para el led de estado

  HAL_UART_Receive_IT(&huart3, (uint8_t *)(&byte_receibe_usart), 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // Configuracion inicial del modulo WiFi por defecto
  //new_ticker_ms(esp_init, 2000, LOW_PRIORITY);

  while (1)
  {
	  ticker_execute_pending();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // Para el USB
	  usbcdc_read_pending();
	  usbcdc_write_pending();

	  esp_read_pending();
	  esp_write_pending();
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
/*void esp_init(void)
{
	switch (wifi.status)
	{
		case WIFI_STATUS_BUSY:


			break;

		case WIFI_STATUS_NO_INIT:
			write_buffer(&write_buffer_UDP, (uint8_t *)("AT"), 2);
			write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

			wifi.status = WIFI_STATUS_BUSY;

			break;

		case WIFI_STATUS_INIT:
			write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CWMODE_CUR=1"), 15);
			write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

			wifi.status = WIFI_STATUS_BUSY;

			break;

		case WIFI_STATUS_STATION:
			write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CWJAP_CUR=\"Gabi-RED\",\"GabiAndi26040102.\""), 43);
			write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

			wifi.status = WIFI_STATUS_BUSY;

			break;

		case WIFI_STATUS_CONNECTED:
			wifi.status = WIFI_STATUS_BUSY;

			break;

		case WIFI_STATUS_GOT_IP:
			write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CIPSTA_CUR=\"10.0.0.10\""), 25);
			write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

			wifi.status = WIFI_STATUS_BUSY;

			break;

		case WIFI_STATUS_SET_IP:
			write_buffer(&write_buffer_UDP, (uint8_t *)("AT+CIPSTATUS"), 12);
			write_buffer(&write_buffer_UDP, (uint8_t *)("\r\n"), 2);

			wifi.status = WIFI_STATUS_BUSY;

			break;

		case WIFI_STATUS_READY:
			change_ticker_ms(led_blink, LED_OK);

			delete_ticker(esp_init);

			break;
	}
}*/

void usbcdc_write_pending(void)
{
	if (usbcdc_buffer_write.read_index != usbcdc_buffer_write.write_index)
	{
		if (CDC_Transmit_FS((uint8_t *)(&usbcdc_buffer_write.data[usbcdc_buffer_write.read_index]), 1) == USBD_OK)
		{
			usbcdc_buffer_write.read_index++;
		}
	}
}

void esp_write_pending(void)
{
	if (esp_buffer_write.read_index != esp_buffer_write.write_index)
	{
		if (HAL_UART_Transmit_IT(&huart3, (uint8_t *)(&esp_buffer_write.data[esp_buffer_write.read_index]), 1) == HAL_OK)
		{
			esp_buffer_write.read_index++;
		}
	}
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
	esp_write_buffer_read((uint8_t *)(&byte_receibe_usart), 1);

	if (debug == DEBUG_ON)
	{
		usbcdc_write_buffer_write((uint8_t *)(&byte_receibe_usart), 1);
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
