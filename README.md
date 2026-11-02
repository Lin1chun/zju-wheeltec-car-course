# 浙江大学《小车传感器与电机控制》课程资源库

WHEELTEC C10B / C50C 双控制板小车的课程工程、实验指导、串口协议与中期检查资料。

> **状态说明（重要）**
>
> - `firmware/original/` 是课程原始基线，实验步骤以原始工程和原始课件为准。
> - `firmware/experimental/` 是在课程任务上做出的扩展实现，**尚未完成实车验收**。
>   其中仍存在 C10B → C50C 通信、MPU6050 姿态输出等未闭环问题。
> - 在通过完整测试前，实验代码只能作为参考，不应作为标准答案或直接用于评分。

> **当前分支：`course/2026-midterm`**。本分支包含完整实验代码；`main` 只保留原始基线和实验入口说明。

## 1. 课程目标

本课程围绕 WHEELTEC 麦克纳姆轮小车，完成从硬件、工具链、传感器、显示、串口通信到底盘运动控制的完整训练。

## 2. 课程进度（按实际课次）

| 课次 | 内容 | 对应文档 |
|---|---|---|
| 1 | PPT 大课：第一讲 显示与执行（OLED、超声、蜂鸣器、舵机） | [课程进度](docs/00-lesson-map.md)、[OLED](docs/03-lab-oled-distance.md)、[舵机/超声](docs/04-lab-servo-ultrasonic.md) |
| 2–3 | 小车组装 | [硬件与接线](docs/01-hardware-and-wiring.md) |
| 4 | PPT 大课：第二讲 姿态感知（MPU6050/DMP） | [串口与姿态](docs/05-lab-serial-attitude.md) |
| 5 | 电机控制实训 | [第五次课：电机控制](docs/11-motor-control-lab.md) |
| 6 | PPT 大课：第三讲 串口通讯 | [串口协议](docs/07-serial-protocol.md) |
| 7 | 综合实验：汉字距离 + 单超声扫描 + 姿态上报 | [第七次课综合任务](docs/12-lesson7-integration.md) |
| 中期 | 3 m 直线、避障暂停、掉头、返回 | [中期检查](docs/06-midterm-3m-round-trip.md) |

往届学生经验（主观，含吐槽与团队建议）：[往届经验与总结](docs/13-student-experience.md)。

核心任务原文：

- 第一讲 p56 课后任务：只使用一个超声波的情况下如何实现更大范围的避障？
- 第三讲 p165 课后任务：把 C10B 计算的姿态数据发给 C50C，并在 OLED 上显示。
- 16×16 汉字距离是第七次课的课程布置；PPT 只写“汉字要自己取模/尝试中文”，没有逐字写出 16×16 和“距离0.05米”。

## 3. 硬件组成

| 部件 | 型号 | 作用 |
|---|---|---|
| 前控制板 C10B | STM32F103RCT6 | 超声测距、舵机、MPU6050、OLED、K210 视觉、向 C50C 上报 |
| 底板控制板 C50C | STM32F407VET6 | 四轮运动控制、编码器、ICM20948、OLED、接收 C10B 数据 |
| 电机驱动 | D50A ×2 | AB、CD 四路直流电机驱动 |
| 电机/编码器 | MD36LP27-24V + GMR | 四轮闭环速度与里程 |
| 超声 | HC-SR04 | 2 cm–400 cm，约 15° 波束 |
| 舵机 | SG90 类 9 g 舵机 | 带动超声波扫描 |
| 显示 | SSD1306 128×64 OLED | C10B / C50C 各一块 |
| 电源 | 24 V 电池 + 24 V→12 V | 整车与 C10B 供电 |

完整接口和接线见 [硬件与接线](docs/01-hardware-and-wiring.md)。

## 3.1 原始资料权威来源

课程资料由老师通过 FTP 发布：

- 主机：`10.13.12.198`
- 端口：`3301`
- 用户名：`zdhsx1`
- 密码：按老师通知/课程群获取，**不写入 Git**

连接方法、FileZilla 设置和排错见 [课程 FTP 资料源](docs/reference/ftp.md)。本地 `E:\ZJU_Documents\Course\car` 是镜像；若与 FTP 不一致，以 FTP 为准。

