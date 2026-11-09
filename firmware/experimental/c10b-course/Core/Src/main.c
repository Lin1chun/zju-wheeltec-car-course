/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float ranger_dis=0;
float temperature=0;
uint8_t k210_buffer[30];
uint8_t k210_active_cnt = 0;
uint8_t k210_active_flag=0;

C10B_Sendmsg c10b_send = { 0 };
/* ================= servo-scanned ultrasonic ================= */
#define SCAN_SECTORS 5
#define RANGER_SCAN_TICKS 10u
#define RANGER_TRIG_TICK_1 5u
#define RANGER_TRIG_TICK_2 9u
#define RANGER_STALE_TICKS 75u
#define RANGER_STOP_MM 600u
#define RANGER_CLEAR_MM 650u
#define RANGER_NO_ECHO_MM 4000u
static const uint16_t scan_pulse_us[SCAN_SECTORS] = {1000,1250,1500,1750,2000};
static const int8_t   scan_angle_deg[SCAN_SECTORS] = {-60,-30,0,30,60};
static uint8_t  scan_sec = 2;
static uint8_t  scan_tick = 0;
static uint16_t scan_dist_mm[SCAN_SECTORS] = {0,0,0,0,0};
static uint8_t  scan_ok[SCAN_SECTORS] = {0,0,0,0,0};
static uint8_t  scan_age[SCAN_SECTORS] = {0,0,0,0,0};
static volatile uint8_t echo_sec = 2;
static volatile uint8_t ranger_cap_wait_down = 0;
static volatile uint8_t ranger_sample_pending = 0;
static uint8_t ranger_echo_timeout = 0;
static uint16_t front_mm = 0;
static uint16_t min_mm = 0;
static uint8_t  obstacle_flag = 1;

static void ranger_tri_ranging(void)
{
    ranger_cap_wait_down = 0;
    ranger_sample_pending = 1;
    ranger_echo_timeout = 0;
    TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2);
    TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_RISING);

    HAL_GPIO_WritePin(ranger_tri_GPIO_Port, ranger_tri_Pin, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(ranger_tri_GPIO_Port, ranger_tri_Pin, GPIO_PIN_SET);
    delay_us(15);
    HAL_GPIO_WritePin(ranger_tri_GPIO_Port, ranger_tri_Pin, GPIO_PIN_RESET);
}

static void scan_update_summary(void)
{
    uint8_t i;
    uint16_t fmin=0xFFFF, amin=0xFFFF;
    for(i=0;i<SCAN_SECTORS;i++)
    {
        if(!scan_ok[i]) continue;
        if(scan_dist_mm[i] && scan_dist_mm[i]<amin) amin=scan_dist_mm[i];
        if(i>=1 && i<=3 && scan_dist_mm[i] && scan_dist_mm[i]<fmin) fmin=scan_dist_mm[i];
    }
    min_mm = (amin==0xFFFF)?0:amin;
    front_mm = (fmin==0xFFFF)?0:fmin;
    if(fmin==0xFFFF) obstacle_flag=1;          /* no valid forward echo: fail safe */
    else if(fmin<RANGER_STOP_MM) obstacle_flag=1;
    else if(fmin>RANGER_CLEAR_MM) obstacle_flag=0;
}

