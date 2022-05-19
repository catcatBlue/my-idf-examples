# 目录

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [目录](#目录)
- [概述](#概述)
  - [命令](#命令)
    - [CatB_console_library](#catb_console_library)
    - [cmd_system](#cmd_system)
    - [cmd_wifi](#cmd_wifi)
    - [cmd_nvs](#cmd_nvs)
  - [cmd_wifi 的简易解析](#cmd_wifi-的简易解析)
    - [调用过程](#调用过程)
    - [编程过程](#编程过程)
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

### CatB_console_library
**exit**
```
退出控制台，没啥用，退出了就回不来了
```
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

## cmd_wifi 的简易解析

cmd_wifi 中主要完成连接 wifi 的功能，`cmd_wifi.h` 声明需要注册的组件接口，`cmd_wifi.c` 存放相关函数，其中大部分都是内部的。

### 调用过程

从调用过程的角度来说，首先调用`void register_wifi(void)`注册 wifi 指令：

1. **指派参数类型**，用以返回解析后对应的参数：将预先定义的参数表结构体通过`argtable3.h`中的`arg_int0()`和`arg_end()`等函数返回对应的参数。
用法可以参考[如何解析c语言程序的命令行参数？—— Argtable3使用教程](https://songyangji.gitee.io/2021/09/23/%E5%A6%82%E4%BD%95%E8%A7%A3%E6%9E%90c%E8%AF%AD%E8%A8%80%E7%A8%8B%E5%BA%8F%E7%9A%84%E5%91%BD%E4%BB%A4%E8%A1%8C%E5%8F%82%E6%95%B0%EF%BC%9F%E2%80%94%E2%80%94-Argtable3%E4%BD%BF%E7%94%A8%E6%95%99%E7%A8%8B/)。
值得一提的是，类似于`arg_int0()`这种后缀为`0`或`1`的函数（如`arg_str1()`）表示，对应的参数可以有一个输入，也可以没有输入；后缀`1`表示，必须要有一个输入。
`arg_end()`有两个用处，一是指明参数表尾，二是用于指向返回的错误信息，里面的整数参数说明最多能保存几个错误，过多的错误信息会被丢弃并替换为单个错误消息“错误太多”。
```c
join_args.timeout  = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
join_args.ssid     = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
join_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
join_args.end      = arg_end(2);
```

2. **注册esp控制台**，使用`esp_console.h`中的`esp_console_cmd_t`类型声明一个包含命令名称、帮助信息、参数提示文本、命令执行函数和参数表的结构体，然后调用`esp_console_cmd_register()`注册命令。
```c
const esp_console_cmd_t join_cmd = {
    .command  = "join",
    .help     = "Join WiFi AP as a station",
    .hint     = NULL,
    .func     = &connect,
    .argtable = &join_args};
ESP_ERROR_CHECK(esp_console_cmd_register(&join_cmd));
```

当控制台输入`join`命令后，调用`esp_console_cmd_register()`注册的命令执行函数`connect()`：
1. `static int connect(int argc, char **argv)`：占位

2. **解析命令参数**，调用`arg_parse()`解析参数并返回错误个数，有错误时`arg_print_errors()`输出错误并返回，没有则可以直接调用其中的参数加入wifi，`wifi_join()`是个通用的函数。

`arg_parse(argc, argv, (void **)&join_args)`

### 编程过程

1. **定义参数表**：`join_args`。
1. **注册组件**：`register_wifi()`。
1. **解析参数**：`connect()`。


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
