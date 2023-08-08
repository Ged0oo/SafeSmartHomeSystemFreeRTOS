/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "LCD_interface.h"
#include "queue.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define IncrementButton			'A'
#define DecrementButton			'C'
#define ChoiceButton			'B'

#define NUMBERS_DATA    		0
#define BUTTONS_DATA 			1

#define STAGE_ONE				1
#define STAGE_TWO				2

#define FIRST_FRAME         	1
#define SECOND_FRAME         	2
#define THIRD_FRAME       		3
#define FOURTH_FRAME        	4
#define FIFTH_FRAME        		5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

uint8_t keypadVal = NOTPRESSED;
uint8_t tries = 3;
uint16_t adminPassword = 1111;
uint8_t systemLockFlag = 0;

uint8_t CurrentData;
uint8_t secondFrameFlag = 0;

uint8_t CurrentFrame = FIRST_FRAME;

uint8_t *FirstScreenFrames[6][2] =
{
		{"**  Mood One  **",	"*  Temprature  *"},
		{"**  Mood Two  **",	"* AirCondition *"},
		{"*  Mood Three  *",	"*  Lighetning  *"},
		{"*  Mood  Four  *",	"**  Humadity  **"},
		{"*  Mood  Five  *",	"* GasDetection *"}
};

uint8_t *ScoundScreenFrames[6][2] =
{
		{"Temperature Mode",	"Temprature => 27"},
		{"AirConditionMode",	"AirCondition=>NO"},
		{" Lightning Mode ",	"Lights are => ON"},
		{"* HumadityMood *",	"Humadity =>  83%"},
		{"**  Gas Mood  **",	"GasDetecte => NO"}
};

