# AGENTS.md

本文件面向后续维护本仓库的智能体、助教和学生。目标是：**原始资料优先、实验代码可追溯、任何结论可复现**。

## 1. 项目定位

本仓库是浙江大学 WHEELTEC C10B/C50C 小车课程的工程与教学资料库，包含：

- 原始课程工程（权威基线）；
- 课程实验指导与接线/排错文档；
- 未实车验收的实验实现；
- 后续中期检查、期末项目扩展的维护规范。

## 2. 最高优先级规则

1. **原始优先**：`firmware/original/` 是权威基线。不要覆盖、重排或“顺手重构”该目录。
2. **实验代码必须标注状态**：完整实验代码在 `course/2026-midterm` 分支，未完成实车验收；`main` 的 `firmware/experimental/` 只是入口说明。
3. **先验证再升级**：任何实验实现通过完整测试后，才能合并到 `main` 或标记为课程标准答案。
4. **协议两端同步**：修改 C10B/C50C 串口帧时，必须同时更新两端结构体、BCC、长度、文档 `docs/07-serial-protocol.md` 和状态页。
5. **不要用 CubeMX 重新生成 C10B 工程**：TIM1 舵机、超声扫描、字库等为手工集成，重新生成会覆盖。
6. **不要提交构建产物、安装包、视频或厂商大文件**：见 `.gitignore`，二进制通过 Releases 发布。
7. **课程进度以 `docs/00-lesson-map.md` 为准**：第一讲/第二讲/第三讲分别对应第 1/4/6 次课；第 2–3 次课组装，第 5 次课电机实训，第 7 次课综合任务。不要按技术难度重排课程。
8. **区分 PDF 原文与课程布置**：16×16、距离示例、3 m 中期要求等并非原 PDF 逐字内容；文档必须标注来源，不能把课程布置写成课件原文。
9. **FTP 凭据不入库**：FTP 地址/端口/用户名可写，密码只放本地密码管理器；禁止提交 `ftp.local.*` 或任何含密码的文件。
10. **往届经验是主观资料**：`docs/13-student-experience.md` 只用于预期/组队/硬件准备，不得当作评分标准；引用时保留免责声明，教师姓名做泛化处理。

## 3. 仓库地图

```text
firmware/original/c10b      原始 C10B 参考工程
firmware/original/c50c      原始 C50C 多车型工程
firmware/experimental/      main 上的实验入口（完整代码见 course/2026-midterm）
docs/                       课程步骤、接线、协议、排错、换硬件
docs/00-lesson-map.md        七次课进度与 PDF 页码对照（权威）
docs/11-motor-control-lab.md 第五次课电机控制实训
docs/12-lesson7-integration.md 第七次课三项任务
docs/reference/             课程原始 PDF/JPG/DOCX（内部使用）
references/servo-example/   原始 PA8/TIM1 舵机例程关键源码
scripts/                    构建/检查辅助脚本
```

## 4. 当前硬件与 Target

- C10B：STM32F103RCT6，Keil Target `C10B`。
- C50C：STM32F407VET6，当前麦轮硬件使用 Target `Mec_Car`。
- 其他 Target：`Akm_Car`、`Diff_Car`、`4WD_Car`、`Omni_Car`。
- 原始 C50C 工程一次包含五种底盘，**不要把所有 CarType 文件同时加入同一目标**，否则会重复定义 `robot`。
- 麦轮原始 `mec_robot_init.c` 的型号 `switch` 被注释，默认固定 SENIOR_MEC_BS + 100 mm 轮径；换型号需改 `Robot_Init()` 参数。

## 5. 文件编码约束

该仓库混合了不同编码，禁止无差别转码：

| 文件 | 编码/换行 |
|---|---|
| C10B `Core/Src/main.c` | UTF-8 |
| C10B `Core/Inc/main.h`、`tim.c`、`bsp_oled.c` 等 | GBK/ANSI |
| C50C 大部分 `.c/.h` | GBK/ANSI |
| 新增 Markdown | UTF-8 |

编辑原则：

- 优先使用能保持原字节的编辑方式；
- 不要用“UTF-8 另存”覆盖 GBK 文件；
- 不要全文件格式化，避免产生巨大无意义 diff；
- 新增中文注释前先确认目标文件编码。

## 6. 构建与验证

### 6.1 Keil

1. 原始 C10B：打开 `firmware/original/c10b/MDK-ARM/C10B.uvprojx`，Target `C10B`；
2. 原始 C50C：打开 `firmware/original/c50c/USER/WHEELTEC.uvprojx`，Target 按硬件选择；
3. 实验工程：`firmware/experimental/...` 对应 `.uvprojx`；
4. Rebuild（不是 Build）后确认 `0 Error(s)`；
5. LOAD 前确认 Flash 算法为 `STM32F10x High-density Flash`（C10B）/对应 F4 算法（C50C）。

### 6.2 静态检查

在没有完整 Keil 环境时，可用 `arm-none-eabi-gcc -fsyntax-only` 做源码检查。FreeRTOS 的 RVDS 端口需要 GCC 兼容 shim；不要把 shim 提交到固件目录。

静态检查通过**不等于**硬件可用。必须在测试矩阵中记录实车结果。

## 7. 测试矩阵

