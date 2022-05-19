# 目录

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [目录](#目录)
- [概述](#概述)
  - [命令](#命令)
    - [cmd_system](#cmd_system)
    - [cmd_wifi](#cmd_wifi)
    - [cmd_nvs](#cmd_nvs)
  - [Example Breakdown](#example-breakdown)
    - [配置 UART](#配置-uart)
    - [行编辑](#行编辑)
    - [命令](#命令-1)
    - [命令处理](#命令处理)
    - [参数解析](#参数解析)
    - [命令行历史](#命令行历史)

<!-- /code_chunk_output -->

# 概述

CatB_console_library 是对 console_example_main.c 的二次封装，添加命令需要进入 CatB_console_library.c 修改.

## 命令

### cmd_system

**free**
```
获取可用堆内存的当前大小
```

**heap**
```
获取程序执行期间可用的最小可用堆内存大小
```

**version**
```
获取芯片和 SDK 的版本
```

**restart**
```
软件复位芯片
```

**tasks**
```
获取有关运行任务的信息
```

**deep_sleep [-t <t>] [--io=<n>] [--io_level=<0|1>]**
```
进入深度睡眠模式。支持两种唤醒模式：定时器和 GPIO。如果未指定唤醒选项，将无限期睡眠。
-t, --time=<t> 起床时间, ms
--io=<n> 如果指定，则使用给定编号的 GPIO 唤醒
--io_level=<0|1> 触发唤醒的 GPIO 级别
```

**light_sleep [-t <t>] [--io=<n>]... [--io_level=<0|1>]...**
```
进入轻度睡眠模式。支持两种唤醒模式：定时器和 GPIO。可以使用“io”和“io\_级别”参数对指定多个 GPIO 引脚。还将在 UART 输入时唤醒。
-t, --time=<t> 起床时间, ms
--io=<n> 如果指定，则使用给定编号的 GPIO 唤醒
--io_level=<0|1> 触发唤醒的 GPIO 级别
```

### cmd_wifi

**join [--timeout=<t>] <ssid> [<pass>]**
```
作为一个站点加入 WiFi AP
--timeout=<t> 连接超时, ms
<ssid> SSID of AP
<pass> PSK of AP
```

### cmd_nvs

**nvs_set <key> <type> -v <value>**
```
在所选命名空间中设置键值对。
Examples:
nvs_set VarName i32 -v
123
nvs_set VarName str -v YourString
nvs_set VarName blob -v 0123456789
abcdef
<key> 要设置的值的键
<type> 类型可以是: i8, u8, i16, u16 i32, u32 i64, u64, str, blob
-v, --value=<value> 要存储的值
```

**nvs_get <key> <type>**
```
从所选命名空间获取键值对。
Example: nvs_get VarName i32
<key> 要读取的值的键
<type> 类型可以是: i8, u8, i16, u16 i32, u32 i64, u64, str, blob
```

**nvs_erase <key>**
```
从当前命名空间中删除键值对
<key> key of the value to be erased
```

**nvs_namespace <namespace>**
```
设置当前 namespace
<namespace> 要选择的分区的 namespace
```

**nvs_list <partition> [-n <namespace>] [-t <type>]**
```
列出存储在 NVS 中的存储键值对。命名空间和类型可以指定为仅打印那些键值对。
下面的命令列出了存储在“nvs”分区内、名称空间“storage”下、类型为 uint32_t 的变量
Example: nvs_list nvs -n storage -t u32

<partition> partition name
-n, --namespace=<namespace> namespace name
-t, --type=<type> type can be: i8, u8, i16, u16 i32, u32 i64, u64, str, blob
```

**nvs_erase_namespace <namespace>**
```
删除指定的命名空间
<namespace> 要删除的命名空间
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

使用`esp_console_cmd_register()`函数注册了几个命令。请参阅`cmd_wifi.c`和`cmd_system.c`中的`register_wifi()`和`register_system()`函数。

### 命令处理

`app_main()`函数中的 main 循环说明了如何使用`linenoise`和`esp_console_run()`实现读取/评估（read/eval）循环。

### 参数解析

在`cmd_wifi.c`和`cmd_system.c`中使用 Argtable3 库解析和检查参数实现了几个命令。

### 命令行历史

每次从`linenoise`获取新命令行时，它都会写入历史记录，并将历史记录保存到闪存中的文件中。重置时，历史从该文件初始化。
