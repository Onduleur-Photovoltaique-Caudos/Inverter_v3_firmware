/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Disable_GPIO_Port, Disable_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Sync_GPIO_Port, Sync_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SYNC_SERIAL_GPIO_Port, SYNC_SERIAL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = Disable_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(Disable_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = Sync_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(Sync_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = SYNC_SERIAL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SYNC_SERIAL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = Psense_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Psense_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_TSC_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(EXTI2_TSC_IRQn);

}

/* USER CODE BEGIN 2 */

void doInitPsenseAsInterrupt()
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = Psense_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(Psense_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI2_TSC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_TSC_IRQn);
	}

	void doPsenseOutput()
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	GPIO_InitStruct.Pin = Psense_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(Psense_GPIO_Port, &GPIO_InitStruct);
}
	void doEnableSwitches()
{
	HAL_GPIO_WritePin(Disable_GPIO_Port, Disable_Pin, GPIO_PIN_RESET);  // enable everything
}

void doDisableSwitches()
{
	HAL_GPIO_WritePin(Disable_GPIO_Port, Disable_Pin, GPIO_PIN_SET);   // enable everything
}

static bool bLedIsGpio;
void setLedPinGpio()
{  /*Configure GPIO pin : Led */
	if (!bLedIsGpio){
		GPIO_InitTypeDef GPIO_InitStruct = { 0 };

		GPIO_InitStruct.Pin = Led_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(Led_GPIO_Port, &GPIO_InitStruct); 
		bLedIsGpio = true;
	}
}

void doPsenseOn()
{
	HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_SET); 
}
void doPsenseOff()
{
	HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_RESET); 
}
void doPsensePulse()
{
	HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_RESET); 
}
void doPsenseToggle()
{
	HAL_GPIO_TogglePin(Psense_GPIO_Port, Psense_Pin); 
}

//
// Sync pin: do not use: PA4 not connected
//
void doSyncOn()
{
	HAL_GPIO_WritePin(Sync_GPIO_Port, Sync_Pin, GPIO_PIN_SET); 
}
void doSyncOff()
{
	HAL_GPIO_WritePin(Sync_GPIO_Port, Sync_Pin, GPIO_PIN_RESET); 
}
void doSyncPulse()
{
	HAL_GPIO_WritePin(Sync_GPIO_Port, Sync_Pin, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(Sync_GPIO_Port, Sync_Pin, GPIO_PIN_RESET); 
}
void doSyncToggle()
{
	HAL_GPIO_TogglePin(Sync_GPIO_Port, Sync_Pin); 
}
void doLedOn()
{
	setLedPinGpio();
	HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_SET); 
}
void doLedOff()
{
	setLedPinGpio();
	HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_RESET); 
}
void doLedPulse()
{
	setLedPinGpio();
	HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_RESET); 
}
void doLedToggle()
{
	setLedPinGpio();
	HAL_GPIO_TogglePin(Led_GPIO_Port, Led_Pin); 
}
void doSyncSerialOn()
{
	HAL_GPIO_WritePin(SYNC_SERIAL_GPIO_Port, SYNC_SERIAL_Pin,GPIO_PIN_SET); 
}
void doSyncSerialOff()
{
	HAL_GPIO_WritePin(SYNC_SERIAL_GPIO_Port, SYNC_SERIAL_Pin,GPIO_PIN_RESET); 
}
void doSyncSerialPulse()
{
	HAL_GPIO_WritePin(SYNC_SERIAL_GPIO_Port, SYNC_SERIAL_Pin, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(SYNC_SERIAL_GPIO_Port, SYNC_SERIAL_Pin, GPIO_PIN_RESET); 
}
void doSyncSerialToggle()
{
	HAL_GPIO_TogglePin(SYNC_SERIAL_GPIO_Port, SYNC_SERIAL_Pin); 
}
/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
