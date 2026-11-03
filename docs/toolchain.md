# 工具链与烧录

## 1. 软件清单

| 软件 | 用途 | 备注 |
|---|---|---|
| Keil MDK 5.41 | 编译/下载 | 工程使用 ARMCC v5 |
| ARM Compiler 5 | 编译 | 课程包提供 ARMCC 压缩包 |
| STM32F1xx DFP | C10B Pack | 必须安装，否则无 Flash 算法 |
| STM32F4xx DFP | C50C Pack | 必须安装 |
| ST-LINK 驱动 | 下载器 | `dpinst_amd64.exe` |
| CH9102 驱动 | 板载 USB 串口 | Windows 驱动 |
| FlyMcu | USB 一键下载 | 可选备用 |
| SSCOM | 串口调试 | 可选 |

## 2. Keil 工程与 Target

| 板卡 | 工程 | Target |
|---|---|---|
| C10B | `firmware/original/c10b/MDK-ARM/C10B.uvprojx` | `C10B` |
| C50C | `firmware/original/c50c/USER/WHEELTEC.uvprojx` | `Mec_Car` 等 |

实验工程同理，分别打开 `firmware/experimental/...` 下的 `.uvprojx`。

## 3. 编译

1. 打开 `.uvprojx`；
2. 选择正确 Target；
3. 点 **Rebuild**（不是 Build），确保源文件真的重新编译；
4. 输出应为 `0 Error(s)`；
5. 记录代码大小和警告数。

## 4. Flash 下载算法

### C10B（STM32F103RCT6，256 KB）

魔术棒 → Utilities → Settings → Programming Algorithm：

```text
STM32F10x High-density Flash    0x08000000    0x00040000
```

如果为空或选成 Med-density，会出现：

- `Flash Download failed - Cortex-M3`
- `No Algorithm found for 0x08000000`
- `Programming Done` 但校验内容不匹配

### C50C（STM32F407VET6）

选择对应的 STM32F4xx High-density/512 KB 算法，由 DFP 提供。

## 5. 下载设置

Debug → Settings：

- Port：SW；
- Max Clock：先 1.8 MHz；
- Reset：Connect under Reset（连不上时）；
- SW Device 应显示芯片 ID。

Flash Download：

- Erase Full Chip；
- Program；
- Verify；
- Reset and Run。

## 6. 常见下载问题

| 现象 | 优先检查 |
|---|---|
| `No ST-LINK detected` | USB 线、驱动、设备管理器 |
| `Cannot access target` | 板子供电、SWD 四线、Connect under Reset |
| `Flash Download failed - Cortex-M3` | F1 DFP、High-density 算法、全片擦除 |
| `Programming Done` 但内容不匹配 | 算法错误、写保护、旧固件未擦除 |
| 校验读回旧固件 | 全片擦除、选项字节/写保护、换 ST-LINK |

## 7. USB 一键下载（备用）

若 ST-LINK 无法写入，可使用板载 CH9102 + FlyMcu：

1. 安装 CH9102 驱动，设备管理器出现 `USB-Enhanced-SERIAL CH9102 (COMx)`；
2. 打开 FlyMcu；
3. 选择 COMx、115200；
4. 勾选 DTR 低电平复位、RTS 高电平进 BL；
5. 选择 Keil 生成的 `C10B.hex`；
6. 勾选“编程后执行”，开始编程。

注意：FlyMcu 烧录的是 hex，不是 axf；hex 在 Keil 输出目录中。

## 8. 串口调试

1. USB-TTL 接 C10B USART1（RX→PA9、TX→PA10、GND）；
2. 打开 SSCOM，选择 COMx；
3. 115200-8-N-1，无流控；
4. 打开串口，应看到调试输出。

若“端口”列表为空：先在设备管理器中确认 COMx 存在，再打开串口助手。
