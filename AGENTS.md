# 仓库维护规则

本仓库面向浙江大学 WHEELTEC C10B/C50C 小车课程。维护目标是保留可复现的原始基线、提供准确的课程实验步骤，并让已验证版本可以追溯。

## 目录职责

| 路径 | 规则 |
|---|---|
| `firmware/original/` | 厂商课程基线，只读。不得覆盖、格式化或重排 |
| `firmware/experimental/` | 课程扩展工程，只能在完成测试后标记为已验证 |
| `docs/labs/` | 实验步骤与验收标准 |
| `docs/reference/` | 原始课件、手册、图片和工具说明 |
| `references/` | 小型参考源码 |
| `scripts/` | 仓库检查和构建辅助脚本 |

## 文件与内容

1. 仓库中的目录名和文件名使用 ASCII 英文。
2. 文档正文使用中文，技术标识符保留英文。
3. 不引用个人电脑绝对路径；使用仓库相对路径。
4. 不新增与课程使用无关的总览、感想或重复说明。
5. 新增内容优先更新现有文档，不另建相似页面。
6. 混合编码源码保持原字节，禁止整体转码或格式化。

## 版本管理

1. `main` 只接受通过构建和适用测试的提交。
2. 功能分支使用英文名，例如 `feature/ultrasonic-scan`、`fix/usart2-rx`。
3. 提交信息使用中文，前缀为 `新增`、`修复`、`文档`、`维护`、`重构` 或 `测试`。
4. 一次提交只处理一个主题，不混入无关文档或格式变化。
5. 稳定版本使用标签，例如 `midterm-2026`。
6. 已验证标签不得被覆盖；修复问题应产生新提交和新标签。

提交示例：

```text
修复：启用C50C串口2初始化
新增：完成单超声波五扇区扫描
文档：统一仓库英文路径与索引
测试：完成三米往返验收
```

## 构建要求

修改固件后至少执行：

1. C10B Target `C10B`：`Rebuild`，要求 `0 Error(s)`；
2. C50C Target `Mec_Car`：`Rebuild`，要求 `0 Error(s)`；
3. 检查生成的 `.hex` 时间和 Target 是否对应；
4. 涉及实车行为时，重新执行相关硬件测试。

构建产物不得提交：

```text
OBJ/
*.hex
*.bin
*.axf
*.uvguix.*
*.uvoptx
*.log
```

## 协议要求

修改 C10B/C50C 通信时，必须同步检查：

- 两端结构体字段顺序与类型；
- `#pragma pack(1)`；
- 帧长度与 BCC 范围；
- 115200-8N1 配置；
- `docs/protocol.md`；
- C50C 第 4 页 `RX / HEAD / OK / BAD / LEN`。

## 验收要求

声称“已验证”前必须记录：

- 日期、板卡和硬件型号；
- Keil Target 与烧录方式；
- 编译结果；
- OLED、串口和实车现象；
- 失败项或适用边界。

2026 年中期稳定版本参数：

```text
MID_SPEED=0.20f
LineDiffParam=52
MID_TURN_MAX=0.50f
MID_STOP_MM=500
MID_CLEAR_MM=550
```

## 安全检查

- 首次运行保留至少 3.5 m 直线空间；
- 障碍测试使用软质纸箱；
- 舵机使用独立 5 V 供电并共地；
- 24 V 电池接通前不得做整车电机测试；
- 通信中断必须触发停车；
- FTP 密码、令牌和个人数据不得写入仓库。

## 提交前检查

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/check-repository.ps1
git diff --check
git status --short
```

不满足检查时不要合并到 `main`。
