/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Fan_GPIO_Port, Fan_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Disable_GPIO_Port, Disable_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Sync_Pin|Led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SYNC_SERIAL_GPIO_Port, SYNC_SERIAL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PFPin PFPin */
  GPIO_InitStruct.Pin = Fan_Pin|Disable_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin */
  GPIO_InitStruct.Pin = Sync_Pin|Led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = SYNC_SERIAL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SYNC_SERIAL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = Psense_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(Psense_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

void doPsenseOn()
{
	HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_SET); 
}
void doPsenseOff()
{
	HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_RESET); 
}
void doensePulse()
{
	HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(Psense_GPIO_Port, Psense_Pin, GPIO_PIN_RESET); 
}
void doPsenseToggle()
{
	HAL_GPIO_TogglePin(Psense_GPIO_Port, Psense_Pin); 
}
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
	HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_SET); 
}
void doLedOff()
{
	HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_RESET); 
}
void doLedPulse()
{
	HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_RESET); 
}
void doLedToggle()
{
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

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
