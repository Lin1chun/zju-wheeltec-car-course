# ZJU WHEELTEC 小车课程

浙江大学 WHEELTEC C10B / C50C 双控制板小车课程仓库，包含原始工程、课程实验、23 字节串口协议、中期检查实现和参考资料。

## 当前版本

| 项目 | 状态 |
|---|---|
| 课程文档 | 已完成 |
| C10B 课程工程 | Keil Rebuild 通过，实车验收通过 |
| C50C 中期工程 | Keil Rebuild 通过，实车验收通过 |
| 中期检查 | 3 m 往返、50 cm 避障暂停、原地掉头、OLED 统计通过 |
| 稳定版本 | `midterm-2026` |

中期参数：`MID_SPEED=0.20f`、`LineDiffParam=52`、`MID_TURN_MAX=0.50f`。

## 课程内容

| 课次 | 内容 | 文档 |
|---|---|---|
| 1 | OLED、超声、蜂鸣器与舵机 | [课程总览](docs/course-map.md) |
| 2–3 | 小车机械与电气装配 | [硬件与接线](docs/hardware.md) |
| 4 | MPU6050 姿态与 DMP | [姿态与串口](docs/labs/serial-attitude.md) |
| 5 | 电机控制实训 | [电机控制](docs/labs/motor-control.md) |
| 6 | 串口通信 | [串口协议](docs/protocol.md) |
| 7 | 汉字距离、单超声扫描、姿态上报 | [综合实验](docs/labs/integration.md) |
| 中期 | 3 m 直线往返与避障 | [中期检查](docs/midterm.md) |

## 仓库结构

```text
.
├── docs/                     课程、实验、协议与维护文档
│   ├── labs/                 第五、七次课实验
│   └── reference/            原始 PDF、PPT、图片和工具说明
├── firmware/
│   ├── original/             厂商原始工程，只读基线
│   └── experimental/         课程扩展工程
├── references/               小型参考代码
├── scripts/                  仓库检查工具
├── AGENTS.md                 维护规则
├── CONTRIBUTING.md           分支、提交与测试规范
└── CHANGELOG.md              版本变更记录
```

## 快速开始

### 原始工程

| 板卡 | 工程 | Keil Target |
|---|---|---|
| C10B | `firmware/original/c10b/MDK-ARM/C10B.uvprojx` | `C10B` |
| C50C | `firmware/original/c50c/USER/WHEELTEC.uvprojx` | `Mec_Car` |

### 课程验收工程

| 工程 | 工程文件 | Keil Target |
|---|---|---|
| C10B | `firmware/experimental/c10b-course/MDK-ARM/C10B.uvprojx` | `C10B` |
| C50C | `firmware/experimental/c50c-midterm/USER/WHEELTEC.uvprojx` | `Mec_Car` |

操作顺序：

1. 打开 `.uvprojx`；
2. 选择对应 Target；
3. 执行 `Rebuild`，确认 `0 Error(s)`；
4. 通过 ST-LINK 执行 `LOAD`；
5. 复位运行并记录 OLED、串口与实车结果。

烧录方法见 [工具链与烧录](docs/toolchain.md)，接线见 [硬件](docs/hardware.md)。

## 文档入口

- [课程进度与验收范围](docs/course-map.md)
- [硬件与接线](docs/hardware.md)
- [工具链与烧录](docs/toolchain.md)
- [OLED 汉字距离](docs/labs/oled-distance.md)
- [舵机与单超声扫描](docs/labs/servo-ultrasonic.md)
- [姿态采集与串口](docs/labs/serial-attitude.md)
- [中期检查](docs/midterm.md)
- [串口协议](docs/protocol.md)
- [故障排查](docs/troubleshooting.md)
- [参考资料索引](docs/reference/README.md)

## 版本管理

- `main` 只保留已验证版本和课程文档；
- 功能开发使用英文分支名，例如 `feature/ultrasonic-scan`；
- 稳定提交使用标签，例如 `midterm-2026`；
- 提交信息使用中文，格式见 [CONTRIBUTING.md](CONTRIBUTING.md)；
- `.hex`、`.bin`、`OBJ/` 和 Keil 用户文件不进入 Git。

## 原始资料

课程资料由老师通过 FTP 发布，地址和连接方式见 [FTP 资料源](docs/reference/ftp.md)。密码不入库。

原始工程和课程资料版权归 WHEELTEC 及浙江大学课程组所有，使用范围见 [NOTICE.md](NOTICE.md)。
