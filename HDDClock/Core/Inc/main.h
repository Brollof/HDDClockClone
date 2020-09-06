/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

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
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define H10_Pin GPIO_PIN_0
#define H10_GPIO_Port GPIOC
#define H1_Pin GPIO_PIN_1
#define H1_GPIO_Port GPIOC
#define HH_Pin GPIO_PIN_2
#define HH_GPIO_Port GPIOC
#define M10_Pin GPIO_PIN_3
#define M10_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_5
#define LED1_GPIO_Port GPIOA
#define M1_Pin GPIO_PIN_4
#define M1_GPIO_Port GPIOC
#define MM_Pin GPIO_PIN_5
#define MM_GPIO_Port GPIOC
#define S10_Pin GPIO_PIN_6
#define S10_GPIO_Port GPIOC
#define S1_Pin GPIO_PIN_7
#define S1_GPIO_Port GPIOC
#define COLON_Pin GPIO_PIN_9
#define COLON_GPIO_Port GPIOC
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