static void scan_add_sample(uint16_t mm,uint8_t sec)
{
    if(sec>=SCAN_SECTORS || mm<20 || mm>4000) return;
    if(!scan_ok[sec] || mm < scan_dist_mm[sec] || mm >= RANGER_NO_ECHO_MM)
        scan_dist_mm[sec]=mm;
    else
        scan_dist_mm[sec]=(uint16_t)(((uint32_t)scan_dist_mm[sec]*3u+mm)/4u);
    scan_ok[sec]=1;
    scan_age[sec]=0;
    scan_update_summary();
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ==========================================================================
 *  C10B 串口数据链路说明（配合 usart.c / k210.c 一起看）
 * ==========================================================================
 *  三路串口分工：
 *      USART1 (PA9/PA10)   调试口，printf 输出，115200-8-N-1，只发
 *      USART3 (PC10/PC11)  接 K210 视觉模块，DMA1_Ch3 + IDLE 接收，115200-8-N-1
 *      UART5  (PC12/PD2)   接 C50C 底盘控制板，仅发送，115200-8-N-1
 *
 *  完整数据流：K210 视觉 → USART3(DMA+IDLE) → STM32 → UART5 → C50C 底盘
 *
 *  关键函数对照：
 *      HAL_UARTEx_RxEventCallback() : IDLE/DMA 接收回调，解析 K210 数据（文件末尾）
 *      HAL_GPIO_EXTI_Callback()     : 借 MPU6050 的 100Hz 中断做 20Hz 上报（文件中段）
 *      fputc()                      : printf 重定向到 USART1（文件中段）
 *      k210_data_callback()         : 13 字节协议组帧与 BCC 校验（WHEELTEC_APP/k210.c）
 * ========================================================================== */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init(); //flash 中断优先级 滴答时钟 JTAG口设置等

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* 外设初始化顺序有讲究：
     MX_DMA_Init() 必须排在 MX_USART3_UART_Init() 之前。
     因为 USART3 的 HAL_UART_MspInit() 里要调用 HAL_DMA_Init() 配置 DMA1_Channel3，
     并且依赖 dma.c 里已经打开的 DMA1 时钟和 NVIC 使能。
     顺序颠倒的典型症状：串口能发不能收、DMA 回调一次都不进。 */
  MX_GPIO_Init();
  MX_DMA_Init();          /* 开 DMA1 时钟 + 配置 DMA1_Channel3 中断（优先级 0，最高） */
  MX_UART5_Init();        /* UART5  -> C50C 底盘，只发 */
  MX_USART3_UART_Init();  /* USART3 -> K210 视觉，DMA + IDLE 收 */
  MX_TIM2_Init();  //分频、计数模式、开时钟、配置PA1、开中断配置中断优先级等
  MX_TIM7_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();  /* USART1 -> 调试口，printf 输出 */
  /* USER CODE BEGIN 2 */
	DWT_Init();
	OLED_Init();
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim7);          /* independent 100 Hz scan timebase */
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,scan_pulse_us[scan_sec]);
	
	//启动超声波通道
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);   //开启捕获中断
	
	//MPU6050初始化（MPU6050.C）
	MPU6050_initialize();  //检查通讯、设置量程、关闭休眠和AUX IIC 接口
	DMP_Init(); //这里包含MPU6050初始化
	
	//启动DMA接收K210数据
	/* 启动 K210 数据接收：DMA + IDLE 组合
   - ReceiveToIdle_DMA 先启动 DMA 搬运，再打开 IDLE 中断；
   - 帧与帧之间总有间隔，总线空闲满 1 个字符时间（115200 下约 87us）后
     硬件置位 IDLE 标志，HAL 据此判定"这一批数据收完了"，进而回调
     HAL_UARTEx_RxEventCallback()；
   - 缓冲区开 30 字节（一帧 13 字节）是留余量：K210 连发两帧(26B)时
     也能一次装下，回调里再把 26 个字节逐字节拆成两帧；
   - 整个程序只调用这一次，之后在回调末尾循环重启，形成"永久接收"。
   注意：缓冲区不要开得比一帧还小，否则 DMA 传输完成(TC)会覆盖未解析数据。 */
	HAL_UARTEx_ReceiveToIdle_DMA(&huart3,k210_buffer,30);
	
	//蜂鸣器测试
	HAL_GPIO_WritePin(GPIOA, beep_Pin, GPIO_PIN_RESET);   
			
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  	  /* 16x16 Chinese distance line: display 0.05 m as requested */
	  OLED_ShowCN16(0,0,cnfont16[CN16_JULI]);
	  OLED_ShowCN16(16,0,cnfont16[CN16_LI]);
	  OLED_ShowFloat16(32,0,front_mm/1000.0f,1,2);
	  OLED_ShowCN16(80,0,cnfont16[CN16_MI]);

	  /* Servo sector, forward-zone distance and obstacle decision */
	  OLED_ShowString(0,16,"S");
	  OLED_ShowShortNum(8,16,scan_angle_deg[scan_sec],3,12);
	  OLED_ShowString(36,16,"OBS");
	  OLED_ShowNumber(60,16,front_mm,4,12);
	  OLED_ShowString(96,16,obstacle_flag?"STOP":"GO  ");

	  OLED_ShowString(0,28,"P");
	  OLED_ShowFloat(10,28,Pitch,3,2);
	  OLED_ShowString(60,28,"R");
	  OLED_ShowFloat(70,28,Roll,3,2);

	  OLED_ShowString(0,40,"Y");
	  OLED_ShowFloat(10,40,Yaw,3,2);
	  OLED_ShowString(64,40,"T");
	  OLED_ShowFloat(74,40,temperature,3,2);

	  OLED_ShowString(0,52,"CX");
	  OLED_ShowNumber(20,52,k210.cx,3,12);
	  OLED_ShowString(48,52,"SZ");
	  OLED_ShowNumber(68,52,k210.size,4,12);

	  OLED_Refresh_Gram();
	  delay_ms(50);
	  {
	      static uint8_t dbg_cnt = 0;
	      if(++dbg_cnt >= 20)          /* 1 Hz debug line on USART1 (PA9/PA10) */
	      {
	          dbg_cnt = 0;
	          printf("D=%4dmm P=%6d R=%6d Y=%6d S=%3d OB=%d\r\n",
	                 front_mm,(int)(Pitch*100),(int)(Roll*100),(int)(Yaw*100),
	                 scan_angle_deg[scan_sec],obstacle_flag);
	      }
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;  //HSE：8MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;               //PLL 倍频：MUL9  SYSCLK = 8 × 9 = 72MHz
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   //AHB(HCLK)：72MHz / 1 = 72MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;    //APB1(PCLK1)：72MHz / 2 = 36MHz 
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    //APB2(PCLK2)：72MHz / 1 = 72MHz

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* ==========================================================================
 *  串口重定向：让 printf 走 USART1
 * ==========================================================================
 *  Keil MDK 里 printf 最终会逐字符调用库函数 fputc()，
 *  所以自己实现一个 fputc 把字符塞进串口，printf 就"改道"到串口了。
 *  前提：工程勾选了 Use MicroLIB（本工程 C10B.uvprojx 中 useUlib=1），
 *        否则标准库还会牵扯 semihosting，需要额外 #pragma 才能编译通过。
 *
 *  调用链： printf("x=%d",1) -> 格式化 -> 逐字符 fputc() -> USART1 发送
 * ========================================================================== */
UART_HandleTypeDef *DebugSerial = &huart1;   /* 调试输出绑定的串口，换口只改这一行 */

/**
  * @brief  printf 的底层字符输出函数（重定向到 USART1）
  * @param  ch     要发送的字符（库传入的是 int，实际只用低 8 位）
  * @param  stream 文件流指针，printf 默认 stdout；本实现不使用，
  *                但函数签名由库规定，参数必须保留
  * @retval 返回 ch 表示"该字符已被接受"，printf 才会继续输出下一个字符
  * @note   阻塞式发送，每字节约 87us@115200。
  *         若在主循环里高频 printf，会明显拖慢主循环节拍，调试完记得注释掉。
  */
int fputc(int ch, FILE* stream)
{
	uint16_t cnt = 0;

	/* HAL_UART_Transmit(句柄, 数据指针, 长度, 超时ms)
	   - (const uint8_t *)&ch : 取 ch 的地址并强制转成字节指针（HAL 需要按字节发）
	   - 1                      : 本次只发 1 个字符
	   - 100                    : 单次发送最长等 100ms，超时返回 HAL_TIMEOUT */
	while( HAL_OK != HAL_UART_Transmit(DebugSerial,(const uint8_t *)&ch,1,100) && cnt <1000);

	/* 【已知问题】上面的 cnt 本意是"最多重试 1000 次"的保险丝，
	   但循环体内没有 cnt++，条件恒为真 -> 超时保护实际失效。
	   一旦串口异常（被其它代码占用、或波特率/时钟配错导致 TXE 一直不来），
	   程序会在这里反复等 100ms 超时，表现为整机卡死在 printf 上。
	   建议改为： while( HAL_OK != HAL_UART_Transmit(...) ) { if(++cnt >= 3) break; } */
	return ch;
}

//MPU6050 external interrupt: attitude and temperature only.
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint32_t temperature_cnt = 0;
	uint8_t tempbuffer[2];

	if( GPIO_Pin == MPU6050_INT_Pin )
	{
		temperature_cnt++;
		if( temperature_cnt>=100*5 )          /* 5 s */
		{
			temperature_cnt=0;
			pIICInterface_t iic = &User_sIICDev;
			iic->read_reg(0x68<<1,0x41,tempbuffer,2,200);
			temperature = (float)((short)(tempbuffer[0]<<8|tempbuffer[1]))/340.0f+36.53f;
		}
		Read_DMP();
	}
}

