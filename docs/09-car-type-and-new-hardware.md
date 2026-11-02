# 车型与换硬件

## 1. 两层选择

C50C 有两层“车型”：

1. **底盘类型**：Keil Target 决定宏（Akm/Diff/Mec/4WD/Omni）；
2. **具体型号**：板上电位器决定 `TYPE:`，`Robot_Init()` 选择轮距/轴距/轮径/减速比。

两者必须匹配实际硬件。

## 2. Keil Target

| 底盘 | Target | 宏 |
|---|---|---|
| 麦克纳姆 | `Mec_Car` | `MEC_CAR` |
| 四驱 | `4WD_Car` | `_4WD_CAR` |
| 阿克曼 | `Akm_Car` | `AKM_CAR` |
| 两轮差速 | `Diff_Car` | `DIFF_CAR` |
| 全向 | `Omni_Car` | `OMNI_CAR` |

打开原始 `WHEELTEC.uvprojx`，在工具栏目标下拉框选择，再 Rebuild。

## 3. 电位器 TYPE

上电后 OLED 显示 `TYPE: n`。型号映射在 `CarType/*_robot_init.c`。

麦轮原始映射（节选）：

| TYPE | 型号 | 轮径 | 电机减速比 |
|---|---|---|---|
| 0/1 | SENIOR_MEC_BS | 100 mm | MD36N_27 |
| 2 | SENIOR_MEC_DL | 152 mm | MD36N_27 |
| 3/4 | TOP_MEC_BS | 152 mm | MD60N_18/47 |
| 5/6 | TOP_MEC_DL | 152 mm | MD60N_18/47 |
| 7/8 | FLAGSHIP_MEC_BS | 152 mm | MD60N_18/47 |
| 9/10 | FLAGSHIP_MEC_DL | 152 mm | MD60N_18/47 |

## 4. 麦轮的特殊注意

原始 `mec_robot_init.c` 的 `switch(TypeNum)` 被注释，实际固定：

```c
Robot_Init(SENIOR_MEC_BS_wheelspacing,
           SENIOR_MEC_BS_axlespacing,
           MD36N_27, GMR_500,
           Mecanum_100, VEL_KP, VEL_KI);
```

如果新麦轮不是“100 mm 轮径 + MD36N_27”，只改 Target 不够；需要：

- 改 `Robot_Init()` 参数；或
- 取消注释 `switch(TypeNum)`，让电位器选型号。

## 5. 参数位置

- 轮距/轴距/轮径：`CarType/Inc/robot_init.h`
- 麦轮型号选择：`CarType/mec_robot_init.c`
- 其他底盘：对应 `*_robot_init.c`

## 6. 换硬件步骤

1. 确认底盘类型；
2. 选择 Target；
3. 读取 OLED `TYPE:`；
4. 测量轮径、轮距、轴距；
5. 查电机标签（MD36N_27 / MD60N_18 / MD60N_47 等）；
6. 改 `Robot_Init()` 或宏；
7. 架空/低速测试四轮方向与编码器符号；
8. 落地低速直线测试；
9. 更新本文档与测试记录。

## 7. C10B

C10B 与底盘类型无关，继续负责超声/舵机/MPU/OLED/K210；换底盘不需要重烧 C10B，除非协议或显示需求变化。

## 8. 实验代码提醒

`firmware/experimental/c50c-midterm/` 针对当前麦轮代码和 23 字节协议编写；换到底盘类型后，运动学、里程计、掉头逻辑都需要重新验证。不要把实验代码直接套到新车型。
