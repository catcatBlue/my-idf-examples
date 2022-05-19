# I2C Tools Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## 概述

[I2C工具](https://i2c.wiki.kernel.org/index.php/I2C_Tools)是一个简单但非常有用的工具，用于开发与I2C相关的应用程序，它在Linux平台上也很有名。本例仅实现了[I2C工具](https://i2c.wiki.kernel.org/index.php/I2C_Tools)的一些基本功能基于[esp32控制台组件](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/console.html). 
如下所示，此示例支持五种命令行工具：

1. `i2cconfig`: 它将用特定的GPIO号、端口号和频率配置I2C总线。
2. `i2cdetect`: 它将扫描I2C总线上的设备，并输出一个带有总线上检测设备列表的表格。
3. `i2cget`: 它将读取通过I2C总线可见的寄存器。
4. `i2cset`: 它将设置通过I2C总线可见的寄存器。
5. `i2cdump`: 它将检查通过I2C总线可见的寄存器。

如果您在开发I2C相关应用程序时遇到一些困难，或者只是想测试一个I2C设备的一些功能，您可以先使用这个示例。

## 如何使用示例

### 所需硬件

要运行这个示例，您应该有任何基于ESP32、ESP32-S和ESP32-C的开发板。出于测试目的，您还应该有一种带有I2C接口的设备。这里，我们将以CCS811传感器为例，展示如何在不编写任何代码的情况下测试该传感器的功能（只需使用本示例支持的命令行工具）。有关CCS811的更多信息，请参考[在线数据表](http://ams.com/ccs811).

#### Pin分配：

**Note:** 默认情况下使用以下管脚分配，您可以随时使用`i2cconfig`命令更改它们。

|                     | SDA    | SCL    | GND  | Other | VCC  |
| ------------------- | ------ | ------ | ---- | ----- | ---- |
| ESP32 I2C Master    | GPIO18 | GPIO19 | GND  | GND   | 3.3V |
| ESP32-S2 I2C Master | GPIO18 | GPIO19 | GND  | GND   | 3.3V |
| ESP32-S3 I2C Master | GPIO1  | GPIO2  | GND  | GND   | 3.3V |
| ESP32-C3 I2C Master | GPIO5  | GPIO6  | GND  | GND   | 3.3V |
| Sensor              | SDA    | SCL    | GND  | WAK   | VCC  |

**Note:** 无需为SDA/SCL引脚添加外部上拉电阻器，因为驱动器将启用内部上拉电阻器本身。

### Configure the project

打开项目配置菜单（`idf.py menuconfig`）。然后进入“示例配置”菜单。

- 您可以在`Store command history in flash`选项中选择是否将命令历史记录保存到闪存中。

### Build and Flash

Run `idf.py -p PORT flash monitor` to build and flash the project..

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## 示例输出

### 检查所有受支持的命令及其用法

```bash
 ==============================================================
 |       Steps to Use i2c-tools on ESP32                      |
 |                                                            |
 |  1. Try 'help', check all supported commands               |
 |  2. Try 'i2cconfig' to configure your I2C bus              |
 |  3. Try 'i2cdetect' to scan devices on the bus             |
 |  4. Try 'i2cget' to get the content of specific register   |
 |  5. Try 'i2cset' to set the value of specific register     |
 |  6. Try 'i2cdump' to dump all the register (Experiment)    |
 |                                                            |
 ==============================================================

i2c-tools> help
help 
  Print the list of registered commands

i2cconfig  [--port=<0|1>] [--freq=<Hz>] --sda=<gpio> --scl=<gpio>
  配置I2C总线
  --port=<0|1>  设置I2C总线端口号
   --freq=<Hz>  设置I2C总线的频率（Hz）
  --sda=<gpio>  设置I2C SDA的gpio
  --scl=<gpio>  为I2C SCL设置gpio

i2cdetect 
  扫描I2C总线上的设备

i2cget  -c <chip_addr> [-r <register_addr>] [-l <length>]
  读取通过I2C总线可见的寄存器
  -c, --chip=<chip_addr>  指定该总线上芯片的地址
  -r, --register=<register_addr>  指定芯片上要读取的地址
  -l, --length=<length>  指定从该数据地址读取的长度

i2cset  -c <chip_addr> [-r <register_addr>] [<data>]...
  设置通过I2C总线可见的寄存器
  -c, --chip=<chip_addr>  指定该总线上芯片的地址
  -r, --register=<register_addr>  指定芯片上要读取的地址
        <data>  指定要写入该数据地址的数据

i2cdump  -c <chip_addr> [-s <size>]
  检查通过I2C总线可见的寄存器
  -c, --chip=<chip_addr>  指定该总线上芯片的地址
  -s, --size=<size>  指定每次读取的大小

free 
  获取可用堆内存的当前大小

heap 
  获取程序执行期间可用的最小可用堆内存大小

version 
  获取芯片和SDK的版本

restart 
  芯片的软件复位

deep_sleep  [-t <t>] [--io=<n>] [--io_level=<0|1>]
  进入深度睡眠模式。支持两种唤醒模式：定时器和GPIO。如果未指定唤醒选项，将无限期睡眠。
  -t, --time=<t>  Wake up time, ms
      --io=<n>  如果指定，则使用给定编号的GPIO唤醒
  --io_level=<0|1>  触发唤醒的GPIO水平

light_sleep  [-t <t>] [--io=<n>]... [--io_level=<0|1>]...
  进入轻度睡眠模式。支持两种唤醒模式：定时器和GPIO。可以使用'io' 和'io_level'参数对指定多个GPIO引脚。还将在UART输入时唤醒。
  -t, --time=<t>  起床时间, ms
      --io=<n>  如果指定，则使用给定编号的GPIO唤醒
  --io_level=<0|1>  触发唤醒的GPIO水平

tasks 
  获取有关运行任务的信息
```

### 配置I2C总线

```bash
esp32> i2cconfig --port=0 --sda=18 --scl=19 --freq=100000
```

* `--port` 选项指定I2C的端口，这里我们选择端口0进行测试。
* `--sda` and `--scl` 选项指定I2C总线使用的gpio号，这里我们选择GPIO18作为SDA，GPIO19作为SCL。
* `--freq` 选项指定I2C总线的频率，这里我们设置为100KHz。

### 检查I2C总线上的I2C地址（7位）

```bash
esp32> i2cdetect
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- 5b -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
```

* 这里我们发现CCS811的地址是0x5b。

### 获取状态寄存器的值

```bash
esp32> i2cget -c 0x5b -r 0x00 -l 1
0x10 
```

* `-c` 选项指定I2C设备的地址（从`i2cdetect`命令获取）。
* `-r` 选项指定要检查的寄存器地址。
* `-l` 选项指定内容的长度。
* 此处返回值0x10表示传感器刚刚处于引导模式，准备进入应用模式。有关CCS811的更多信息，请访问[官方网站](http://ams.com/ccs811).

### 改变工作模式

```bash
esp32> i2cset -c 0x5b -r 0xF4
I (734717) cmd_i2ctools: Write OK
esp32> i2cset -c 0x5b -r 0x01 0x10
I (1072047) cmd_i2ctools: Write OK
esp32> i2cget -c 0x5b -r 0x00 -l 1
0x98 
```

* 在这里，我们将模式从引导更改为应用程序，并设置适当的测量模式（通过将0x10写入寄存器0x01）
* 现在传感器的状态值为0x98，这意味着可以读取有效数据

### 读取传感器数据

```bash
esp32> i2cget -c 0x5b -r 0x02 -l 8
0x01 0xb0 0x00 0x04 0x98 0x00 0x19 0x8f 
```

* 寄存器0x02将输出8字节的结果，主要包括eCO~2~、TVOC值和原始值。所以eCO~2的值是0x01b0 ppm，TVOC的值是0x04 ppb。

## 故障排除

* 运行`i2cdetect`命令时，我找不到任何可用地址。
  * 确保你的接线正确。
  * 一些传感器会有一个“唤醒”引脚，用户可以通过该引脚将传感器置于睡眠模式。因此，确保传感器处于非睡眠状态。
  * 重置I2C设备，然后再次运行“i2cdetect”。
* 运行`i2cdump`命令时无法获取正确的内容。
  * 目前，`i2cdump`只支持那些在I2C设备内具有相同寄存器内容长度的人。例如，如果一个设备有三个寄存器地址，这些地址的内容长度分别为1字节、2字节和4字节。在这种情况下，您不应该期望该命令正确转储寄存器。


（对于任何技术问题，请打开[问题](https://github.com/espressif/esp-idf/issues)在GitHub上。我们会尽快回复您。）