/* Independent 100 Hz timebase (TIM7): servo scan + 20 Hz UART5 frame.
   It no longer depends on the MPU6050 INT, so distance/obstacle data keep
   working even if the IMU is not initialized. */
static void C10B_Tick100Hz(void)
{
	static uint8_t c50c_send_cnt = 0;
	uint8_t i;

	/* Servo sector scan: 5 sectors x 100 ms, two pings per sector. */
	for(i=0;i<SCAN_SECTORS;i++)
	{
		if(scan_age[i]<255) scan_age[i]++;
		if(scan_age[i]>RANGER_STALE_TICKS) scan_ok[i]=0;
	}
	scan_tick++;
	if(scan_tick>=RANGER_SCAN_TICKS)
	{
		scan_tick=0;
		scan_sec=(uint8_t)((scan_sec+1)%SCAN_SECTORS);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,scan_pulse_us[scan_sec]);
	}
	if(scan_tick==RANGER_TRIG_TICK_1 || scan_tick==RANGER_TRIG_TICK_2)
	{
		echo_sec=scan_sec;
		ranger_tri_ranging();
	}
	else if(ranger_sample_pending)
	{
		ranger_echo_timeout++;
		if(ranger_echo_timeout>4)
		{
			ranger_echo_timeout=0;
			ranger_sample_pending=0;
			ranger_cap_wait_down=0;
			TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2);
			TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_RISING);
			scan_add_sample(RANGER_NO_ECHO_MM,echo_sec);
		}
	}
	scan_update_summary();

	/* K210 alive timeout */
	k210_active_cnt++;
	if( k210_active_cnt>=100*1 ) k210_active_flag = 0;

	/* 20 Hz frame to C50C (UART5, 115200) */
	c50c_send_cnt++;
	if( c50c_send_cnt>=5 )
	{
		c50c_send_cnt = 0;
		c10b_send.Head1 = C10B_HEAD1;
		c10b_send.Head2 = C10B_HEAD2;
		c10b_send.k210_alive = k210_active_flag;
		c10b_send.flags = (uint8_t)((obstacle_flag?0x01:0)|(scan_ok[2]?0x02:0)|(k210_active_flag?0x04:0));
		c10b_send.k210_cx = k210.cx;
		c10b_send.k210_size = k210.size;
		c10b_send.distance = front_mm;
		c10b_send.distance_min = min_mm;
		c10b_send.temperature = (short)(temperature*100.0f);
		c10b_send.pitch = (short)(Pitch*100.0f);
		c10b_send.roll  = (short)(Roll*100.0f);
		c10b_send.yaw   = (short)(Yaw*100.0f);
		c10b_send.servo_deg = scan_angle_deg[scan_sec];
		c10b_send.scan_mask = (uint8_t)((scan_ok[0]?0x01:0)|(scan_ok[1]?0x02:0)|(scan_ok[2]?0x04:0)|(scan_ok[3]?0x08:0)|(scan_ok[4]?0x10:0));
		{
			uint8_t* sendptr = (uint8_t*)&c10b_send;
			c10b_send.BCCcheck = calculateBCC(sendptr,sizeof(c10b_send)-1);
			HAL_UART_Transmit(&huart5,sendptr,sizeof(c10b_send),10);
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM7) C10B_Tick100Hz();
}
//超声波采集辅助变量
uint32_t upvalue=0;
uint32_t downvalue=0;
//超声波采集中断，已经捕获到上升/下降沿中断
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	#define TIM_COUNT_FREQ 1000000.0f
	enum {
		WaitUp=0,
		WaitDown,
	};
	
	
	//超声波接口触发中断
	if( htim == &htim2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2 )
	{
		switch (ranger_cap_wait_down)
		{
			case WaitUp:
			{
				// __HAL_TIM_SET_COUNTER(htim, 0);  //计数器清0
        //ovf_cnt = 0;  //溢出计数清0   要启用__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
				
				//读出计数值
				upvalue = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);  //当引脚产生边沿捕获事件时，硬件自动把当前 CNT 计数器值锁存进 CCR2。
				
				//设置下降沿捕获
				TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2);  //复位ch2捕获极性，默认上升沿
				TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_FALLING);   //设置ch2捕获极性为下降沿
				
				//状态机切换
				ranger_cap_wait_down = 1;
				break;
			}
			
			case WaitDown:
			{
				//读取计数值
				downvalue = HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);
				
				//计算距离
				if( upvalue > downvalue )//溢出计算,注意没有启用计数器溢出更新中断
					ranger_dis = downvalue+0xFFFF - upvalue;  //以目前的距离最多只会溢出一次
				else
					ranger_dis = downvalue - upvalue;      //得到us
				
				// S = V*t/2
				ranger_dis = (ranger_dis/TIM_COUNT_FREQ)/2.0f * 340.0f;  //us转成s，乘以声速，得到距离是m
				{
					uint16_t mm=(uint16_t)(ranger_dis*1000.0f+0.5f);
					scan_add_sample(mm,echo_sec);
				}
				ranger_sample_pending=0;
				//设置上升沿捕获
				TIM_RESET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2);
				TIM_SET_CAPTUREPOLARITY(&htim2,TIM_CHANNEL_2,TIM_ICPOLARITY_RISING);
				
				//状态机切换
				ranger_cap_wait_down = 0;
				
				break;
			}
		}	
	}

}

