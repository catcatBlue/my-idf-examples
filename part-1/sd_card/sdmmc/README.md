| Supported Targets | ESP32 |
| ----------------- | ----- |

# SD Card example (SDMMC)

(See the README.md file in the upper level 'examples' directory for more information about examples.)

本示例演示如何将 SD 卡与 ESP 设备一起使用。示例执行以下步骤：

1. 使用“一体式”的`esp_vfs_fat_sdmmc_mount`功能：
   - 初始化 SDMMC 外设，
   - 探测并初始化连接到 SD/MMC 插槽 1 的卡（HS2_CMD、HS2_CLK、HS2_D0、HS2_D1、HS2_D2、HS2_D3 线），
   - 使用 FATFS 库安装 FAT 文件系统（如果无法安装文件系统，则使用格式化卡），
   - 在 VFS 中注册 FAT 文件系统，以便使用 C 标准库和 POSIX 函数。
2. 打印有关卡的信息，例如名称、类型、容量和支持的最大频率。
3. 使用`fopen`创建文件，并使用`fprintf`写入。
4. 重命名文件。重命名之前，请使用`stat`函数检查目标文件是否已存在，并使用`unlink`函数将其删除。
5. 打开重命名的文件进行读取，读回该行，并将其打印到终端。

本例支持 SD（SDSC、SDHC、SDXC）卡和 eMMC 芯片。

## 硬件连接

此示例在 ESP-WROVER-KIT 板上运行，无需任何额外修改，只需将 SD 卡插入插槽。

其他 ESP32 开发板需要连接到 SD 卡，如下所示：

| ESP32 pin     | SD card pin | Notes                                                                                   |
| ------------- | ----------- | --------------------------------------------------------------------------------------- |
| GPIO14 (MTMS) | CLK         | 10k pullup in SD mode                                                                   |
| GPIO15 (MTDO) | CMD         | 10k pullup in SD mode                                                                   |
| GPIO2         | D0          | 10k pullup in SD mode, pull low to go into download mode (see Note about GPIO2 below!)  |
| GPIO4         | D1          | not used in 1-line SD mode; 10k pullup in 4-line SD mode                                |
| GPIO12 (MTDI) | D2          | not used in 1-line SD mode; 10k pullup in 4-line SD mode (see Note about GPIO12 below!) |
| GPIO13 (MTCK) | D3          | not used in 1-line SD mode, but card's D3 pin must have a 10k pullup                    |

本例不使用 SD 卡插槽中的卡检测（CD）和写保护（WP）信号。

### 关于 GPIO2 的说明（仅限 ESP32）

GPIO2 引脚用作自举引脚，应处于低位才能进入 UART 下载模式。一种方法是使用跳线连接 GPIO0 和 GPIO2，然后在进入下载模式时，大多数开发板上的自动复位电路将 GPIO2 与 GPIO0 一起拉低。

- 一些电路板在 GPIO2 上有下拉和/或 LED。LED 通常正常，但下拉会干扰 D0 信号，必须移除。检查开发板的示意图，查看是否有任何连接到 GPIO2 的内容。

### 关于 GPIO12 的说明（仅限 ESP32）

GPIO12 用作自举引脚，用于选择为闪存芯片（VDD_SDIO）供电的内部稳压器的输出电压。该引脚有一个内部下拉菜单，因此如果保持未连接状态，复位时的读数将为低（选择默认 3.3V 操作）。在为 SD 卡操作添加一个上拉针时，请考虑下列事项：

- 对于不使用内部调节器（VDD_SDIO）为闪存供电的电路板，GPIO12 可以拉高。
- 对于使用 1.8V 闪存芯片的电路板，需要在复位时将 GPIO12 拉高。这与 SD 卡操作完全兼容。
- 在使用内部调节器和 3.3V 闪存芯片的电路板上，GPIO12 必须在复位时处于低电平。这与 SD 卡操作不兼容。
  - In most cases, external pullup can be omitted and an internal pullup can be enabled using a `gpio_pullup_en(GPIO_NUM_12);` call. Most SD cards work fine when an internal pullup on GPIO12 line is enabled. Note that if ESP32 experiences a power-on reset while the SD card is sending data, high level on GPIO12 can be latched into the bootstrapping register, and ESP32 will enter a boot loop until external reset with correct GPIO12 level is applied.
  - Another option is to burn the flash voltage selection efuses. This will permanently select 3.3V output voltage for the internal regulator, and GPIO12 will not be used as a bootstrapping pin. Then it is safe to connect a pullup resistor to GPIO12. This option is suggested for production use.

