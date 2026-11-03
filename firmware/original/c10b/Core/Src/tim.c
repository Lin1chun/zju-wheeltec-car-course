/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim2;      //定义定时器句柄
TIM_HandleTypeDef htim7;

/* TIM2 init function */
void MX_TIM2_Init(void)      //TIM2初始化函数，CubeMX自动生成，配置TIM2全部参数
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */
  //主模式同步配置不采用门控/双通道输入捕获方式可以不要
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};  //定时器时钟源配置结构体，清零初始化
  //TIM_MasterConfigTypeDef sMasterConfig = {0};    //定时器主模式同步配置结构体，清零初始化
  TIM_IC_InitTypeDef sConfigIC = {0};               //输入捕获通道配置结构体，清零初始化

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;                //句柄绑定硬件外设：指定这个句柄对应芯片内部TIM2定时器  
  htim2.Init.Prescaler = 71;            //预分频PSC=71；TIM2时钟72MHz，72/(71+1)=1MHz，1计数脉冲=1微秒
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP; //计数模式：向上计数，从0累加到ARR再溢出归零
  htim2.Init.Period = 65535;            //自动重装载值ARR=0xFFFF，16位定时器最大值，单次最长计时65535us
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  //定时器分频因子，死区/采样时钟不分频，输入捕获默认DIV1
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; //自动重装载预装载关闭：写入ARR寄存器立刻生效，不用等更新事件
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)   //自动调用：`HAL_TIM_Base_MspInit(&htim2)` 开时钟、配置PA1、开启定时器中断、配置中断优先级
  {
    Error_Handler();            //初始化失败进入错误死循环
  }
	
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;  //时钟源选择内部时钟（芯片系统总线时钟，不用外部脉冲）
  //配置定时器时钟源
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
	
  // 初始化定时器输入捕获整体模块，使能IC相关寄存器  
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
	
  /*sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;    //主模式输出触发信号TRGO：计数器更新时输出复位信号
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;//关闭主从同步模式，定时器不受其他定时器触发控制
  // 写入主模式同步配置
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }*/
	
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;   //捕获极性：上升沿触发捕获
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;         // CH2直接读取PA1引脚echo信号
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;                   //输入捕获预分频：引脚信号每1个边沿触发一次捕获，不分频
  sConfigIC.ICFilter = 0; //输入滤波系数0，不做软件滤波，引脚电平直接采样
	// 把上面捕获参数配置到TIM2通道2
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}
/* TIM7 init function */
void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 71;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 9999;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

//供用户修改的弱定义函数
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA1     ------> TIM2_CH2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 4, 0);//给 TIM2 整个中断向量设置抢占优先级 4、子优先级 0；决定 TIM2 中断和其他外设中断谁先执行。
    HAL_NVIC_EnableIRQ(TIM2_IRQn);        //告诉 CPU “允许 TIM2 产生中断请求并进入中断服务函数”
			                                    //以上2句只管 CPU 能不能收到 TIM2 的中断信号，不管 TIM2 内部哪一种中断（更新 / 捕获 / 比较）
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM7)
  {
  /* USER CODE BEGIN TIM7_MspInit 0 */

  /* USER CODE END TIM7_MspInit 0 */
    /* TIM7 clock enable */
    __HAL_RCC_TIM7_CLK_ENABLE();

    /* TIM7 interrupt Init */
    HAL_NVIC_SetPriority(TIM7_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
  /* USER CODE BEGIN TIM7_MspInit 1 */

  /* USER CODE END TIM7_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /**TIM2 GPIO Configuration
    PA1     ------> TIM2_CH2
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM7)
  {
  /* USER CODE BEGIN TIM7_MspDeInit 0 */

  /* USER CODE END TIM7_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM7_CLK_DISABLE();

    /* TIM7 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM7_IRQn);
  /* USER CODE BEGIN TIM7_MspDeInit 1 */

  /* USER CODE END TIM7_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
