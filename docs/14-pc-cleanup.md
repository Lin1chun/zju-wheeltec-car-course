# 课程软件与电脑清理指引

本课程会在电脑上留下几类内容：课程资料、Keil 安装、Keil Pack、USB 驱动、便携工具、编译产物和临时解压文件。清理时先分清“课程数据”和“系统安装”，不要直接删整个用户目录。

## 1. 位置总表

| 位置 | 约占用 | 内容 | 建议 |
|---|---:|---|---|
| `E:\ZJU_Documents\Course\car` | 约 5–6 GB | 课程原始资料、工程、视频、安装包、仓库 | 按 `README` 分层保留；删除临时/重复文件 |
| `C:\Users\<用户>\AppData\Local\Keil_v5` | 约 3.2 GB | Keil MDK 5.41、ARMCC、UV4 | 不需要 Keil 时用自带 `Uninstall.exe` 卸载 |
| `C:\Users\<用户>\AppData\Local\Arm\Packs` | 约 2.4 GB | Keil Pack 仓库（STM32F1/F4 等） | 只装过本课程可清理；其他 Keil 项目共用时保留 |
| `C:\Users\<用户>\AppData\Local\Temp\course_pdfs` | 几十 MB | 本次 PDF 文本提取临时文件 | 可直接删除 |
| `E:\...\car\work\tools` | 约 2 GB | GCC/ARMCC 校验工具、压缩包 | 仅本次源码校验使用，可删除 |
| `E:\...\car\work\*\OBJ` | 几十 MB | Keil 编译产物 | 可删除，重新编译生成 |
| `E:\...\car\usb_drivers`（如保留） | 几 MB | CH9102/CH343/ST-LINK 驱动 | 驱动装好后可保留压缩包作备份 |
| `Desktop\Keil uVision5.LNK` | 几 KB | Keil 快捷方式 | 卸载 Keil 时一并删除 |
| 注册表 `HKLM\SOFTWARE\WOW6432Node\Keil` | — | Keil 安装信息 | 用 Keil 卸载器处理，不要手工乱删 |
| 注册表 Uninstall 项 `Keil µVision4` | — | 卸载入口 | 对应 `Keil_v5\Uninstall.exe` |

> 不同用户名的路径以实际为准；本文用 `<用户>` 代指 `C:\Users\70336`。

## 2. Keil MDK 卸载

### 推荐方式

1. 关闭 Keil 和所有工程；
2. 运行：

```text
C:\Users\70336\AppData\Local\Keil_v5\Uninstall.exe
```

3. 或在“设置 → 应用 → 已安装的应用”里找到 `Keil μVision4`（版本 5.41）卸载；
4. 卸载后检查桌面/开始菜单快捷方式；
5. 如需彻底清理，再删除 `C:\Users\70336\AppData\Local\Keil_v5` 和 `C:\Users\70336\AppData\Local\Arm\Packs`。

### 注意

- `Arm\Packs` 可能被其他 Keil/STM32 课程共用，不要未确认就删；
- 卸载器可能保留注册表和 Pack 目录；
- 卸载前备份自己的工程和 `.uvprojx` 文件。

## 3. USB 串口/ST-LINK 驱动

CH9102/CH343 和 ST-LINK 通常以 INF 驱动安装，不一定出现在“应用和功能”里。

### 设备管理器移除

1. 插上设备；
2. 设备管理器找到 `USB-Enhanced-SERIAL CH343` 或 `STM32 STLink`；
3. 右键 → **卸载设备**；
4. 如果不再使用，勾选“尝试删除此设备的驱动程序”；
5. 拔掉设备。

### 查看/删除驱动包（高级）

以管理员身份运行：

```powershell
pnputil /enum-drivers
```

找到 `ch343ser.inf` / `stlinkwinusb.inf` 对应的 `oemXX.inf`，再执行：

```powershell
pnputil /delete-driver oemXX.inf /uninstall
```

不要删除不确定的 `oemXX.inf`；先记录原始名称。

## 4. 课程数据清理

### 建议保留

- `01_原始资料/`：课程 PDF、PPT、接线图、安装手册；
- `05_课程仓库/`：Git 仓库（含 `.git`）；
- `03_原始工程/`：原始 C10B/C50C；
- `04_实验工程/`：需要继续调试的工程。

### 可删除

- `work/tools/`：GCC/ARMCC 校验工具；
- `work/` 下的 `OBJ`、`.uvguix`、`.uvoptx`、`*.hex`、`*.bin`、`*.axf`；
- `Temp/course_pdfs`；
- 重复解压的原始工程（仓库已有）；
- 不再需要的安装包/视频（确认有备份或 FTP 可重新下载）。

### 不要删除

- 课程原始 PDF/RAR（FTP 可能受限，保留一份离线镜像）；
- Git 仓库的 `.git`；
- 个人实验报告、照片、测试数据；
- 其他课程/项目的 Keil Pack。

## 5. 磁盘占用检查

```powershell
Get-ChildItem "E:\ZJU_Documents\Course\car" -Directory |
  ForEach-Object {
    $s=(Get-ChildItem $_.FullName -Recurse -File -ErrorAction SilentlyContinue |
        Measure-Object Length -Sum).Sum
    [pscustomobject]@{Name=$_.Name; GB=[math]::Round($s/1GB,2)}
  } | Sort-Object GB -Descending
```

## 6. 卸载后验证

- Keil 快捷方式不存在、`UV4.exe` 不存在；
- 设备管理器不再出现 CH343/STLink（设备拔掉后）；
- 课程仓库仍可打开，原始资料仍可读；
- 新工程不再依赖旧 Keil Pack。
