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
#include "usart.h"
#include "gpio.h"
#include "stdio.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


/* Function processing DMA Rx data. Counts how many capital and small letters are in sentence.
 * Result is supposed to be stored in global variable of type "letter_count_" that is defined in "main.h"
 *
 * @param1 - received sign
 */
void proccesDmaData(uint8_t sign);
void countCharacters(char *data, uint8_t len);

/* Space for your global variables. */

	// type your global variables here:
uint8_t lowercase_count;
uint8_t uppercase_count;


int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();

  /* Space for your local variables, callback registration ...*/

  	  //type your code here:
  USART2_RegisterCallback(proccesDmaData);

  while (1)
  {
	  /* Periodic transmission of information about DMA Rx buffer state.
	   * Transmission frequency - 5Hz.
	   * Message format - "Buffer capacity: %d bytes, occupied memory: %d bytes, load [in %]: %f%"
	   * Example message (what I wish to see in terminal) - Buffer capacity: 1000 bytes, occupied memory: 231 bytes, load [in %]: 23.1%
	   */

  	  	  	  //type your code here:
	  uint8_t string[150];
	  uint16_t occupied_memory = DMA_USART2_BUFFER_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_6);
	  uint8_t size = sprintf((char*)string,
			  "\r\nBuffer capacity: %d bytes, occupied memory: %d bytes, load [in %%]: %.1f\r\n",
			  DMA_USART2_BUFFER_SIZE, occupied_memory, (float)occupied_memory/DMA_USART2_BUFFER_SIZE*100);
	  USART2_PutBuffer(string, size);

	  LL_mDelay(1000);
  }
  /* USER CODE END 3 */
}


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

/*
 * Implementation of function processing data received via USART.
 */
void proccesDmaData(uint8_t sign) {
	static char buffer[36];
	static uint8_t started;
	static uint8_t pos;

	if (sign == '#') {
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

	if ((sign == '$') && (started)) {
		countCharacters(buffer, pos);
		uint8_t string[150];
		uint8_t size = sprintf((char*) string,
						"Number of lowercase letters is %d, number of uppercase letters is %d\n\r",
						lowercase_count, uppercase_count);
		USART2_PutBuffer(string, size);
		started = 0;
		pos = 0;
	}
}

void countCharacters(char *data, uint8_t len){
	lowercase_count = 0;
	uppercase_count = 0;
	for(int i = 0; i<len; i++){
		if((data[i] >= 'A') && (data[i] <= 'Z')){
			uppercase_count++;
		}
		if((data[i] >= 'a') && (data[i] <= 'z')){
			lowercase_count++;
		}
	}
}


void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT

void assert_failed(char *file, uint32_t line)
{ 

}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/