/* ==========================================================================
 *  串口 DMA 接收事件回调 —— K210 数据处理的核心入口
 * ==========================================================================
 *  什么时候进这里：
 *    1) IDLE（总线空闲）事件：一帧发完、线路静默 1 个字符时间后触发；
 *       此时 Size = 本次实际收到的字节数（可能是 13、26 或半帧）；
 *    2) DMA 传输完成：缓冲区被填满时触发（本工程 30 字节，正常不会发生）。
 *
 *  为什么先判断 RxState == READY：
 *    HAL 在调用本回调前已经把接收状态机复位成 READY，只有这样才能安全地
 *    再次启动下一次接收；若状态仍为 BUSY_RX 就重复调用 ReceiveToIdle_DMA，
 *    HAL 会直接返回 HAL_BUSY，导致接收再也起不来（串口"假死"）。
 *
 *  为什么要在回调里重新启动接收：
 *    DMA 配置为 NORMAL 单次模式，传完/空闲后通道就停了，
 *    必须每次重新武装，才能形成持续接收。这是 STM32 HAL 的标准套路。
 * ========================================================================== */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if( huart == &huart3 && HAL_UART_STATE_READY == huart->RxState )
	{
		/* 收到数据即刷新"视觉在线"状态：
		   清零计数（1 秒超时的计时器），并置位在线标志。
		   这两个变量在 HAL_GPIO_EXTI_Callback() 里被读取/累加，
		   跨中断共享但都是单字节访问，实际风险很低。 */
		k210_active_cnt=0;
		k210_active_flag=1;

		/* 把本次 DMA 收到的 Size 个字节逐字节喂给组帧状态机。
		   注意这里必须循环全部字节，不能只取前 13 个：
		   K210 连续发送时一次可能收到 26 字节（两帧），
		   少喂就会丢帧；多喂也无害，状态机会自己找帧头。
		   返回值（是否刚好凑成一帧）本工程不使用。 */
		for(uint8_t i=0;i<Size;i++)
		{
			k210_data_callback(k210_buffer[i]);    //K210.C
		}

		/* 重新武装 DMA + IDLE，准备接下一批数据。
		   若这里返回 HAL_BUSY，通常说明上一次接收没干净结束，
		   可在 HAL_UART_ErrorCallback() 里补救（本工程尚未实现）。 */
		HAL_UARTEx_ReceiveToIdle_DMA(&huart3,k210_buffer,30);
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
