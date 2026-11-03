# 原始工程（权威基线）

本目录保存课程包中的原始工程，用于对照课件、恢复现场和作为实验起点。

## C10B

- 路径：`c10b/MDK-ARM/C10B.uvprojx`
- Target：`C10B`
- MCU：STM32F103RCT6
- 内容：OLED、MPU6050 + DMP、HC-SR04、K210、UART5 12 字节上报

## C50C

- 路径：`c50c/USER/WHEELTEC.uvprojx`
- MCU：STM32F407VET6
- Target：`Akm_Car`、`Diff_Car`、`Mec_Car`、`4WD_Car`、`Omni_Car`
- 当前麦轮硬件：`Mec_Car`

## 约束

- 不要直接修改本目录；
- 实验性改动放到 `firmware/experimental/`；
- 原始工程中的 `.uvoptx`、`.uvguix` 和构建产物已剔除，首次打开 Keil 会重新生成用户配置。
