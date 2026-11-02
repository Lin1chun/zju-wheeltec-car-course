/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
/* ==========================================================================
 *              串口（USART/UART）模块总览 —— C10B 的数据链路
 * ==========================================================================
 *  本工程使用 3 路异步串口，全部工作在 115200-8-N-1、无硬件流控、16 倍过采样。
 *  三路各司其职，互不重叠：
 *
 *   ① USART1   PA9(TX) / PA10(RX)   APB2 = 72MHz   只发
 *      调试口。printf 通过 fputc() 重定向到这里（见 main.c），
 *      接 USB 转 TTL 到电脑用串口助手看日志。不开中断、不挂 DMA。
 *
 *   ② USART3   PC10(TX) / PC11(RX)  APB1 = 36MHz   收（+理论上的发）
 *      接 K210 视觉模块，引脚为"部分重映射"。接收方向挂 DMA1_Channel3，
 *      并用 IDLE（总线空闲）中断判定一帧结束 → ReceiveToIdle_DMA。
 *      一帧定长 13 字节，协议见 WHEELTEC_APP/Inc/k210.h。
 *      本工程只用它收，不向 K210 发数据。
 *
 *   ③ UART5    PC12(TX) / PD2(RX)   APB1 = 36MHz   只发
 *      接 C50C 底盘运动控制板。在 MPU6050 的 100Hz 中断里每 5 次（20Hz）
 *      打包一次 c10b_send 帧，用阻塞式 HAL_UART_Transmit 发出去。
 *      协议见 Core/Inc/main.h 的 C10B_Sendmsg。
 *
 *  数据流：
 *
 *      K210 摄像头 ──USART3 RX(DMA+IDLE)──► STM32F103xE ──UART5 TX──► C50C 底盘
 *                                                 │
 *                     OLED 本地显示 ◄─────────────┤
 *                                                 └──USART1 TX──► PC 串口助手（调试）
 *
 * ========================================================================== */
/* USER CODE END 0 */

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;

/**
  * @brief  UART5 初始化 —— 与 C50C 底盘控制板通信（只发不收）
  * @note   115200-8-N-1，无流控。
  *         UART5 在 STM32F103 上只有 TX/RX 两根线，没有 RTS/CTS 引脚，
  *         所以即便是硬件流控也开不了，协议层靠 BCC 异或校验保证完整性。
  *         本函数只配置外设寄存器；引脚/时钟/中断在 HAL_UART_MspInit() 里配。
  * @retval None
  */
/* UART5 init function */
void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;                          /* 外设实例：UART5（挂在 APB1 总线上） */
  huart5.Init.BaudRate = 115200;                    /* 波特率 115200bps，与 C50C 侧约定一致 */
  huart5.Init.WordLength = UART_WORDLENGTH_8B;      /* 数据位 8 位 */
  huart5.Init.StopBits = UART_STOPBITS_1;           /* 停止位 1 位 */
  huart5.Init.Parity = UART_PARITY_NONE;            /* 无奇偶校验：线路误码交给协议层 BCC 兜底 */
  huart5.Init.Mode = UART_MODE_TX_RX;               /* 收发均使能（实际只用 TX，RX 留着备用） */
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;      /* 无 RTS/CTS 硬件流控（UART5 也没有这两个引脚） */
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;  /* 16 倍过采样：常规速率的标准选择；
                                                       只有 >4.5Mbps 时才需要改 8 倍以留出采样余量 */
  /* HAL_UART_Init() 内部会回调 HAL_UART_MspInit()，完成时钟/引脚/NVIC 配置，
     然后才真正写 BRR、CR1 等寄存器。返回非 HAL_OK 说明参数非法或外设未就绪，
     进 Error_Handler() 死循环（关中断），便于调试器停下来定位。 */
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}
/**
  * @brief  USART1 初始化 —— 调试串口，printf 的输出通道（只发不收）
  * @note   PA9=TX / PA10=RX，接 USB-TTL 到 PC。
  *         本路不开中断、不挂 DMA，fputc() 用阻塞轮询方式逐字节发送，
  *         所以 printf 的实时性取决于波特率：115200 下 1 字节约 87us。
  * @retval None
  */