The following command can be used to program flash voltage selection efuses **to 3.3V**:

```sh
    components/esptool_py/esptool/espefuse.py set_flash_voltage 3.3V
```

This command will burn the `XPD_SDIO_TIEH`, `XPD_SDIO_FORCE`, and `XPD_SDIO_REG` efuses. With all three burned to value 1, the internal VDD_SDIO flash voltage regulator is permanently enabled at 3.3V. See the technical reference manual for more details.

`espefuse.py` has a `--do-not-confirm` option if running from an automated flashing script.

See [the document about pullup requirements](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/sd_pullup_requirements.html) for more details about pullup support and compatibility of modules and development boards.

## 如何使用示例

### 4 线和 1 线 SD 模式

默认情况下，示例代码使用以下初始值设定项进行 SDMMC 插槽配置：

```c++
sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
```

除此之外，这将设置`slot_config.width = 0`，这意味着 SD/MMC 驱动程序将使用插槽支持的最大总线宽度。对于插槽 1，初始化卡时将切换到 4 线模式（初始通信总是在 1 线模式下进行）。 如果卡的一些 D1、D2、D3 引脚未连接到 ESP32，请设置`slot_config.width=1`，然后 SD/MMC 驱动程序将不会尝试切换到 4 线模式。

请注意，即使卡的 D3 线未连接到 ESP32，也必须将其拔出，否则卡将进入 SPI 协议模式。

### Build and flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with serial port name.)

(To exit the serial monitor, type `Ctrl-]`.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example output

下面是一个控制台输出示例。在本例中，连接了一个 128MB SDSC 卡，并启用了`EXAMPLE_FORMAT_IF_MOUNT_FAILED`菜单配置选项。卡未格式化，因此初始装载失败。然后对卡片进行分区、格式化并再次装入。

```
I (336) example: Initializing SD card
I (336) example: Using SDMMC peripheral
I (336) gpio: GPIO[13]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0
W (596) vfs_fat_sdmmc: failed to mount card (13)
W (596) vfs_fat_sdmmc: partitioning card
W (596) vfs_fat_sdmmc: formatting card, allocation unit size=16384
W (7386) vfs_fat_sdmmc: mounting again
Name: XA0E5
Type: SDHC/SDXC
Speed: 20 MHz
Size: 61068MB
I (7386) example: Opening file /sdcard/hello.txt
I (7396) example: File written
I (7396) example: Renaming file /sdcard/hello.txt to /sdcard/foo.txt
I (7396) example: Reading file /sdcard/foo.txt
I (7396) example: Read from file: 'Hello XA0E5!'
I (7396) example: Card unmounted
```

## Troubleshooting

### 未能下载示例

```
Connecting........_____....._____....._____....._____....._____....._____....._____

A fatal error occurred: Failed to connect to Espressif device: Invalid head of packet (0x34)
```

断开 SD 卡 D0/MISO 线路与 GPIO2 的连接，然后再次尝试上载。阅读上面的“关于 GPIO2 的说明”。

### 卡无法初始化，出现`sdmmc_init_sd_scr: send_scr (1) returned 0x107`错误

检查卡和 ESP32 之间的连接。例如，如果您已断开 GPIO2 以解决闪烁问题，请将其重新连接并重置 ESP32（使用开发板上的按钮，或按 IDF Monitor 中的 Ctrl-T Ctrl-R）。

### 卡无法初始化，`sdmmc_check_scr: send_scr returned 0xffffffff`错误

卡和 ESP32 之间的连接对于使用的频率来说太长。尝试使用更短的连接，或尝试降低 SD 接口的时钟速度。

### 安装文件系统失败

```
example: Failed to mount filesystem. If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.
```

该示例将只能装载使用 FAT32 文件系统格式化的卡。如果该卡被格式化为 exFAT 或其他文件系统，则可以在示例代码中对其进行格式化。启用'EXAMPLE_FORMAT_IF_MOUNT_FAILED'菜单配置选项，然后构建并刷新示例。
