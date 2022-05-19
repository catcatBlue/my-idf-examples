# NMEA Parser Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## Overview

This example will show how to parse NMEA-0183 data streams output from GPS/BDS/GLONASS modules based on ESP UART Event driver and ESP event loop library.
For the convenience of the presentation, this example will only parse the following basic statements:

- GGA
- GSA
- GSV
- RMC
- GLL
- VTG

See [Limitation for multiple navigation system](#Limitation) for more information about this example.

Usually, modules will also output some vendor specific statements which common nmea library can not cover. In this example, the NMEA Parser will propagate all unknown statements to the user, where a custom handler can parse information from it.

## How to use example

### Hardware Required

要运行此示例，您需要一个 ESP32、ESP32-S 或 ESP32-C 系列开发板（例如 ESP32-WROVER 工具包）。出于测试目的，您还需要一个 GPS 模块。这里我们使用[ATGM332D-5N](http://www.icofchina.com/pro/mokuai/2016-08-01/5.html)作为示例，演示如何解析 NMEA 语句并输出常见信息，如 UTC 时间、纬度、经度、海拔、速度等。

#### Pin Assignment:

**Note:** The following pin assignments are used by default which can be changed in `nmea_parser_config_t` structure.

| ESP                        | GPS    |
| -------------------------- | ------ |
| UART-RX (GPIO5 by default) | GPS-TX |
| GND                        | GND    |
| 5V                         | VCC    |

**Note:** 如果仅使用 UART 接收数据，则不需要 UART TX 引脚。

### Configure the project

Open the project configuration menu (`idf.py menuconfig`). Then go into `Example Configuration` menu.

- Set the size of ring buffer used by uart driver in `NMEA Parser Ring Buffer Size` option.
- Set the stack size of the NMEA Parser task in `NMEA Parser Task Stack Size` option.
- Set the priority of the NMEA Parser task in `NMEA Parser Task Priority` option.
- In the `NMEA Statement support` submenu, you can choose the type of statements that you want to parse. **Note:** you should choose at least one statement to parse.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build and flash the project..

(To exit the serial monitor, type `Ctrl-]`.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```bash
I (0) cpu_start: Starting scheduler on APP CPU.
I (317) uart: queue free spaces: 16
I (317) nmea_parser: NMEA Parser init OK
I (1067) gps_demo: 2018/12/4 13:59:34 =>
						latitude   = 31.20177°N
						longitude = 121.57933°E
						altitude   = 17.30m
						speed      = 0.370400m/s
W (1177) gps_demo: Unknown statement:$GPTXT,01,01,01,ANTENNA OK*35
I (2067) gps_demo: 2018/12/4 13:59:35 =>
						latitude   = 31.20177°N
						longitude  = 121.57933°E
						altitude   = 17.30m
						speed      = 0.000000m/s
W (2177) gps_demo: Unknown statement:$GPTXT,01,01,01,ANTENNA OK*35
I (3067) gps_demo: 2018/12/4 13:59:36 =>
						latitude   = 31.20178°N
						longitude  = 121.57933°E
						altitude   = 17.30m
						speed      = 0.000000m/s
W (3177) gps_demo: Unknown statement:$GPTXT,01,01,01,ANTENNA OK*35
I (4067) gps_demo: 2018/12/4 13:59:37 =>
						latitude   = 31.20178°N
						longitude  = 121.57933°E
						altitude   = 17.30m
						speed      = 0.000000m/s
W (4177) gps_demo: Unknown statement:$GPTXT,01,01,01,ANTENNA OK*35
I (5067) gps_demo: 2018/12/4 13:59:38 =>
						latitude   = 31.20178°N
						longitude  = 121.57933°E
						altitude   = 17.30m
						speed      = 0.685240m/s
W (5177) gps_demo: Unknown statement:$GPTXT,01,01,01,ANTENNA OK*35
```

如上所示，ESP 板在解析了 NMEA0183 格式的语句后，最终获得了信息。但由于我们没有在库中添加`GPTXT`类型语句（这意味着 NMEA 解析器库不知道它），所以它被传播给用户，而无需任何处理。

## Troubleshooting

1. 虽然我已经检查了所有的 pin 连接，但我无法从 GPS 收到任何声明。
   - 通过其他终端（例如 minicom、putty）测试您的 GPS，以检查正确的通信参数（例如 GPS 支持的波特率）。

## Limitation

如果 GPS 模块支持多个卫星导航系统（例如 GPS、BDS），则卫星 ID 和说明可能会以不同的声明（例如 GPGSV、BDGSV、GPGSA、BDGSA）提供，具体取决于 GPS 模块使用的 NMEA 协议版本。该示例目前只能记录来自一个导航系统的卫星 id 和描述。
然而，对于其他语句，无论导航系统是什么，本例都可以正确解析它们。

### Steps to skip the limitation

1. Uncheck the `GSA` and `GSV` statements in menuconfig.
2. In the `gps_event_handler` will get a signal called `GPS_UNKNOWN`, and the unknown statement itself (It's a deep copy of the original statement).
3. Manually parse the unknown statements and get the satellites' descriptions.

4. 取消选中 menuconfig 中的`GSA` 和 `GSV`语句。
5. 在`gps_event_handler`中，处理器将收到一个名为`GPS_UNKNOWN`的信号，以及未知语句本身（它是原始语句的深度副本）。
6. 手动解析未知语句并获取卫星的描述。

(For any technical queries, please open an [issue](https://github.com/espressif/esp-idf/issues) on GitHub. We will get back to you as soon as possible.)