## 4. 仓库结构

```text
.
├── README.md
├── AGENTS.md                     # 后续维护者/智能体的工作约束
├── CONTRIBUTING.md               # 学生与协作者提交规范
├── NOTICE.md                     # 课程资料与第三方代码声明
├── docs/
│   ├── 00-course-overview.md     # 课程总览与学习路径
│   ├── 00-lesson-map.md          # 七次课实际进度与资料页码
│   ├── 00-course-package-inventory.md # 原始资料清单与取舍
│   ├── 01-hardware-and-wiring.md
│   ├── 02-toolchain-and-flashing.md
│   ├── 03-lab-oled-distance.md
│   ├── 04-lab-servo-ultrasonic.md
│   ├── 05-lab-serial-attitude.md
│   ├── 06-midterm-3m-round-trip.md
│   ├── 07-serial-protocol.md
│   ├── 08-troubleshooting.md
│   ├── 09-car-type-and-new-hardware.md
│   ├── 10-maintenance-and-roadmap.md
│   ├── 11-motor-control-lab.md   # 第五次课
│   ├── 12-lesson7-integration.md # 第七次课三项任务
│   ├── 13-student-experience.md  # 往届经验（主观）
│   └── reference/                # 课程原始 PDF/JPG/DOCX（内部使用）
├── firmware/
│   ├── original/
│   │   ├── c10b/                 # 原始 C10B 参考工程（权威基线）
│   │   └── c50c/                 # 原始 C50C 工程（权威基线）
│   └── experimental/             # main 上仅保留入口说明
├── references/
│   └── servo-example/            # 原始舵机参考例程的关键源码
└── scripts/                      # 构建/检查辅助脚本
```

## 5. 快速开始

### 4.1 软件环境

- Keil MDK 5.41；
- ARM Compiler 5（工程使用 `Use default compiler version 5`）；
- `Keil.STM32F1xx_DFP` 与 `Keil.STM32F4xx_DFP`；
- ST-LINK V2 驱动；
- 板载 CH9102 USB 转串口驱动；
- 可选：FlyMcu（USB 一键下载）、SSCOM（串口调试）。

安装、下载与常见报错处理见 [工具链与烧录](docs/02-toolchain-and-flashing.md)。

### 4.2 先编译原始工程

| 板卡 | 工程 | Target |
|---|---|---|
| C10B | `firmware/original/c10b/MDK-ARM/C10B.uvprojx` | `C10B` |
| C50C | `firmware/original/c50c/USER/WHEELTEC.uvprojx` | `Mec_Car` |

步骤：

1. Keil 打开 `.uvprojx`；
2. 选择上表 Target；
3. Rebuild，确认 `0 Error(s)`；
4. ST-LINK 接 SWDIO / SWCLK / GND / 3V3；
5. LOAD，复位运行。

### 4.3 再按课程顺序做实验

按实际课次：

0. [完成检查清单](docs/CHECKLIST.md)
1. [课程进度对照](docs/00-lesson-map.md)
2. 第 1 课：[课程总览](docs/00-course-overview.md)、[OLED](docs/03-lab-oled-distance.md)、[舵机/超声](docs/04-lab-servo-ultrasonic.md)
3. 第 2–3 课：[硬件与接线](docs/01-hardware-and-wiring.md)、[工具链与烧录](docs/02-toolchain-and-flashing.md)
4. 第 4 课：[姿态与串口基础](docs/05-lab-serial-attitude.md)
5. 第 5 课：[电机控制实训](docs/11-motor-control-lab.md)
6. 第 6 课：[串口协议](docs/07-serial-protocol.md)
7. 第 7 课：[综合任务](docs/12-lesson7-integration.md)
8. 中期：[3 m 往返](docs/06-midterm-3m-round-trip.md)

## 6. 原始基线 vs 实验实现

### `firmware/original/`

- 来自课程包原始工程，优先用于对照课件和恢复现场。
- C10B：原始 OLED ASCII 显示、MPU6050 + DMP、20 Hz 12 字节上报。
- C50C：原始多车型工程，支持 `Akm_Car`、`Diff_Car`、`Mec_Car`、`4WD_Car`、`Omni_Car`。
- **不要直接修改该目录**；实验性改动放到 `firmware/experimental/` 或独立分支。

