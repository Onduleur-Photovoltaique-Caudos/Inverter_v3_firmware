/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
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
#define SYNC_SERIAL_Pin GPIO_PIN_10
#define SYNC_SERIAL_GPIO_Port GPIOC
#define Psense_Pin GPIO_PIN_2
#define Psense_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */
extern volatile int mutexStats;
void start_us_DWT(int slot);
unsigned int get_us_DWT(int slot);
void delay_us_DWT(unsigned long  uSec);


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
