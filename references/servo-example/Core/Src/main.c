/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * 硬件说明：
  * 1. PA8 -> TIM1_CH1 输出50Hz PWM驱动SG90 9G舵机
  *    500us  = -90°；1500us = 0°中位；2500us = +90°
  * 2. PA0 按键输入，内部下拉，按键接3.3V，按下高电平有效
  * 3. 舵机需独立5V供电，GND与单片机共地，防止压降复位
  * 控制逻辑：
  * 1. 上电舵机归中0°，停止扫动
  * 2. 第一次按下按键：启动舵机±90°连续往复转动
  * 3. 再次按下按键：停止扫动，保持当前角度
  * 4. 全程非阻塞：按键消抖、角度步进均使用系统滴答时间戳，无延时阻塞
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//TIM_HandleTypeDef htim1;

/* ---------------- 舵机 PWM 参数 ---------------- */
#define SERVO_TIM_CLK_HZ      72000000UL
#define SERVO_PSC             (72U - 1U)      /* 1 MHz 计数, 1 tick = 1us   */
#define SERVO_ARR             (20000U - 1U)  /* 20 ms 周期 = 50 Hz         */

/* 9G 舵机典型脉宽: 0.5ms~2.5ms 对应 0°~180°
 * 映射到 ±90°:  -90° -> 500us,  0° -> 1500us,  +90° -> 2500us            */
#define SERVO_MIN_US          500U         // 最小脉冲宽度 -90° 
#define SERVO_CENTER_US       1500U        // 中位脉冲宽度 0°
#define SERVO_MAX_US          2500U        // 最大脉冲宽度 +90°

/* ---------------- 扫动速度参数 (非阻塞时间片) ----------------
 * 每 SWEEP_INTERVAL_MS 毫秒, 脉宽走 SWEEP_STEP_US 微秒
 * 单边行程时间 = (MAX-MIN)/STEP * INTERVAL
 *   = 2000/5 * 10ms = 4.0 s   (可按需调大 STEP 加快)                     */
#define SWEEP_STEP_US         5U          // 每次角度变化脉冲增量，数值越大转动越快
#define SWEEP_INTERVAL_MS     10U         // 每次更新角度间隔ms，非阻塞时间片

/* ---------------- 按键参数 ---------------- */
#define BTN_PORT              GPIOA
#define BTN_PIN               GPIO_PIN_0
#define BTN_ACTIVE            GPIO_PIN_SET
#define BTN_DEBOUNCE_MS       20U         // 按键消抖稳定时间20ms

/* ---------------- 运行时状态 ---------------- */
static volatile uint8_t  g_running    = 0;
static volatile uint16_t g_pulse_us   = SERVO_CENTER_US;
static volatile int8_t   g_dir        = 1;     /* +1: 往 +90°; -1: 往 -90°  */

static uint32_t g_last_step_tick   = 0;

/* 按键消抖状态机 */
static GPIO_PinState btn_raw_last   = GPIO_PIN_RESET;   // 按键上一次的原始电平
static GPIO_PinState btn_stable     = GPIO_PIN_RESET;   // 消抖后稳定按键电平
static uint32_t      btn_last_chg   = 0;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ---------------- 函数原型 ---------------- */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);       //加static 限定内部链接，防止重复定义
static void MX_TIM1_Init(void);

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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();	//优先调用本文件定义的同名函数初始化
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */  
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, SERVO_CENTER_US);   // 初始化TIM1 CH1 PWM输出，输出上电中位1500us脉冲 
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);           //通道使能和主输出使能，启动定时器
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
        uint32_t now = HAL_GetTick();

        /* ---------- 1. 按键: 非阻塞消抖 + 上升沿触发 ---------- */
        GPIO_PinState raw = HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN);     //读取PA0当前原始输入电平

        if (raw != btn_raw_last) {          /* 电平变化, 重置消抖计时 */
            btn_raw_last = raw;       // 更新上一次电平缓存
            btn_last_chg = now;       // 记录跳变时刻，重置消抖计时
        }
        if ((now - btn_last_chg) >= BTN_DEBOUNCE_MS) {   //电平保持稳定超过消抖时间，确认有效电平
            if (raw != btn_stable) {        /* 稳定后的新状态 */
                btn_stable = raw;        // 稳定电平与缓存电平不一致，更新稳定按键状态
                /* 上升沿 = 按下 (高电平有效) -> 切换启停 */
                if (btn_stable == BTN_ACTIVE) {
                    g_running = !g_running;  //稳定状态是高电平，切换运行状态
                }
            }
        }

        /* ---------- 2. 扫动: 时间片步进, 非阻塞 ---------- */
        if (g_running && (now - g_last_step_tick) >= SWEEP_INTERVAL_MS) { // 判断是否到达角度刷新时间片
            g_last_step_tick = now;         // 更新本次刷新时间戳

            uint16_t p = g_pulse_us;      //当前脉冲电平时间 
            if (g_dir > 0) {
                p += SWEEP_STEP_US;       // 根据方向增减脉冲宽度
                if (p >= SERVO_MAX_US) { p = SERVO_MAX_US; g_dir = -1; }   // 到达正负90°边界，反转转动方向
            } else {
                p -= SWEEP_STEP_US;       // 根据方向增减脉冲宽度
                if (p <= SERVO_MIN_US) { p = SERVO_MIN_US; g_dir = 1; }    // 到达正负90°边界，反转转动方向
            }
            g_pulse_us = p;
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, p);  // 将计算后的脉冲值写入TIM1_CH1捕获比较寄存器，更新PWM高电平时间
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* ---------------- TIM1: PWM 50Hz on PA8 (CH1) ---------------- */
static void MX_TIM1_Init(void)
{
    TIM_OC_InitTypeDef oc = {0};

    htim1.Instance               = TIM1;
    htim1.Init.Prescaler         = SERVO_PSC;                    //71
    htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim1.Init.Period            = SERVO_ARR;                    //20ms
    htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim1);

    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = SERVO_CENTER_US;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    oc.OCIdleState= TIM_OCIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&htim1, &oc, TIM_CHANNEL_1);

    /* TIM1 是高级定时器, 必须使能主输出 MOE */		
    __HAL_TIM_MOE_ENABLE(&htim1);    /* Enable the main output */
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);   /* 保险, 不影响 */
}

/* ---------------- GPIO: PA0 输入, PA8 复用推挽 ---------------- */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();

    /* PA0: 按键输入, 高电平有效, 内部下拉(外部再用 10k 下拉更稳) */
    gpio.Pin  = BTN_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BTN_PORT, &gpio);

    /* PA8: TIM1_CH1 复用推挽输出 */
    gpio.Pin   = GPIO_PIN_8;
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);
}

/* HAL 时基 (默认 SysTick) */
//void SysTick_Handler(void)
//{
//    HAL_IncTick();
//}

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
