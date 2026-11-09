# 辅助脚本

`build-keil.ps1` 用于在已安装 Keil 的 Windows 机器上批量构建工程。

`test_obstacle_safety.c` 验证原始距离即时触发、500/550 mm 滞回、缺回波失效保护、快触发慢恢复滤波以及中心优先扫描序列。需要支持 C11 的主机 C 编译器，例如：

```powershell
zig cc -std=c11 -Wall -Wextra -Werror `
  -I ../firmware/experimental/c10b-course/WHEELTEC_APP/Inc `
  test_obstacle_safety.c -o test_obstacle_safety.exe
.\test_obstacle_safety.exe
```

示例：

```powershell
$env:UV4 = "C:\Keil_v5\UV4\UV4.exe"
.\build-keil.ps1 -Project ..\firmware\original\c10b\MDK-ARM\C10B.uvprojx -Target C10B
.\build-keil.ps1 -Project ..\firmware\original\c50c\USER\WHEELTEC.uvprojx -Target Mec_Car
```

脚本只调用 Keil 批处理构建，不负责烧录。构建日志写在工程目录的 `build-<target>.log`，该文件已被 `.gitignore` 忽略。
