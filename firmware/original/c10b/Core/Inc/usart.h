/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* --------------------------------------------------------------------------
 * 三路串口的全局句柄（实际定义在 usart.c）
 *    huart1 : USART1  调试口，printf 输出，115200-8-N-1，只发
 *    huart3 : USART3  接 K210 视觉模块，DMA+IDLE 接收，115200-8-N-1
 *    huart5 : UART5   接 C50C 底盘控制板，仅发送，115200-8-N-1
 * 其它 .c 文件只要包含本头文件，就能直接对这些句柄收发数据。
 * -------------------------------------------------------------------------- */
extern UART_HandleTypeDef huart5;   /* UART5：C50C 通信口 */
extern UART_HandleTypeDef huart1;   /* USART1：调试口 */
extern UART_HandleTypeDef huart3;   /* USART3：K210 视觉数据口 */

/* USER CODE BEGIN Private defines */
/* 三路串口的公共参数（如需统一修改，建议在此定义宏，再由 usart.c 引用）：
 *   波特率 : 115200
 *   帧格式 : 8 数据位 / 1 停止位 / 无校验（8-N-1）
 *   流控   : 无
 *   过采样 : 16 倍
 * 注意 USART1 挂 APB2(72MHz)，USART3/UART5 挂 APB1(36MHz)，
 * 同一个波特率由 HAL 按各自总线时钟算分频，不要手动套用同一个 BRR 值。
 */
/* USER CODE END Private defines */

/**
  * @brief  UART5 初始化（C50C 底盘通信，只发）
  * @brief  USART1 初始化（调试口，printf 重定向）
  * @brief  USART3 初始化（K210 视觉模块，DMA+IDLE 接收）
  * @note   三个初始化函数都由 CubeMX 生成，必须在 MX_DMA_Init() 之后调用
  *         （USART3 的 MspInit 里要用到 DMA 时钟与 NVIC 配置）。
  * @retval None
  */
void MX_UART5_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

