/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_lcd.h"
#include "draw.h"
#include "resources.h"
#include <stdlib.h>
#include "game1.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
  int num=0;
  extern uint8_t uptMove;
  unsigned char Mpu6050=0;
  
  
//extern uint8_t uptMove;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */




/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

	TaskHandle_t xOLEDTaskHandle;
	TaskHandle_t xLEDTaskHandle;
	TaskHandle_t xSouldTaskHandle;
	TaskHandle_t xKEYTaskHandle;

	struct printinfo
	{
		uint8_t y;
		char name[16];
	};

	struct printinfo task1={0,"taks1:"};	
	struct printinfo task2={10,"taks2:"};	
	struct printinfo task3={15,"taks3:"};	
	
	
void OLED(void *argument);
void LED(void *argument);
void Sould(void *argument);
void KEY(void *argument);
void PRINT(void *argument);
void game1_task(void *params);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
   LCD_Init();
   LCD_Clear();
   LCD_PrintString(0, 0, "Starting");
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */

	
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
//  xTaskCreate(OLED,"OLEDTask",128,NULL,osPriorityNormal,&xOLEDTaskHandle);
//  xTaskCreate(LED,"LEDTask",128,NULL,osPriorityNormal,&xLEDTaskHandle);  
  xTaskCreate(game1_task, "GameTask", 128, NULL, osPriorityNormal, NULL);
  xTaskCreate(KEY,"KEYTask",128,NULL,osPriorityNormal,&xKEYTaskHandle);  
//  xTaskCreate(Sould,"SouldTask",128,NULL,osPriorityNormal,&xSouldTaskHandle);  


//  xTaskCreate(PRINT,"PRINTTask1",128,&task1,osPriorityNormal,NULL);  
//  xTaskCreate(PRINT,"PRINTTask2",128,&task2,osPriorityNormal,NULL);  
//  xTaskCreate(PRINT,"PRINTTask3",128,&task3,osPriorityNormal,NULL);  

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */





void OLED(void *argument)
{


  for(;;)
  {
	


  }

}

unsigned  char canuse=1;

void PRINT(void *argument)
{
	struct printinfo *pInfo = argument;
	uint16_t cnt =0;
  for(;;)
  {
	  if(canuse)
	  {
		  canuse=0;

		  canuse=1;
	  }
	  
//	  vTaskDelay(1000);
	  osDelay(100);
  }

}

void KEY(void *argument)
{

  for(;;)
  {
	  if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)
	  {
		osDelay(100);
		if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9)==0)
		{
			uptMove = 1;
		}
	  }
	  if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2)==0)
	  {
		osDelay(100);
		if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2)==0)
		{			
			uptMove = 2;
		}
	  }
	  if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6)==0)
	  {
		osDelay(100);
		if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6)==0)
		{		
			if(Mpu6050)Mpu6050=0;
			else Mpu6050=1;
			
		}
	  }	  
	  
	  osDelay(20);
		
  }

}

void LED(void *argument)
{

  for(;;)
  {
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,1);
		osDelay(1000);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,0);
		osDelay(1000);	  
  }

}

void Sould(void *argument)
{

  for(;;)
  {
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,1);
		osDelay(1000);
	  	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,0);
		osDelay(1000);
  }

}


/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

