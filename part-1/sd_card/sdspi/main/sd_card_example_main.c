/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// 本例使用SPI外围设备与SD卡通信。

#include <string.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_vfs_fat.h"

#include "sdmmc_cmd.h"

#include "test_fatfs_common.h"

#define SPI_DMA_CHAN SPI_DMA_CH_AUTO
#define MOUNT_POINT "/sdcard"

// Pin mapping

/* kushi */
// #define SD_D3 GPIO_NUM_8
// #define SD_CMD GPIO_NUM_4
// #define SD_CLK GPIO_NUM_3
// #define SD_D0 GPIO_NUM_0
// #define PIN_NUM_MISO SD_D0
// #define PIN_NUM_MOSI SD_CMD
// #define PIN_NUM_CLK SD_CLK
// #define PIN_NUM_CS SD_D3

/* spi2 */
// #define PIN_NUM_MISO GPIO_NUM_13
// #define PIN_NUM_MOSI GPIO_NUM_11
// #define PIN_NUM_CLK GPIO_NUM_12
// #define PIN_NUM_CS GPIO_NUM_10

/* spi3 */
#define PIN_NUM_CS GPIO_NUM_4
#define PIN_NUM_MOSI GPIO_NUM_5
#define PIN_NUM_CLK GPIO_NUM_6
#define PIN_NUM_MISO GPIO_NUM_7

// #define PIN_NUM_CS GPIO_NUM_4
// #define PIN_NUM_MOSI GPIO_NUM_5
// #define PIN_NUM_CLK GPIO_NUM_6
// #define PIN_NUM_MISO GPIO_NUM_7

static const char *TAG = "example";

void app_main(void)
{
    esp_err_t ret;

    /* 设置D3输出高 */
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_NUM_14,
        .pull_down_en = 0,
        .pull_up_en   = 1,
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_14, 1);

    //安装文件系统的选项。
    //如果format_If_mount_failed设置为true，则SD卡将被分区并格式化，以防装载失败。
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif  // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files            = 5,
        .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;
    const char    mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    //使用上面定义的设置初始化SD卡并装入FAT文件系统。
    //注：esp_vfs_fat_sdmmc/sdspi_mount是一款集多功能于一体的便利功能。
    //请检查其源代码，并在开发生产应用程序时实现错误恢复。
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    // host.max_freq_khz = /* 10 * 1000 *//* SDMMC_FREQ_52M *//* SDMMC_FREQ_HIGHSPEED */;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = PIN_NUM_MOSI,
        .miso_io_num     = PIN_NUM_MISO,
        .sclk_io_num     = PIN_NUM_CLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    //这会在没有卡检测（CD）和写保护（WP）信号的情况下初始化插槽。
    //修改插槽slot_config.gpio_cd and slot_config.gpio_wp，如果你的电路板有这些信号。
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs               = PIN_NUM_CS;
    slot_config.host_id               = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.

    // First create a file.
    const char *file_hello = MOUNT_POINT "/hello.txt";

    ESP_LOGI(TAG, "Opening file %s", file_hello);
    FILE *f = fopen(file_hello, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    const char *file_foo = MOUNT_POINT "/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file %s", file_foo);
    f = fopen(file_foo, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    // Read a line from file
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // const size_t buf_size = 16 * 1024;
    const size_t buf_size = 32 * 1024;
    uint32_t    *buf      = (uint32_t *)calloc(1, buf_size);
    esp_fill_random(buf, buf_size);
    // const size_t file_size = 256 * 1024;
    const size_t file_size = 512 * 1024;
    // const char* file = "/sdcard/256k.bin";
    const char *file = "/sdcard/512k.bin";

    test_fatfs_rw_speed(file, buf, 4 * 1024, file_size, true);
    test_fatfs_rw_speed(file, buf, 8 * 1024, file_size, true);
    test_fatfs_rw_speed(file, buf, 16 * 1024, file_size, true);
    test_fatfs_rw_speed(file, buf, 32 * 1024, file_size, true);

    test_fatfs_rw_speed(file, buf, 4 * 1024, file_size, false);
    test_fatfs_rw_speed(file, buf, 8 * 1024, file_size, false);
    test_fatfs_rw_speed(file, buf, 16 * 1024, file_size, false);
    test_fatfs_rw_speed(file, buf, 32 * 1024, file_size, false);


    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    // deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
}
