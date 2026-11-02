# 实验实现入口（未实车验收）

`main` 分支只保留原始基线。完整的实验代码放在：

```text
course/2026-midterm
```

该分支的 `firmware/experimental/` 包含：

- `c10b-course/`：16×16 汉字、舵机扫描、TIM7 100 Hz 时基、23 字节帧；
- `c50c-midterm/`：23 字节解析、3 m 往返状态机、姿态显示、通信计数页。

状态：**源码语法检查通过，实车验收未完成**。已知问题：

1. C10B → C50C 通信未闭环；
2. MPU6050 Pitch/Roll/Yaw 现场恒为 0；
3. 3 m 直线/掉头/返回未实测。

使用方式：

```powershell
git switch course/2026-midterm
```

先跑通 `firmware/original/`，再对照实验分支；验证通过后才合并到 `main`。
