/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void proccesDmaData(uint8_t sign);
void parseCommand(char *data, uint8_t len);

/* Space for your global variables. */

	// type your global variables here:
int aktualna_intenzita = 0;
int ziadana_intenzita = 0;
int smer = 1;
int rezim = 0;
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

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* Space for your local variables, callback registration ...*/

  	  //type your code here:
  USART2_RegisterCallback(proccesDmaData);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
 	  	  //type your code here:

	  LL_mDelay(1000);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
}

/* USER CODE BEGIN 4 */
/*
 * Implementation of function processing data received via USART.
 */
void proccesDmaData(uint8_t sign) {
	static char buffer[36];
	static uint8_t started;
	static uint8_t pos;

	if (sign == '$' && started == 1) {
		started = 2;
	}
	if (sign == '$' && started == 0) {
		started = 1;
	} else if (started == 0) {
		return;
	}

	if (pos < 36) {
		buffer[pos] = sign;
		pos++;
	} else {
		started = 0;
		pos = 0;
	}

	if ((sign == '$') && (started == 2)) {
		parseCommand(buffer, pos);
		uint8_t string[150];
		if (rezim) {
			uint8_t size = sprintf((char*) string,
					"Rezim: manual, ziadana intenzita: %d\n\r",
					ziadana_intenzita);
			USART2_PutBuffer(string, size);
		} else {
			uint8_t size = sprintf((char*) string,
					"Rezim: auto, ziadana intenzita: %d\n\r",
					ziadana_intenzita);
			USART2_PutBuffer(string, size);
		}

		started = 0;
		pos = 0;
	}
}

void parseCommand(char *data, uint8_t len){
	char manual[] = "manual";
	char automatic[] = "auto";
	char PWMxx[] = "PWMxx";
	char string[len-1];
	for(int i = 1; i < len-1; i++){ // odstrihne $ znaky
		string[i-1] = data[i];
	}
	string[len-2] = 0;
	if (!strcmp(string, manual)){
		rezim = 1;
		ziadana_intenzita = aktualna_intenzita;
	}
	if (!strcmp(string, automatic)){
		rezim = 0;
	}
	if (strcmp(string, PWMxx)<3){
		int correct = 1;
		for (int i = 0; i < len - 1; i++) {
			if (i < 3 && string[i] != PWMxx[i]) { // preveri ci sa zacina na PWM
				correct = 0;
			}
			if (i == 3 && string[i] >= '0' && string[i] <= '9' && correct){ // extrahovanie cisla z konca prikazu
				ziadana_intenzita = (string[3] - 48) * 10;
				if (string[4] >= '0' && string[4] <= '9') {	// osetrenie pripadu, ked do prikazu zadame iba jednociferne cislo + ignoruje pripadne zadanie "neciselneho" znaku
					ziadana_intenzita += string[4] - 48;
				} else {
					ziadana_intenzita /= 10;
				}
			}
		}
	}
}

void setDutyCycle(uint8_t D){
	LL_TIM_OC_SetCompareCH1(TIM2, D);
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
void assert_failed(char *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
