# 贡献与提交规范

## 分支

- `main`：原始基线、课程文档、已验证内容；
- `course/<年份>-<阶段>`：课程实验分支，例如 `course/2026-midterm`；
- `feature/<主题>`：单个功能或修复。

## 提交前检查

1. 没有提交 `OBJ/`、`*.hex`、`*.bin`、`*.axf`、`*.uvguix.*`、`*.uvoptx`；
2. 原始工程未被覆盖；
3. 实验代码在 README/实验说明中标注验证状态；
4. 修改协议时两端结构体/长度/BCC 同步；
5. 更新对应 `docs/` 页面和测试记录；
6. 不把本机绝对路径写进通用文档。

## 提交信息

使用简短英文动词开头：

```text
lab1: add 16x16 distance glyphs
lab2: integrate servo sector scan
lab3: extend C10B frame with attitude
midterm: add 3 m round-trip state machine
docs: update C10B/C50C wiring
fix: correct flash algorithm notes
```

## 测试记录

新增/修改固件时，请附上：

- 板卡与硬件版本；
- Keil Target；
- 编译结果（0 Error / 警告数）；
- 实车现象与测量数据；
- 未通过项和下一步。