### `course/2026-midterm` 分支

完整的实验代码不在 `main`，而在 `course/2026-midterm` 分支：

- C10B：16×16 汉字“距离/路程/均速”、舵机五扇区扫描、TIM7 独立 100 Hz 时基、23 字节上报帧；
- C50C：23 字节协议解析、OLED 第 1/2/4 页、3 m 往返状态机、陀螺仪辅助 180° 掉头。

当前状态：**仅做过源码级语法检查，未完成实车验收**。已知问题见
[实验分支说明](firmware/experimental/README.md)。

切换：

```powershell
git switch course/2026-midterm
```

## 7. 串口协议摘要

原始 C10B 帧为 **12 字节**，实验帧扩展为 **23 字节**。两端必须使用同一版本，否则会出现“有字节但帧校验失败”或完全收不到数据。

| 版本 | 长度 | 内容 |
|---|---:|---|
| 原始 | 12 | `CC DD`、K210 状态/坐标/面积、距离、温度、BCC |
| 实验 | 23 | 原始字段 + 前向/扫描最小距离、Pitch/Roll/Yaw、舵机扇区、扫描掩码 |

完整字段、BCC 与兼容性规则见 [串口协议](docs/07-serial-protocol.md)。

## 8. 换车型/换硬件

C50C 原始工程通过 Keil Target 选择底盘类型，通过板上电位器选择具体型号：

- Target：`Mec_Car` / `4WD_Car` / `Akm_Car` / `Diff_Car` / `Omni_Car`；
- 电位器：决定 `TYPE:` 编号和 `Robot_Init()` 参数；
- 麦轮原始工程的型号 `switch` 被注释，实际固定为 SENIOR_MEC_BS + 100 mm 轮径，换麦轮型号时需要手动改参数。

详见 [车型与换硬件](docs/09-car-type-and-new-hardware.md)。

## 9. 验证状态

| 项目 | 状态 |
|---|---|
| 原始工程 | 课程包基线，未在本仓库内重新编译 |
| 实验 C10B（分支） | GCC 语法检查通过；未实车验收 |
| 实验 C50C（分支） | GCC 语法检查通过；未实车验收 |
| C10B→C50C 通信 | 未闭环，需现场继续排查接线/协议 |
| MPU6050 姿态 | 现场出现 P/R/Y 恒为 0，原因待定位 |
| 3 m 中期任务 | 逻辑已实现，未完成实车验收 |

**任何“完成”结论必须以实车测试记录为准，不以代码存在为准。**

## 10. 后续维护

- 中期检查后的更新：新建 `course/2026-midterm` 分支，保留原始基线；
- 期末考试/大作业：新建 `course/2027-final` 分支；
- 稳定版本打标签：`midterm-2026`、`final-2027`；
- 仅通过 GitHub Releases 发布 `.hex`，不把构建产物提交到 Git；
- 具体流程见 [维护与路线图](docs/10-maintenance-and-roadmap.md)。

课程相关软件/驱动/Keil Pack 的安装位置与卸载方式见 [电脑清理指引](docs/14-pc-cleanup.md)。清理时不要删除原始资料和仓库 `.git`。

## 11. 版权与使用范围

仓库包含 WHEELTEC 课程原始工程与课程 PDF，仅供本项目组课程学习使用。原始资料版权归 WHEELTEC / 浙江大学课程组所有；未经授权不得公开转载或用于商业用途。详见 [NOTICE.md](NOTICE.md)。

## 12. 原始资料索引

- 完整资料清单与取舍：[课程原始资料清单](docs/00-course-package-inventory.md)
- 传感器完整讲义：`docs/reference/sensors/传感器与串口通讯-完整版.pdf`
- 传感器 PPT：`docs/reference/sensors/传感器大课ppt.pdf`
- 机械安装与下载手册：`docs/reference/hardware/03 麦轮小车机械安装与下载程序手册20260918.pdf`
- 电气连接图：`docs/reference/hardware/01 麦轮小车电气连接图20260918更新.jpg`
- 舵机参考例程：`references/servo-example/`
