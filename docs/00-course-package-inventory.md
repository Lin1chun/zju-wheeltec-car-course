# 课程原始资料清单与仓库取舍

本文记录 `E:\ZJU_Documents\Course\car` 原始课程包的目录构成，以及哪些内容进入了 Git 仓库、哪些保留在本地课程包。

## 1. 原始顶层目录

| 名称 | 类型 | 约大小 | 处理 |
|---|---|---:|---|
| `01 麦轮小车电气连接图20260918更新.jpg` | 图片 | 2.7 MB | 收入 `docs/reference/hardware/` |
| `03 麦轮小车机械安装与下载程序手册20260918.pdf` | 手册 | 4.3 MB | 收入 `docs/reference/hardware/` |
| `04 安装指南及常见问题汇总20260918.docx` | 文档 | 1.1 MB | 收入 `docs/reference/hardware/` |
| `06 多模智控手柄用户手册.pdf` | 手册 | 0.7 MB | 收入 `docs/reference/hardware/` |
| `麦轮小车安装调试讲解视频20260918.mp4` | 视频 | 约 668 MB | 不收入；体积过大 |
| `05 组装完成后测试程序及下载软件_20260916/` | 测试程序/工具 | 约 17 MB | 只收入烧录教程；软件/hex 不收入 |
| `传感器大课课件/` | 课件 | 约 6.7 MB | 收入 `docs/reference/sensors/` |
| `传感器及串口通讯参考程序/` | 参考工程 RAR | 约 224 MB | 解压后的原始工程收入 `firmware/original/`；RAR 不收入 |
| `电机控制实训/` | 安装包/C50C RAR | 约 2.4 GB | 只收入 C50C 原始工程；Keil/视频/RAR 不收入 |
| `蓝牙APP及使用说明/` | APP 资料 | 约 110 MB | 不收入；课程任务不依赖 |
| `串口助手/` | 调试工具 | 约 0.4 MB | 不收入；使用 SSCOM |
| `work/` | 本地解压/工作目录 | 约 2.5 GB | 不作为课程包内容上传；仅提取源工程 |

## 2. 进入仓库的内容

### 原始工程

- C10B：`firmware/original/c10b/`
- C50C：`firmware/original/c50c/`

### 实验工程（分支）

- 分支：`course/2026-midterm`
- C10B 扩展：`firmware/experimental/c10b-course/`
- C50C 中期：`firmware/experimental/c50c-midterm/`

### 课程资料

- 传感器讲义/PPT：`docs/reference/sensors/`
- 硬件/机械/电气手册：`docs/reference/hardware/`
- 烧录/ST-LINK 教程：`docs/reference/toolchain/`

### 参考代码

- 舵机 PA8/TIM1 例程：`references/servo-example/`

## 3. 未进入仓库的内容及原因

| 内容 | 原因 | 获取方式 |
|---|---|---|
| Keil MDK 安装包 | 约 1.8 GB，且为第三方安装程序 | 课程原始资料包 |
| 整车安装视频 | 约 668 MB | 课程原始资料包 |
| 蓝牙 APP 资料 | 约 110 MB，非本课程主线 | 课程原始资料包 |
| 原始 RAR 工程包 | 与解压后的原始工程重复 | 课程原始资料包 |
| 预编译 hex/bin | 属于特定构建产物 | 本地 Keil 重新生成 |
| `OBJ/`、`.uvguix`、`.uvoptx` | 用户/构建临时文件 | Keil 自动生成 |

## 4. 使用建议

1. 先以 `firmware/original/` 和 `docs/reference/` 为准；
2. 需要安装包/视频/APP 时，从课程原始资料包获取；
3. 不要把安装包、视频或构建产物提交到 Git；
4. 固件二进制通过 GitHub Releases 发布。

## 5. 版权

原始工程与课程资料的版权归 WHEELTEC / 浙江大学课程组所有；本仓库保持私有，仅供课程学习与内部维护。详见 `NOTICE.md`。
