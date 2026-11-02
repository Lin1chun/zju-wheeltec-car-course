/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, OLED_SCL_Pin|ranger_tri_Pin, GPIO_PIN_RESET); //设置引脚为低电平

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, IIC_SCL_Pin|IIC_SDA_Pin|OLED_DC_Pin|OLED_RES_Pin
                          |OLED_SDA_Pin, GPIO_PIN_SET);                   //设置引脚为高电平  

	//蜂鸣器输出引脚PA15设置
	GPIO_InitStruct.Pin = beep_Pin;                         //配置PA15
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;             //推挽输出：引脚既能输出高电平、也能输出低电平，驱动能力强，普通 IO 输出标准模式。 
  GPIO_InitStruct.Pull = GPIO_NOPULL;                     //不上拉、不下拉，浮空输出 
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;            //低速模式，功耗更低，适合不需要高速翻转的普通 IO 
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    
	
  /*Configure GPIO pins : OLED_SCL_Pin ranger_tri_Pin */
  GPIO_InitStruct.Pin = OLED_SCL_Pin|ranger_tri_Pin;      //同时配置PC口2个引脚OLED 时钟脚、超声波测距触发脚
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;             //推挽输出：引脚既能输出高电平、也能输出低电平，驱动能力强，普通 IO 输出标准模式。 
  GPIO_InitStruct.Pull = GPIO_NOPULL;                     //不上拉、不下拉，浮空输出 
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;            //低速模式，功耗更低，适合不需要高速翻转的普通 IO 
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);                 //把上面配置写入 GPIOC 端口寄存器，完成初始化

  /*Configure GPIO pins : IIC_SCL_Pin， IIC_SDA_Pin ，OLED_DC_Pin ，OLED_RES_Pin ，OLED_SDA_Pin */
  GPIO_InitStruct.Pin = IIC_SCL_Pin|IIC_SDA_Pin|OLED_DC_Pin|OLED_RES_Pin|OLED_SDA_Pin;  //PB14 PB15
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     //为什么不用GPIO_MODE_OUTPUT_OD？？？？
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;          //高速模式：IIC 通信需要引脚快速高低翻转，所以开高速
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MPU6050_INT_Pin */
  GPIO_InitStruct.Pin = MPU6050_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;            //IT = Interrupt 外部中断；FALLING = 下降沿触发
  GPIO_InitStruct.Pull = GPIO_PULLUP;                     //开启内部上拉电阻：空闲时引脚稳定高电平，避免浮空误触发中断 
  HAL_GPIO_Init(MPU6050_INT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 6, 0);            //抢占优先级 = 6，子优先级 = 0；数字越大优先级越低，该中断优先级属于中等偏低
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);                    //开启这条外部中断通道，否则就算引脚检测到下降沿也不会进中断函数 
                                                       //STM32 外部中断分组：引脚 5~9 共用这一条中断服务函数
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
