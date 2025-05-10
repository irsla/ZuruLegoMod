/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

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
#define SDMMC1_DETECT_Pin GPIO_PIN_0
#define SDMMC1_DETECT_GPIO_Port GPIOA
#define SDMMC1_DETECT_EXTI_IRQn EXTI0_IRQn
#define SCREEN_CS_Pin GPIO_PIN_0
#define SCREEN_CS_GPIO_Port GPIOB
#define SCREEN_DC_Pin GPIO_PIN_1
#define SCREEN_DC_GPIO_Port GPIOB
#define SCREEN_RES_Pin GPIO_PIN_2
#define SCREEN_RES_GPIO_Port GPIOB
#define OLED_EN_Pin GPIO_PIN_9
#define OLED_EN_GPIO_Port GPIOE
#define LED_PWM_Pin GPIO_PIN_8
#define LED_PWM_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