/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;                         /* 外设实例：USART1（挂在 APB2 总线上） */
  huart1.Init.BaudRate = 115200;                    /* 调试口常用 115200，串口助手直接对上 */
  huart1.Init.WordLength = UART_WORDLENGTH_8B;      /* 数据位 8 位 */
  huart1.Init.StopBits = UART_STOPBITS_1;           /* 停止位 1 位 */
  huart1.Init.Parity = UART_PARITY_NONE;            /* 无校验 */
  huart1.Init.Mode = UART_MODE_TX_RX;               /* 声明为收发模式；实际代码只发不收，
                                                       配成 TX_RX 不影响功能，只是 RX 引脚被占用 */
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;      /* 无硬件流控 */
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;  /* 16 倍过采样 */
  if (HAL_UART_Init(&huart1) != HAL_OK)  /* 失败一般是引脚冲突或 APB2 时钟没开 */
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/**
  * @brief  USART3 初始化 —— 接收 K210 视觉模块数据（DMA + IDLE 方式）
  * @note   PC10=TX / PC11=RX（部分重映射）。115200-8-N-1。
  *         接收链路：USART3 → DMA1_Channel3 → k210_buffer[30]
  *                   → IDLE 空闲中断 → HAL_UARTEx_RxEventCallback()
  *                   → k210_data_callback() 逐字节组帧（见 k210.c）
  *         注意：本函数只配置波特率/帧格式，DMA、GPIO、NVIC 都在 MspInit 里。
  * @retval None
  */
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;                         /* 外设实例：USART3（挂在 APB1 总线上） */
  huart3.Init.BaudRate = 115200;                    /* 与 K210 端 uart 初始化保持一致 */
  huart3.Init.WordLength = UART_WORDLENGTH_8B;      /* 数据位 8 位 */
  huart3.Init.StopBits = UART_STOPBITS_1;           /* 停止位 1 位 */
  huart3.Init.Parity = UART_PARITY_NONE;            /* 无校验（协议自带 BCC） */
  huart3.Init.Mode = UART_MODE_TX_RX;               /* 收为主，TX 暂未使用 */
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;      /* 无硬件流控 */
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;  /* 16 倍过采样 */
  if (HAL_UART_Init(&huart3) != HAL_OK)  /* 失败常见原因：AFIO 时钟没开(重映射无效)、DMA 未先初始化 */
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief  UART 底层初始化回调（MSP = MCU Support Package，芯片级支持包）
  * @note   HAL_UART_Init() 里会自动调用本函数，专门负责"外设寄存器之外"的三件事：
  *           1) 打开外设时钟（USARTx / GPIOx / DMA1 / AFIO）
  *           2) 配置 TX/RX 引脚的复用功能
  *           3) 配置 NVIC 中断优先级并使能
  *         这样分层的好处：HAL 只关心通用外设逻辑，换板子时只改 MSP 即可。
  * @param  uartHandle 由 HAL_UART_Init() 传进来的串口句柄，用 Instance 区分是哪一路
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspInit 0 */

  /* USER CODE END UART5_MspInit 0 */
    /* 打开 UART5 外设时钟（APB1 总线）。不使能时钟的话，
       后面写 UART5 的任何寄存器都不会生效，是"串口没反应"最常见的原因之一 */
    __HAL_RCC_UART5_CLK_ENABLE();

    /* UART5 的 TX 和 RX 分别落在 GPIOC 和 GPIOD 两个端口上，两个端口时钟都得开 */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**UART5 GPIO Configuration
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX
    */
    /* ---------- TX：PC12 → UART5_TX ----------
       复用推挽输出(AF_PP)：推挽结构能输出完整的高低电平，驱动能力足够；
       高速档(50MHz)是为了保证 115200 下边沿足够陡，减少波形畸变 */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* ---------- RX：PD2 → UART5_RX ----------
       STM32F1 的 HAL 惯例是把 RX 配成"普通浮空输入"，真正的采样由 USART 外设内部完成。
       注意：浮空输入在接线松动/悬空时容易收到噪声，产生乱码甚至 ORE 溢出错误。
       若现场干扰大，可改成 GPIO_MODE_IPU（内部上拉），让空闲态保持确定的高电平。 */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* 使能 UART5 中断，抢占优先级 5 / 子优先级 0。
       注意：本工程 UART5 只做阻塞发送，从未调用过接收中断 API。 */
    HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspInit 1 */

  /* USER CODE END UART5_MspInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    /* ---------- TX：PA9 → USART1_TX（复用推挽输出） ---------- */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ---------- RX：PA10 → USART1_RX（浮空输入，本工程实际不收） ---------- */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PC10     ------> USART3_TX
    PC11     ------> USART3_RX
    */
    /* ---------- TX：PC10 → USART3_TX ---------- */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* ---------- RX：PC11 → USART3_RX ----------
       这里是本工程最关键的一根接收线：K210 的 TX 接过来。
       浮空输入 + 杜邦线较长时容易出现误码，建议硬件上串 22~100Ω 电阻并缩短走线。 */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* ---------- 引脚重映射 ----------
       STM32F103 的 USART3 默认引脚是 PB10/PB11，而本板走的是 PC10/PC11，
       属于"部分重映射"（PARTIAL），必须调用这句切换 AFIO 内部的复用开关。
       AFIO 时钟已在 HAL_MspInit() 里统一打开（__HAL_RCC_AFIO_CLK_ENABLE），
       这里不用重复开。
       注：标准写法建议把这句放在 GPIO_Init 之前，本工程放在之后功能上等价
       （重映射只是切内部通路），但按顺序写可读性更好。 */
    __HAL_AFIO_REMAP_USART3_PARTIAL();

    /* ===================== USART3 接收 DMA 配置 =====================
       为什么必须是 DMA1_Channel3？
       STM32F1 的 DMA 请求映射是硬连线的，查参考手册的 DMA1 请求映射表：
           Channel2 → USART3_TX
           Channel3 → USART3_RX   ← 本工程用的就是这个
       通道号写错的话，DMA 永远等不到请求，串口接收会一直超时。 */
    hdma_usart3_rx.Instance = DMA1_Channel3;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;            /* 传输方向：外设 → 内存（接收） */
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;                /* 外设地址不自增：源永远是 USART3->DR 同一个地址 */
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;                    /* 内存地址自增：逐字节填进 k210_buffer[] 的连续位置 */
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;   /* 外设侧数据宽度：8bit（串口一次就是一个字节） */
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;      /* 内存侧数据宽度：8bit */
    hdma_usart3_rx.Init.Mode = DMA_NORMAL;                           /* 单次(NORMAL)：传满长度就停，需在回调里重新启动。
                                                                        若改成 CIRCULAR 循环模式，缓冲区会被从头覆盖，
                                                                        必须配合半传输/双缓冲使用，否则正在解析的数据会被冲掉 */
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;                 /* 通道仲裁优先级：多个通道抢总线时用，与 NVIC 优先级是两回事 */
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
    {
      Error_Handler();
    }

    /* 把 DMA 句柄挂到 UART 句柄上（写 hdmarx 指针）。
       之后 HAL_UART_Transmit_DMA / ReceiveToIdle_DMA 才能顺着 huart->hdmarx 找到通道 */
    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart3_rx);

    /* USART3 中断：优先级 5，主要用途是捕获 IDLE（总线空闲）标志，
       用来判断"这一帧数据发完了"，从而触发 HAL_UARTEx_RxEventCallback()。 */
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

/**
  * @brief  UART 底层反初始化回调
  * @note   HAL_UART_DeInit() 时调用，把 MspInit 里申请的资源全部归还：
  *         关时钟、把引脚恢复成默认状态、关 NVIC、反初始化 DMA。
  *         本工程全程不调用 DeInit，这段代码实际不会执行，保留是 CubeMX 的模板。
  * @param  uartHandle 要反初始化的串口句柄
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspDeInit 0 */

  /* USER CODE END UART5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART5_CLK_DISABLE();

    /**UART5 GPIO Configuration
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    /* UART5 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspDeInit 1 */

  /* USER CODE END UART5_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PC10     ------> USART3_TX
    PC11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
