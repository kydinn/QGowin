# QGowin
[English](https://github.com/kydinn/QGowin/blob/main/README.md) | 简体中文



基于 Qt + Gowin_Programmer_cli 实现的高云 FPGA 烧写上位机



## 快速开始

### Windows

下载预编译的压缩包后双击执行`QGowin.exe`，自带支持了高云的`GW1N-4D`芯片。



## 二次开发

### 开发环境
- Qt6.9
- MSVC 2019
- Windows 11  24H2
- Gowin_Programmer Version 1.9.9 (64-bit) build(31129)

如果需要适配新的芯片，只需要在`fpga_update_page.h`中追加对应的芯片名称即可。

```cpp
static QMap<QString, QString> device_map = {
	{"GW1N-4D", "GW1N-4D"},
    // 添加新的芯片
};
```

