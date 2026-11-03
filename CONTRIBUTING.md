# 贡献规范

## 分支

| 分支 | 用途 |
|---|---|
| `main` | 已验证版本 |
| `feature/<topic>` | 新功能 |
| `fix/<topic>` | 问题修复 |
| `chore/<topic>` | 仓库结构与维护 |

分支名使用英文小写和连字符。

## 提交信息

提交标题使用中文，保持单一主题。

```text
新增：实现舵机五扇区扫描
修复：初始化C50C串口2
文档：更新中期验收参数
维护：统一仓库目录与索引
重构：拆出底盘状态机
测试：完成三米往返验收
```

禁止使用 `update`、`fix bug`、`changes`、`final` 等无信息标题。

## 提交前检查

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/check-repository.ps1
git diff --check
git status --short
```

固件提交还需附上：

- C10B / C50C Target；
- Rebuild 的 Error/Warning 数；
- 受影响功能的实车结果；
- 未验证范围。

## 文件要求

- 路径和文件名使用英文；
- 不提交 `OBJ/`、`.hex`、`.bin`、`.axf`、`.uvoptx`、`.uvguix.*`；
- 不修改 `firmware/original/`；
- 不提交密码、令牌、个人数据或课程安装包；
- 大文件通过 GitHub Releases 发布。

## 合并要求

1. 原始工程保持不变；
2. 协议两端同步；
3. 底盘 Target、型号和轮径参数一致；
4. 文档与固件参数一致；
5. 提交历史中没有临时文件或失败构建产物。
