| Supported Targets | ESP32 |
| ----------------- | ----- |

# 外部Flash示例中的FAT FS

(See the README.md file in the upper level 'examples' directory for more information about examples.)

该示例与[wear Leveling](../wear_levelling/README.md)示例类似，只是它使用了一个外部SPI闪存芯片。如果您需要为只有4 MB闪存大小的模块添加更多存储空间，这将非常有用。

示例的流程如下所示：

1. 初始化SPI总线并配置引脚。在本例中，使用VSPI外围设备。本例中选择的引脚对应于VSPI外围设备的IOMUX引脚。如果管脚分配发生变化，SPI驱动程序将使用GPIO矩阵将外围设备连接到管脚。

2. 初始化SPI闪存芯片。这涉及到创建一个描述闪存芯片的运行时对象（`esp_flash_t`），探测闪存芯片，并将其配置为所选的读取模式。默认情况下，本例使用DIO模式，只需要4个管脚（MOSI、MISO、SCLK、CS），但我们强烈建议连接（或上拉）WP和HD管脚。对于QIO和QOUT等模式，必须连接其他引脚（WP/DQ2、HD/DQ3）。

3. 将闪存芯片的整个区域注册为*分区*（`esp_partition_t`）。这允许其他组件（FATF、SPIFF、NVS等）使用外部闪存芯片提供的存储器。

4. 使用C标准库函数执行一些读写操作：创建文件、写入文件、打开文件进行读取、将内容打印到控制台。

## 如何使用示例

### 所需硬件

本例需要一个SPI NOR闪存芯片连接到ESP32。SPI闪存芯片必须具有3.3V逻辑电平。该示例已使用Winbond W25Q32 SPI闪存芯片进行了测试。

使用以下管脚分配：

ESP32 pin     | SPI bus signal | SPI Flash pin 
--------------|----------------|----------------
GPIO23        | MOSI           | DI
GPIO19        | MISO           | DO
GPIO18        | SCLK           | CLK
GPIO5         | CS             | CMD
GPIO22        | WP             | WP
GPIO21        | HD             | HOLD
GND           |                | GND
VCC           |                | VCC

### Build and flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with serial port name.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example output

Here is a typical example console output. 

```
I (328) example: Initializing external SPI Flash
I (338) example: Pin assignments:
I (338) example: MOSI: 23   MISO: 19   SCLK: 18   CS:  5
I (348) spi_flash: detected chip: generic
I (348) spi_flash: flash io: dio
I (348) example: Initialized external Flash, size=4096 KB, ID=0xef4016
I (358) example: Adding external Flash as a partition, label="storage", size=4096 KB
I (368) example: Mounting FAT filesystem
I (378) example: FAT FS: 4024 kB total, 4020 kB free
I (378) example: Opening file
I (958) example: File written
I (958) example: Reading file
I (958) example: Read from file: 'Written using ESP-IDF v4.0-dev-1301-g0a1160468'
```
