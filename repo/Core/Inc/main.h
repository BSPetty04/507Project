/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define BTN1_Pin GPIO_PIN_13
#define BTN1_GPIO_Port GPIOC
#define BTN2_Pin GPIO_PIN_14
#define BTN2_GPIO_Port GPIOC
#define BTN3_Pin GPIO_PIN_15
#define BTN3_GPIO_Port GPIOC
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOH
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOH
#define ADC_24V_Pin GPIO_PIN_0
#define ADC_24V_GPIO_Port GPIOA
#define ACT_EN_Pin GPIO_PIN_1
#define ACT_EN_GPIO_Port GPIOA
#define ACT_DIR_Pin GPIO_PIN_2
#define ACT_DIR_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_4
#define TFT_CS_GPIO_Port GPIOA
#define TFT_SCK_Pin GPIO_PIN_5
#define TFT_SCK_GPIO_Port GPIOA
#define TFT_MISO_Pin GPIO_PIN_6
#define TFT_MISO_GPIO_Port GPIOA
#define TFT_MOSI_Pin GPIO_PIN_7
#define TFT_MOSI_GPIO_Port GPIOA
#define SW_NO_Pin GPIO_PIN_0
#define SW_NO_GPIO_Port GPIOB
#define SW_NO_EXTI_IRQn EXTI0_IRQn
#define ACT_SLEEP_Pin GPIO_PIN_2
#define ACT_SLEEP_GPIO_Port GPIOB
#define TFT_DC_Pin GPIO_PIN_10
#define TFT_DC_GPIO_Port GPIOB
#define ACT_FAULT_Pin GPIO_PIN_12
#define ACT_FAULT_GPIO_Port GPIOB
#define PWM_Pin GPIO_PIN_8
#define PWM_GPIO_Port GPIOA
#define DIR1_Pin GPIO_PIN_9
#define DIR1_GPIO_Port GPIOA
#define DIR2_Pin GPIO_PIN_10
#define DIR2_GPIO_Port GPIOA
#define USB_D__Pin GPIO_PIN_11
#define USB_D__GPIO_Port GPIOA
#define USB_D_A12_Pin GPIO_PIN_12
#define USB_D_A12_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWDCLK_Pin GPIO_PIN_14
#define SWDCLK_GPIO_Port GPIOA
#define TFT_RST_Pin GPIO_PIN_15
#define TFT_RST_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define ENC_A_Pin GPIO_PIN_4
#define ENC_A_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_5
#define ENC_B_GPIO_Port GPIOB
#define SHAFT_ENC_A_Pin GPIO_PIN_6
#define SHAFT_ENC_A_GPIO_Port GPIOB
#define SHAFT_ENC_B_Pin GPIO_PIN_7
#define SHAFT_ENC_B_GPIO_Port GPIOB
#define TFT_LITE_Pin GPIO_PIN_8
#define TFT_LITE_GPIO_Port GPIOB
#define SHAFT_ENC_Z_Pin GPIO_PIN_9
#define SHAFT_ENC_Z_GPIO_Port GPIOB
#define SHAFT_ENC_Z_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
