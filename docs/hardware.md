# 硬件与接线

## 1. 系统组成

```text
24 V 电池
 ├─ 24 V→12 V → C10B（前控制板）
 ├─ 24 V → C50C（底板）
 └─ 24 V → D50A 电机驱动 ×2

C10B ── UART5 ── C50C
  │                 │
  ├ OLED            ├ OLED
  ├ MPU6050         ├ ICM20948
  ├ HC-SR04         ├ 4×编码器
  ├ 舵机            └ 4×电机
  └ K210
```

## 2. C10B 关键引脚

| 功能 | 引脚 | 说明 |
|---|---|---|
| OLED DC / RES / SDA / SCL | PB3 / PB4 / PB5 / PC14 | 4 线 SPI |
| MPU6050 SCL / SDA / INT | PB14 / PB15 / PB9 | 软件 I2C + 外部中断 |
| 超声 Trig | PC15 | GPIO 输出 |
| 超声 Echo | PA1 | TIM2_CH2 输入捕获 |
| 舵机信号 | PA8 | TIM1_CH1，50 Hz PWM |
| 蜂鸣器 | PA15 | 有源蜂鸣器，高电平驱动；H2 跳线帽需插上 |
| 调试串口 USART1 TX/RX | PA9 / PA10 | USB 转串口 / 下载 |
| K210 串口 | PC10 / PC11 | 工程中为 USART3 |
| 到 C50C TX/RX | PC12 / PD2 | UART5 |

## 3. C50C 关键引脚

| 功能 | 引脚 | 说明 |
|---|---|---|
| 到 C10B TX/RX | PA2 / PA3 | USART2，AF7 |
| OLED | PD4–PD7 | 4 线 SPI |
| 调试/ROS | USART1 / USART3 | PA9/PA10、PB10/PB11 |
| 蓝牙 | UART4 | PC10/PC11 |
| ICM20948 | I2C | 见原工程 |
| 电机/编码器 | 原车接口 | A/B/C/D 四轮 |

## 4. 两板串口连线

**必须交叉 TX/RX，并共地。**

```text
C10B PC12 (UART5_TX)  ─────→  C50C PA3 (USART2_RX)
C10B PD2  (UART5_RX)  ←─────  C50C PA2 (USART2_TX)
C10B GND              ───────  C50C GND
```

常见错误：

- TX 接 TX、RX 接 RX；
- 只接两根信号线，没有共地；
- 插到 K210 的 PC10/PC11 或 USB 调试口；
- 用直通线连接本应交叉的接口。

## 5. ST-LINK 下载

### C10B

H10 外侧四针：

| ST-LINK | C10B |
|---|---|
| 3.3V | 3V3 |
| GND | G |
| SWDIO | A13 |
| SWCLK | A14 |

### C50C

右上角四针：

| ST-LINK | C50C |
|---|---|
| 3.3V | 3V3 |
| GND | GND |
| SWDIO | A13 |
| SWCLK | A14 |

注意：ST-LINK 的 3.3V 只作参考/小电流供电；整车测试时必须打开小车电池，避免“能连上但 Flash 写不进去”。

## 6. 舵机与超声

舵机三线：

```text
棕/黑 → GND
红    → 5 V
黄/橙 → PA8
```

超声四线：

```text
VCC  → 5 V
Trig → PC15
Echo → PA1
GND  → GND
```

要求：

- 超声固定在舵机舵盘上，1500 µs 对应正前方；
- 不要用 ST-LINK 3.3V 给舵机供电；
- 舵机抖动/复位时，使用独立 5 V 电源并与主板共地；
- 插头方向按板上 `S/+/-` 或 `PWM/5V/GND` 丝印，避免反插。

### 蜂鸣器（第一讲 p41–44）

- C10B 使用**有源蜂鸣器**，给电即响；
- 引脚 PA15，高电平驱动，经三极管放大；
- H2 跳线帽必须插上，否则板上没有通路；
- 有源/无源外观相似：无源两脚电阻约 15 Ω，需要 2–4 kHz 方波；有源两脚电阻 MΩ 以上。

## 7. 调试串口

USB-TTL 接 C10B USART1：

```text
USB-TTL RX  → C10B PA9 (TX)
USB-TTL TX  → C10B PA10 (RX)
USB-TTL GND → C10B GND
```

串口助手参数：115200、8 数据位、1 停止位、无校验、无流控。

## 8. 上电顺序与安全

1. 接线完成后检查 5 V/3.3 V 无短路；
2. 先开小车电池，再开使能开关；
3. 下载时确认 ST-LINK 与目标板共地；
4. 首次运行架空或留 3.5 m 空间；
5. 手边随时可断电；
6. 舵机/超声固定牢固，避免运动中掉落。

## 9. C50C 车型选择

C50C 有两层车型配置：

| 层级 | 设置位置 | 作用 |
|---|---|---|
| 底盘类型 | Keil Target | `Akm_Car`、`Diff_Car`、`Mec_Car`、`4WD_Car`、`Omni_Car` |
| 具体型号 | 板上电位器 `TYPE` | 选择轮距、轴距、轮径和减速比 |

本课程使用 `Mec_Car`、`TYPE=1`、SENIOR_MEC_BS、100 mm 轮径、MD36N_27。

麦轮常用型号：

| TYPE | 型号 | 轮径 | 减速比 |
|---|---|---:|---|
| 0/1 | SENIOR_MEC_BS | 100 mm | MD36N_27 |
| 2 | SENIOR_MEC_DL | 152 mm | MD36N_27 |
| 3/4 | TOP_MEC_BS | 152 mm | MD60N_18/47 |
| 5/6 | TOP_MEC_DL | 152 mm | MD60N_18/47 |

原始 `mec_robot_init.c` 固定使用 SENIOR_MEC_BS 参数。换成其他麦轮型号时，需要修改 `Robot_Init()`，不能只切换 Target。

## 10. 新板首轮检查

1. 断电，用小螺丝刀把 C50C 车型电位器调到 `TYPE=1`；
2. 上电确认 OLED 显示 `TYPE: 1`；
3. 接 24 V 电池，确认电压约 22–25 V；
4. 架空车轮，低速确认四轮方向；
5. 烧录课程固件后，检查 C50C 第 4 页 `LEN=23`；
6. C10B/C50C 通信正常后再落地运行。

新板默认 `TYPE=6` 时，轻微触碰可能引起车轮持续转动。先确认车型，再排查程序。

## 11. C50C 电压显示

C50C OLED 的 `V` 是 24 V 电池电压，采样路径为 `PB0`、`ADC1_IN8`，约 11:1 分压。

| 显示 | 含义 |
|---|---|
| 22–25 V | 24 V 电池正常 |
| 低于 20 V | 触发低电压保护 |
| 0.02 V | ADC 输入接近 0 |

只接 USB 或 ST-LINK 时，`0.02 V` 是可能的。整车测试前必须接通 24 V 电池并测量实际电压。
