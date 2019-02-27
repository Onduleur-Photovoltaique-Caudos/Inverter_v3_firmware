/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#define USE_SERIAL
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define Fan_Pin GPIO_PIN_0
#define Fan_GPIO_Port GPIOF
#define Disable_Pin GPIO_PIN_1
#define Disable_GPIO_Port GPIOF
#define H1_LON_Pin GPIO_PIN_0
#define H1_LON_GPIO_Port GPIOC
#define H2_HON_Pin GPIO_PIN_1
#define H2_HON_GPIO_Port GPIOC
#define M_IHFL_Pin GPIO_PIN_2
#define M_IHFL_GPIO_Port GPIOC
#define M_VIN_Pin GPIO_PIN_0
#define M_VIN_GPIO_Port GPIOA
#define M_V225_Pin GPIO_PIN_1
#define M_V225_GPIO_Port GPIOA
#define Sync_Pin GPIO_PIN_4
#define Sync_GPIO_Port GPIOA
#define Led_Pin GPIO_PIN_5
#define Led_GPIO_Port GPIOA
#define M_V175_Pin GPIO_PIN_6
#define M_V175_GPIO_Port GPIOA
#define M_IHOUT_Pin GPIO_PIN_7
#define M_IHOUT_GPIO_Port GPIOA
#define M_IH1_Pin GPIO_PIN_4
#define M_IH1_GPIO_Port GPIOC
#define M_IH2_Pin GPIO_PIN_5
#define M_IH2_GPIO_Port GPIOC
#define M_VOUT1_Pin GPIO_PIN_0
#define M_VOUT1_GPIO_Port GPIOB
#define M_VOUT2_Pin GPIO_PIN_1
#define M_VOUT2_GPIO_Port GPIOB
#define M_IIN_Pin GPIO_PIN_2
#define M_IIN_GPIO_Port GPIOB
#define C_175_Pin GPIO_PIN_12
#define C_175_GPIO_Port GPIOB
#define H1_HON_Pin GPIO_PIN_13
#define H1_HON_GPIO_Port GPIOB
#define M_I175_Pin GPIO_PIN_14
#define M_I175_GPIO_Port GPIOB
#define M_I225_Pin GPIO_PIN_15
#define M_I225_GPIO_Port GPIOB
#define C_225_Pin GPIO_PIN_8
#define C_225_GPIO_Port GPIOC
#define CZ_225_Pin GPIO_PIN_8
#define CZ_225_GPIO_Port GPIOA
#define C_HFL_Pin GPIO_PIN_9
#define C_HFL_GPIO_Port GPIOA
#define CZ_175_Pin GPIO_PIN_10
#define CZ_175_GPIO_Port GPIOA
#define C_HFH_Pin GPIO_PIN_11
#define C_HFH_GPIO_Port GPIOA
#define H2_LON_Pin GPIO_PIN_12
#define H2_LON_GPIO_Port GPIOA
#define SYNC_TIM1_ADC_Pin GPIO_PIN_15
#define SYNC_TIM1_ADC_GPIO_Port GPIOA
#define SYNC_SERIAL_Pin GPIO_PIN_10
#define SYNC_SERIAL_GPIO_Port GPIOC
#define Psense_Pin GPIO_PIN_2
#define Psense_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */
extern volatile int mutexStats;
void start_us_DWT(int slot);
unsigned int get_us_DWT(int slot);

void cpp_ErrorHandler(char *, int);

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
