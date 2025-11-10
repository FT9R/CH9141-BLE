/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BLE_RST_Pin GPIO_PIN_3
#define BLE_RST_GPIO_Port GPIOC
#define BLE_AT_Pin GPIO_PIN_2
#define BLE_AT_GPIO_Port GPIOA
#define BTN_Pin GPIO_PIN_2
#define BTN_GPIO_Port GPIOB
#define PWR_Pin GPIO_PIN_12
#define PWR_GPIO_Port GPIOB
#define LEDR_Pin GPIO_PIN_8
#define LEDR_GPIO_Port GPIOA
#define LEDG_Pin GPIO_PIN_9
#define LEDG_GPIO_Port GPIOA
#define LEDB_Pin GPIO_PIN_10
#define LEDB_GPIO_Port GPIOA
#define VIBRO_Pin GPIO_PIN_12
#define VIBRO_GPIO_Port GPIOA
#define BLE_RLD_Pin GPIO_PIN_8
#define BLE_RLD_GPIO_Port GPIOB
#define BLE_SLEEP_Pin GPIO_PIN_9
#define BLE_SLEEP_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define LEDR_ON  HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_RESET)
#define LEDG_ON  HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin, GPIO_PIN_RESET)
#define LEDB_ON  HAL_GPIO_WritePin(LEDB_GPIO_Port, LEDB_Pin, GPIO_PIN_RESET)
#define LEDR_OFF HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET)
#define LEDG_OFF HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin, GPIO_PIN_SET)
#define LEDB_OFF HAL_GPIO_WritePin(LEDB_GPIO_Port, LEDB_Pin, GPIO_PIN_SET)

#define PWR_ON  HAL_GPIO_WritePin(PWR_GPIO_Port, PWR_Pin, GPIO_PIN_RESET)
#define PWR_OFF HAL_GPIO_WritePin(PWR_GPIO_Port, PWR_Pin, GPIO_PIN_SET)

#define BTN_CHECK HAL_GPIO_ReadPin(BTN_GPIO_Port, BTN_Pin)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
