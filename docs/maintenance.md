# 维护与发布

## 分支

| 分支 | 用途 |
|---|---|
| `main` | 已验证版本和课程文档 |
| `feature/<topic>` | 新功能 |
| `fix/<topic>` | 问题修复 |
| `chore/<topic>` | 仓库结构和工具链维护 |

## 提交

提交信息使用中文主题前缀。一次提交只处理一个主题，提交前运行：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/check-repository.ps1
git diff --check
git status --short
```

## 固件发布

1. 分别 Rebuild C10B Target `C10B` 和 C50C Target `Mec_Car`；
2. 记录 Error/Warning 数和固件输出时间；
3. 完成受影响功能的实车测试；
4. 合并到 `main`；
5. 创建版本标签，例如 `midterm-2026`；
6. 在 GitHub Releases 上传 `.hex`，不要在 Git 中提交二进制。

Release 说明至少包含板卡、Target、烧录方式、已验证功能和已知限制。

## 下一届课程

1. 从 `main` 创建新的英文功能分支；
2. 保持 `firmware/original/` 不变；
3. 在 `firmware/experimental/` 扩展课程工程；
4. 更新已有的实验文档，不重复创建同类页面；
5. 完成测试后合并并创建新标签。

## 文档一致性

修改以下内容时必须同步更新对应文档：

| 修改 | 同步位置 |
|---|---|
| 串口帧字段、长度或 BCC | `docs/protocol.md` |
| 接线、引脚或车型 | `docs/hardware.md` |
| 中期距离、速度或阈值 | `docs/midterm.md`、`AGENTS.md` |
| 烧录方式或工具版本 | `docs/toolchain.md` |
