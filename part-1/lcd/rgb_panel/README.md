| Supported Targets | ESP32-S3 |
| ----------------- | -------- |
# RGB panel example

[esp_lcd](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/lcd.html)支持RGB接口的LCD面板，带有由驱动程序自己管理的帧缓冲区。

本例显示了安装RGB面板驱动程序的一般过程，并基于LVGL库在屏幕上显示散点图。有关移植LVGL库的更多信息，请参阅[另一个LVGL移植示例](../i80_controller/README.md)。

## How to use the example

### Hardware Required

* ESP开发板，支持RGB LCD外围设备和PSRAM
* 一个通用RGB面板，16位宽，带有HSYNC、VSYNC和DE信号
* 用于供电和编程的USB电缆

### Hardware Connection

The connection between ESP Board and the LCD is as follows:

```
       ESP Board                           RGB  Panel
+-----------------------+              +-------------------+
|                   GND +--------------+GND                |
|                       |              |                   |
|                   3V3 +--------------+VCC                |
|                       |              |                   |
|                   PCLK+--------------+PCLK               |
|                       |              |                   |
|             DATA[15:0]+--------------+DATA[15:0]         |
|                       |              |                   |
|                  HSYNC+--------------+HSYNC              |
|                       |              |                   |
|                  VSYNC+--------------+VSYNC              |
|                       |              |                   |
|                     DE+--------------+DE                 |
|                       |              |                   |
|               BK_LIGHT+--------------+BLK                |
+-----------------------+              |                   |
                               3V3-----+DISP_EN            |
                                       |                   |
                                       +-------------------+
```

本例使用的GPIO号可以在[lvgl_example_main.c](main/rgb_lcd_example_main.c)中更改。

特别是，请注意用于打开LCD背光的电平，一些LCD模块需要低电平才能打开，而另一些则需要高电平。您可以在[lvgl_EXAMPLE_main.c](main/rgb_LCD_EXAMPLE_main.c)中更改背光级别宏`EXAMPLE_LCD_BK_LIGHT_ON_level`。

如果RGB LCD面板仅支持DE模式，则可以通过将`EXAMPLE_PIN_NUM_HSYNC`和`EXAMPLE_PIN_NUM_VSYNC`指定为`-1`来绕过`HSYNC`和`VSYNC`信号。

### Build and Flash

Run `idf.py -p PORT build flash monitor` to build, flash and monitor the project. A scatter chart will show up on the LCD as expected.

The first time you run `idf.py` for the example will cost extra time as the build system needs to address the component dependencies and downloads the missing components from registry into `managed_components` folder.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

### Example Output

```bash
...
I (0) cpu_start: Starting scheduler on APP CPU.
I (731) spiram: Reserving pool of 32K of internal memory for DMA/internal allocations
I (731) example: Turn off LCD backlight
I (731) gpio: GPIO[39]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0
I (731) example: Install RGB panel driver
I (741) example: Turn on LCD backlight
I (741) example: Initialize LVGL library
I (741) example: Register display driver to LVGL
I (741) example: Install LVGL tick timer
I (741) example: Display LVGL Scatter Chart
...
```

## Troubleshooting

* Why the LCD doesn't light up?
  * Check the backlight's turn-on level, and update it in `EXAMPLE_LCD_BK_LIGHT_ON_LEVEL`
* No memory for frame buffer
  * The frame buffer of RGB panel is located in ESP side (unlike other controller based LCDs, where the frame buffer is located in external chip). As the frame buffer usually consumes much RAM (depends on the LCD resolution and color depth), we recommend to put the frame buffer into PSRAM (like what we do in this example). However, putting frame buffer in PSRAM will limit the PCLK to around 12MHz (due to the bandwidth of PSRAM).
* LCD screen drift
  * Slow down the PCLK frequency
  * Adjust other timing parameters like PCLK clock edge (by `pclk_active_neg`), sync porches like HBP (by `hsync_back_porch`) according to your LCD spec

For any technical queries, please open an [issue] (https://github.com/espressif/esp-idf/issues) on GitHub. We will get back to you soon.