| 编号 | 测试 | 通过标准 |
|---|---|---|
| T1 | C10B 超声显示 | 纸箱从 30 cm→5 cm，OLED 距离连续变化 |
| T2 | 舵机扫描 | 5 扇区往复，`S` 变化；斜前方 30° 物体可触发 STOP |
| T3 | 串口调试 | USART1 115200 输出 `D/P/R/Y/S/OB` |
| T4 | 姿态上报 | C50C 第 2 页 P/R/Y 随姿态变化 |
| T5 | 3 m 直线 | 起点到停点约 3.00 m，横向偏差在课程要求内 |
| T6 | 遇障暂停 | 前方 50 cm 停车，不移除不前进 |
| T7 | 障碍撤离恢复 | 撤离并稳定 0.4 s 后继续 |
| T8 | 原地掉头 | 180°±容差，无平移 |
| T9 | 返回起点 | 返回 3 m，终点误差可记录 |
| T10 | OLED 统计 | 总路程 ≈ 6 m，均速合理 |

测试记录至少包含：日期、板卡版本、Target、烧录方式、测试人、现象、结论。

## 8. 安全约束

- 首次运行留出至少 3.5 m 直线空间；
- 手边随时可断电池/急停；
- 舵机电源使用 5 V，不与 ST-LINK 3.3 V 混用；
- 超声/舵机固定牢固，避免运动中掉落；
- 障碍测试使用软质纸箱，不使用尖锐/易倒物；
- 任何通信中断都应使底盘停车（链路看门狗）。

## 9. 修改协议时的检查清单

- [ ] C10B `C10B_Sendmsg` 字段顺序/类型；
- [ ] C50C `C10B_Recvmsg` 字段顺序/类型；
- [ ] `#pragma pack(1)` 与 `sizeof` 长度；
- [ ] BCC 覆盖范围（当前为前 N-1 字节异或）；
- [ ] 两端波特率 115200、8N1；
- [ ] C50C 接收计数页 RX/HEAD/OK/BAD；
- [ ] `docs/07-serial-protocol.md`；
- [ ] 变更记录与兼容性说明。

## 10. 车型/硬件变更

流程：

1. 确认底盘类型，选择对应 Target；
2. 读取 C50C OLED 的 `TYPE:`；
3. 核对轮径、轮距、轴距、电机减速比；
4. 修改 `CarType/Inc/robot_init.h` 或 `Robot_Init()` 调用；
5. 先低速、架空/留空测试，再落地；
6. 更新 `docs/09-car-type-and-new-hardware.md`。

详见该文档。

## 11. Git 工作流

- `main`：原始基线 + 已验证文档；
- `course/2026-midterm`：中期检查实验实现；
- `course/2027-final`：后续期末项目；
- 标签：`midterm-2026`、`final-2027`；
- 提交信息：`lab2: fix servo scan timing`、`docs: update wiring table`；
- 大文件（hex/bin/pdf）不入库，hex 通过 Releases 发布。

## 12. 文档维护

每个新增功能至少更新：

1. 对应 `docs/` 实验页；
2. `README.md` 状态/索引；
3. 如涉及协议，更新 `docs/07-serial-protocol.md`；
4. 如涉及车型，更新 `docs/09-car-type-and-new-hardware.md`；
5. 如涉及验证状态，更新本文件测试矩阵/README 状态表。

## 13. 已知未闭环问题

- C10B → C50C 通信：需确认两端协议版本（12/23 字节）、PA3 接线、共地、RX 计数；
- MPU6050 姿态：现场 P/R/Y 恒为 0，需检查 I2C/INT/DMP；
- 实验代码未完成实车验收，不得标记为“已验证”。

## 14. 参考

- 课程原始讲义：`docs/reference/sensors/`
- 硬件手册：`docs/reference/hardware/`
- 舵机参考：`references/servo-example/`
- 维护流程：`docs/10-maintenance-and-roadmap.md`

## 15. 本课程对电脑的安装位置与卸载

课程会留下课程数据、Keil 安装、Keil Pack、USB 驱动、便携工具和编译产物。完整清理步骤见 `docs/14-pc-cleanup.md`；这里只列必须知道的位置：

| 位置 | 内容 | 处理 |
|---|---|---|
| `E:\ZJU_Documents\Course\car` | 课程资料/工程/视频/安装包 | 按分层结构保留，删除临时/重复 |
| `C:\Users\70336\AppData\Local\Keil_v5` | Keil MDK 5.41（约 3.2 GB） | 运行 `Uninstall.exe` |
| `C:\Users\70336\AppData\Local\Arm\Packs` | Keil Pack（约 2.4 GB） | 确认无其他 Keil 项目后再删 |
| `C:\Users\70336\AppData\Local\Temp\course_pdfs` | PDF 文本提取临时文件 | 可直接删除 |
| `E:\...\car\work\tools` | GCC/ARMCC 校验工具（约 2 GB） | 可删除 |
| `E:\...\car\work\*\OBJ` | Keil 构建产物 | 可删除 |
| 设备管理器 `CH343` / `STM32 STLink` | USB 串口/下载器驱动 | 右键卸载设备，必要时删驱动 |
| 注册表 `HKLM\SOFTWARE\WOW6432Node\Keil` | Keil 安装信息 | 用卸载器处理 |

**不要删除**：`firmware/original/`、仓库 `.git`、原始课程 PDF/RAR、个人报告/数据、其他课程共用 Pack。

清理前先读 `docs/14-pc-cleanup.md`；FTP 密码、许可证、个人数据不要写进仓库或清理脚本。
