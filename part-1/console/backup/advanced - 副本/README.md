# Console Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

此示例说明了[Console 组件](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/console.html#console)在 ESP32 上创建交互式 shell 的用法。然后，可以通过串口（UART）控制/交互 ESP32 上运行的交互式 shell。

本例中实现的交互式 shell 包含多种命令，可以作为需要命令行界面（CLI）的应用程序的基础。

## How to use example

### Hardware Required

这个例子应该能够在任何常用的 ESP32 开发板上运行。

### Configure the project

```
idf.py menuconfig
```

- 根据需要启用/禁用`Example Configuration > Store command history in flash`

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type `Ctrl-]`.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

输入`help`命令获取所有可用命令的完整列表。以下是控制台示例的示例会话，其中使用了控制台示例提供的各种命令。请注意，GPIO15 连接到 GND 以删除引导日志输出。

```
This is an example of ESP-IDF console component.
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
[esp32]> help
help
  Print the list of registered commands

free
  Get the total size of heap memory available

restart
  Restart the program

deep_sleep  [-t <t>] [--io=<n>] [--io_level=<0|1>]
  Enter deep sleep mode. Two wakeup modes are supported: timer and GPIO. If no
  wakeup option is specified, will sleep indefinitely.
  -t, --time=<t>  Wake up time, ms
      --io=<n>  If specified, wakeup using GPIO with given number
  --io_level=<0|1>  GPIO level to trigger wakeup

join  [--timeout=<t>] <ssid> [<pass>]
  Join WiFi AP as a station
  --timeout=<t>  Connection timeout, ms
        <ssid>  SSID of AP
        <pass>  PSK of AP

[esp32]> free
257200
[esp32]> deep_sleep -t 1000
I (146929) deep_sleep: Enabling timer wakeup, timeout=1000000us
I (619) heap_init: Initializing. RAM available for dynamic allocation:
I (620) heap_init: At 3FFAE2A0 len 00001D60 (7 KiB): DRAM
I (626) heap_init: At 3FFB7EA0 len 00028160 (160 KiB): DRAM
I (645) heap_init: At 3FFE0440 len 00003BC0 (14 KiB): D/IRAM
I (664) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (684) heap_init: At 40093EA8 len 0000C158 (48 KiB): IRAM

This is an example of ESP-IDF console component.
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
[esp32]> join --timeout 10000 test_ap test_password
I (182639) connect: Connecting to 'test_ap'
I (184619) connect: Connected
[esp32]> free
212328
[esp32]> restart
I (205639) restart: Restarting
I (616) heap_init: Initializing. RAM available for dynamic allocation:
I (617) heap_init: At 3FFAE2A0 len 00001D60 (7 KiB): DRAM
I (623) heap_init: At 3FFB7EA0 len 00028160 (160 KiB): DRAM
I (642) heap_init: At 3FFE0440 len 00003BC0 (14 KiB): D/IRAM
I (661) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (681) heap_init: At 40093EA8 len 0000C158 (48 KiB): IRAM

This is an example of ESP-IDF console component.
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
[esp32]>

```

## Troubleshooting

### 行尾

控制台示例中的线路端点配置为与特定的串行监视器匹配。因此，如果出现以下日志输出，请考虑使用不同的串行监视器（例如 Windows 的 Putty）或修改示例 [UART configuration](#Configuring-UART-and-VFS).

```
This is an example of ESP-IDF console component.
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
Your terminal application does not support escape sequences.
Line editing and history features are disabled.
On Windows, try using Putty instead.
esp32>
```

## Example Breakdown

### 配置 UART

本例中的`initialize_console()`函数配置与控制台操作相关的 UART 的某些方面。

- **Line Endings**: 默认的行尾配置为与常见串行监控程序（如`screen`, `minicom`, 和 `idf_monitor.py`）预期/生成的行尾匹配。py`包含在 SDK 中。这些命令的默认行为是:
  - 当按下键盘上的“回车”键时，`CR` (0x13) 代码被发送到串行设备。
  - 要将光标移动到下一行的开头，串行设备需要发送 `CR LF` (0x13 0x10) 序列。

### 行编辑

该示例的主要源文件演示了如何使用“linenoise”库，包括行完成、提示和历史记录。

### 命令

使用`esp_console_cmd_register()`函数注册了几个命令。请参阅`cmd_wifi.c`和`cmd_system.c`中的`register_wifi()`和`register_system()`函数。c`和'cmd\_系统。

### 命令处理

`app_main()`函数中的 main 循环说明了如何使用`linenoise`和`esp_console_run（）`实现读取/评估（read/eval）循环。

### 参数解析

在`cmd_wifi.c`和`cmd_system.c`中使用 Argtable3 库解析和检查参数实现了几个命令。

### 命令行历史

每次从`linenoise`获取新命令行时，它都会写入历史记录，并将历史记录保存到闪存中的文件中。重置时，历史从该文件初始化。
