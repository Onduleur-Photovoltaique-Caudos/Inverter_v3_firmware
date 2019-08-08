/**
  ******************************************************************************
  * File Name          : HRTIM.h
  * Description        : This file provides code for the configuration
  *                      of the HRTIM instances.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __hrtim_H
#define __hrtim_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern HRTIM_HandleTypeDef hhrtim1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_HRTIM1_Init(void);
                        
void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef *hhrtim);
                    
/* USER CODE BEGIN Prototypes */
void setCompareMaster1(int newValue);
void setCompareA1(int newValue);
void setCompareA2(int newValue);
void setCompareA3(int newValue);
void setCompareB1(int newValue);
void setCompareB2(int newValue);
void setCompareB3(int newValue);
void setOutputA1(int newValue);
void setOutputA2(int newValue); 
void setOutputB1(int newValue);
void setOutputB2(int newValue);
void doUpdateMAB(void);
void doUpdateMaster(void);
void doUpdateTimA(void);
void doUpdateTimB(void);
void doOutputSetSourceA1None(void);
void doOutputSetSourceA1MasterPer(void);
void doOutputSetSourceB1None(void);
void doOutputSetSourceB1MasterCMP1(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ hrtim_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
