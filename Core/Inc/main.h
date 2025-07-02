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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define R2_Pin GPIO_PIN_13
#define R2_GPIO_Port GPIOC
#define R2_EXTI_IRQn EXTI15_10_IRQn
#define R4_Pin GPIO_PIN_14
#define R4_GPIO_Port GPIOC
#define R4_EXTI_IRQn EXTI15_10_IRQn
#define R3_Pin GPIO_PIN_15
#define R3_GPIO_Port GPIOC
#define R3_EXTI_IRQn EXTI15_10_IRQn
#define C2_Pin GPIO_PIN_0
#define C2_GPIO_Port GPIOC
#define C3_Pin GPIO_PIN_1
#define C3_GPIO_Port GPIOC
#define C1_Pin GPIO_PIN_2
#define C1_GPIO_Port GPIOC
#define C4_Pin GPIO_PIN_3
#define C4_GPIO_Port GPIOC
#define TRIGGER2_PIN_Pin GPIO_PIN_5
#define TRIGGER2_PIN_GPIO_Port GPIOA
#define ECHO2_PIN_Pin GPIO_PIN_7
#define ECHO2_PIN_GPIO_Port GPIOA
#define ECHO2_PIN_EXTI_IRQn EXTI9_5_IRQn
#define LED_ROSSO_Pin GPIO_PIN_9
#define LED_ROSSO_GPIO_Port GPIOE
#define DEBUG_Pin GPIO_PIN_10
#define DEBUG_GPIO_Port GPIOE
#define LED_VERDE_Pin GPIO_PIN_11
#define LED_VERDE_GPIO_Port GPIOE
#define SERVO_SBARRA_Pin GPIO_PIN_12
#define SERVO_SBARRA_GPIO_Port GPIOD
#define TRIGGER_PIN_Pin GPIO_PIN_8
#define TRIGGER_PIN_GPIO_Port GPIOA
#define ECHO_PIN_Pin GPIO_PIN_9
#define ECHO_PIN_GPIO_Port GPIOA
#define ECHO_PIN_EXTI_IRQn EXTI9_5_IRQn
#define R1_Pin GPIO_PIN_11
#define R1_GPIO_Port GPIOC
#define R1_EXTI_IRQn EXTI15_10_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