/* USER CODE END Variables */
/* Definitions for setFrameTask */
osThreadId_t setFrameTaskHandle;
const osThreadAttr_t setFrameTask_attributes = {
  .name = "setFrameTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for secondFrameTask */
osThreadId_t secondFrameTaskHandle;
const osThreadAttr_t secondFrameTask_attributes = {
  .name = "secondFrameTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for keypadTask */
osThreadId_t keypadTaskHandle;
const osThreadAttr_t keypadTask_attributes = {
  .name = "keypadTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for IntializationTa */
osThreadId_t IntializationTaHandle;
const osThreadAttr_t IntializationTa_attributes = {
  .name = "IntializationTa",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh1,
};
/* Definitions for FramesQueue */
osMessageQueueId_t FramesQueueHandle;
const osMessageQueueAttr_t FramesQueue_attributes = {
  .name = "FramesQueue"
};
/* Definitions for ButtonsQueue */
osMessageQueueId_t ButtonsQueueHandle;
const osMessageQueueAttr_t ButtonsQueue_attributes = {
  .name = "ButtonsQueue"
};
/* Definitions for NumbersQueue */
osMessageQueueId_t NumbersQueueHandle;
const osMessageQueueAttr_t NumbersQueue_attributes = {
  .name = "NumbersQueue"
};
/* Definitions for KeypadLcdSemaphore */
osSemaphoreId_t KeypadLcdSemaphoreHandle;
const osSemaphoreAttr_t KeypadLcdSemaphore_attributes = {
  .name = "KeypadLcdSemaphore"
};
/* Definitions for FramesSemaphore */
osSemaphoreId_t FramesSemaphoreHandle;
const osSemaphoreAttr_t FramesSemaphore_attributes = {
  .name = "FramesSemaphore"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void GetPassword();
void PutWelcommingMessegae();
void LcdSetFrame(uint8_t stage, uint8_t frame);

/* USER CODE END FunctionPrototypes */

void vSetFrameTask(void *argument);
void vSecondFrameTask(void *argument);
void vKeypadTask(void *argument);
void vInitTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{

  /* creation of IntializationTa */
  IntializationTaHandle = osThreadNew(vInitTask, NULL, &IntializationTa_attributes);

}

/* USER CODE BEGIN Header_vSetFrameTask */
/**
  * @brief  Function implementing the setFrameTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_vSetFrameTask */
void vSetFrameTask(void *argument)
{
  /* USER CODE BEGIN vSetFrameTask */
	BaseType_t RetVal = pdTRUE;
	uint8_t QueueDirection;
	/* Infinite loop */
	for(;;)
	{
		RetVal= xQueueReceive(ButtonsQueueHandle, &QueueDirection, 0);

		if(QueueDirection == IncrementButton)
		{
			if(CurrentFrame == FIFTH_FRAME)
				CurrentFrame = FIRST_FRAME;
			else CurrentFrame++;
			LcdSetFrame(STAGE_ONE, CurrentFrame);
		}

		else if(QueueDirection == DecrementButton)
		{
			if(CurrentFrame == FIRST_FRAME)
				CurrentFrame = FIFTH_FRAME;
			else CurrentFrame-- ;
			LcdSetFrame(STAGE_ONE, CurrentFrame);
		}

		else if(QueueDirection == ChoiceButton)
		{
			secondFrameFlag ^= 1;
			RetVal = xQueueSendToFront(FramesQueueHandle, (void *)&CurrentFrame, HAL_MAX_DELAY);
			osSemaphoreAcquire(FramesSemaphoreHandle, HAL_MAX_DELAY);
		}

		osSemaphoreRelease(KeypadLcdSemaphoreHandle);
	}
  /* USER CODE END vSetFrameTask */
}

/* USER CODE BEGIN Header_vSecondFrameTask */
/**
* @brief Function implementing the secondFrameTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vSecondFrameTask */
void vSecondFrameTask(void *argument)
{
  /* USER CODE BEGIN vSecondFrameTask */
	BaseType_t RetVal = pdTRUE;
	uint8_t message = 0;
	uint8_t QueueDirection;
	uint8_t choice = 0;
	/* Infinite loop */
	for(;;)
	{
		RetVal= xQueueReceive(FramesQueueHandle, &message, HAL_MAX_DELAY);

		if(pdTRUE == RetVal)
		{
			if(secondFrameFlag)
			{
				LcdSetFrame(STAGE_TWO, message);
			}
			else
			{
				LcdSetFrame(STAGE_ONE, message);
			}
		}

		osSemaphoreRelease(FramesSemaphoreHandle);
	}
  /* USER CODE END vSecondFrameTask */
}

/* USER CODE BEGIN Header_vKeypadTask */
/**
* @brief Function implementing the keypadTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vKeypadTask */
void vKeypadTask(void *argument)
{
  /* USER CODE BEGIN vKeypadTask */
	BaseType_t RetVal = pdTRUE;
	/* Infinite loop */
	for(;;)
	{
		keypadVal = NOTPRESSED;
		keypadVal = read_keypad(&keypad_1);
		if(keypadVal != NOTPRESSED)
		{
			if(keypadVal<'1' || keypadVal>'9')
			{
				CurrentData = BUTTONS_DATA;
				RetVal = xQueueSendToFront(ButtonsQueueHandle, (void *)&keypadVal, HAL_MAX_DELAY);
			}

			osSemaphoreAcquire(KeypadLcdSemaphoreHandle, HAL_MAX_DELAY);
		}

	}
  /* USER CODE END vKeypadTask */
}

/* USER CODE BEGIN Header_vInitTask */
/**
* @brief Function implementing the IntializationTa thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vInitTask */
void vInitTask(void *argument)
{
	/* USER CODE BEGIN vInitTask */
	/* Infinite loop */
	for(;;)
	{
		lcd_4bit_intialize(&lcd_1);
		keypad_initialize(&keypad_1);

		PutWelcommingMessegae();

		if(0 == systemLockFlag)
		{
			while((tries) && (0 == systemLockFlag)) GetPassword();
		}

		if((0 == tries) && (0 == systemLockFlag))
		{
			LCD_Clear(&lcd_1);
			lcd_4bit_send_string_pos(&lcd_1, 1, 1, "**   ERROR!   **");
			HAL_Delay(200);
			lcd_4bit_send_string_pos(&lcd_1, 2, 2, "System Locked");
			HAL_Delay(200);
			while(1);
		}
		else
		{
			  KeypadLcdSemaphoreHandle = osSemaphoreNew(1, 1, &KeypadLcdSemaphore_attributes);
			  FramesSemaphoreHandle = osSemaphoreNew(1, 1, &FramesSemaphore_attributes);
			  FramesQueueHandle = osMessageQueueNew (5, sizeof(uint8_t), &FramesQueue_attributes);
			  ButtonsQueueHandle = osMessageQueueNew (5, sizeof(uint8_t), &ButtonsQueue_attributes);
			  NumbersQueueHandle = osMessageQueueNew (5, sizeof(uint8_t), &NumbersQueue_attributes);
			  setFrameTaskHandle = osThreadNew(vSetFrameTask, NULL, &setFrameTask_attributes);
			  secondFrameTaskHandle = osThreadNew(vSecondFrameTask, NULL, &secondFrameTask_attributes);
			  keypadTaskHandle = osThreadNew(vKeypadTask, NULL, &keypadTask_attributes);
			  IntializationTaHandle = osThreadNew(vInitTask, NULL, &IntializationTa_attributes);
			  LcdSetFrame(STAGE_ONE, CurrentFrame);
			  vTaskSuspend(IntializationTaHandle);
		}
	}
	/* USER CODE END vInitTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


void LcdSetFrame(uint8_t stage, uint8_t frame)
{
	frame--;
	switch(stage)
	{
		case STAGE_ONE :
			lcd_4bit_send_string_pos(&lcd_1, 1, 1, FirstScreenFrames[frame][0]);
			lcd_4bit_send_string_pos(&lcd_1, 2, 1, FirstScreenFrames[frame][1]);
			break;

		case STAGE_TWO :
			lcd_4bit_send_string_pos(&lcd_1, 1, 1, ScoundScreenFrames[frame][0]);
			lcd_4bit_send_string_pos(&lcd_1, 2, 1, ScoundScreenFrames[frame][1]);
			break;
	}
}


void PutWelcommingMessegae()
{
	LCD_Clear(&lcd_1);
	lcd_4bit_send_string_pos(&lcd_1, 1, 6, "Hello");
	HAL_Delay(200);
	lcd_4bit_send_string_pos(&lcd_1, 2, 5, "Welcome");
	HAL_Delay(500);
}


void GetPassword()
{
	uint8_t passLength = 4, digit=0;
	uint16_t password=0;

	LCD_Clear(&lcd_1);
	lcd_4bit_send_string_pos(&lcd_1, 1, 2, "Admin Password");
	HAL_Delay(500);

	while(passLength)
	{
		keypadVal = NOTPRESSED;
		keypadVal = read_keypad(&keypad_1);
		if(keypadVal != NOTPRESSED)
		{
			digit = keypadVal-'0';
			password = (password*10) + digit;

			lcd_4bit_send_char_data_pos(&lcd_1, 2, (4-passLength)+7, keypadVal);
			HAL_Delay(150);
			lcd_4bit_send_char_data_pos(&lcd_1, 2, (4-passLength)+7, '*');
			HAL_Delay(150);

			passLength--;
		}
	}

	if(password == adminPassword)
	{
		systemLockFlag = 1;

		LCD_Clear(&lcd_1);
		lcd_4bit_send_string_pos(&lcd_1, 1, 6, "Hello");
		HAL_Delay(200);

		lcd_4bit_send_string_pos(&lcd_1, 2, 4, "You are in");
		HAL_Delay(500);
	}
	else
	{
		systemLockFlag = 0;
		tries--;

		LCD_Clear(&lcd_1);
		lcd_4bit_send_string_pos(&lcd_1, 1, 1, "* WrongPassword *");
		HAL_Delay(200);

		if(0 == tries) lcd_4bit_send_string_pos(&lcd_1, 2, 2, "No Tries Left");
		else
		{
			lcd_4bit_send_string_pos(&lcd_1, 2, 1, "* Tries Left ");
			LCD_WriteNumber_Position(tries, 2, 14);
			lcd_4bit_send_string_pos(&lcd_1, 2, 15, " *");
		}

		HAL_Delay(1000);
	}
}

/* Task to be created. */


/* USER CODE END Application */

