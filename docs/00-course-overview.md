# 课程总览

## 1. 课程定位

本课程以 WHEELTEC C10B/C50C 双板麦克纳姆轮小车为载体，串联：

- 嵌入式工具链与下载；
- OLED 显示与字模；
- 超声测距与舵机扫描；
- MPU6050 姿态与串口通信；
- 编码器里程计、运动学与闭环控制；
- 中期综合任务：3 m 直线、避障暂停、原地掉头、返回。

## 2. 学习路径

| 阶段 | 内容 | 主要产出 |
|---|---|---|
| 0 | 硬件、工具链、烧录 | 两块板能编译、下载、运行 |
| 1 | OLED 汉字距离 | 16×16 字模 + 距离显示 |
| 2 | 舵机 + 单超声 | 可解释的扫描避障方案 |
| 3 | USB 转串口 + 姿态上报 | 串口助手数据 + C50C 显示 |
| 4 | 中期检查 | 3 m 往返状态机与测试记录 |
| 5 | 期末/大作业 | 在稳定基线上扩展 |

## 3. 原始资料与工程

本仓库遵循“原始优先”：

1. 先读原始课件与原始工程；
2. 按 `docs/` 步骤做实验；
3. 需要参考实现时，再看 `firmware/experimental/`；
4. 实验代码未通过实车验收前，不作为标准答案。

原始资料位置：

- 传感器讲义/PPT：`docs/reference/sensors/`
- 机械/电气/安装手册：`docs/reference/hardware/`
- 烧录/ST-LINK 教程：`docs/reference/toolchain/`
- 舵机参考源码：`references/servo-example/`

## 4. 两份固件

| 板卡 | 原始工程 | 作用 |
|---|---|---|
| C10B | `firmware/original/c10b/MDK-ARM/C10B.uvprojx` | 前板传感器、显示、上报 |
| C50C | `firmware/original/c50c/USER/WHEELTEC.uvprojx` | 底盘运动控制、显示 |

实验工程：

- `firmware/experimental/c10b-course/`
- `firmware/experimental/c50c-midterm/`

状态见 [实验工程说明](../firmware/experimental/README.md)。

## 5. 推荐实验记录

每次实验至少记录：

- 日期、板卡、Target、烧录方式；
- 关键接线照片/表；
- OLED/串口现象；
- 测量数据（距离、时间、路径偏差）；
- 失败现象与排查结论；
- 是否通过验收。

## 6. 课程验收原则

- 代码存在不等于功能完成；
- 编译通过不等于硬件可用；
- “已验证”必须有实车记录；
- 原始工程能恢复现场，实验工程用于对照与改进。
