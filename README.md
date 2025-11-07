# QGowin
English | [简体中文](https://github.com/kydinn/QGowin/blob/main/README_zh_CN.md)



Gowin FPGA factoring hosts based on Qt + Gowin_Programmer_cli



## Quick Start

### Windows

After downloading the pre-compiled compressed package, double-click to run `QGowin.exe`, which comes with built-in support for Gowin's `GW1N-4D` chip.



## Secondary Development

### Development Environment

- Qt6.9
- MSVC 2019
- Windows 11 24H2
- Gowin_Programmer Version 1.9.9 (64-bit) build(31129)

If you need to adapt to a new chip, simply append the corresponding chip name to `fpga_update_page.h`.

```cpp
static QMap<QString, QString> device_map = {
	{"GW1N-4D", "GW1N-4D"},
    // Add a new chip
};
```
