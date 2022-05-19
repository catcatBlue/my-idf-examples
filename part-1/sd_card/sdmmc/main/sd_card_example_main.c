/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

//本例使用SDMMC外设与SD卡通信。

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"

#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include "driver/gpio.h"

#include "test_fatfs_common.h"

/* kushi */
// #define SD_D3 GPIO_NUM_8
// #define SD_CMD GPIO_NUM_4
// #define SD_CLK GPIO_NUM_3
// #define SD_D0 GPIO_NUM_0

/*  */
#define SD_D3 GPIO_NUM_4
#define SD_CMD GPIO_NUM_5
#define SD_CLK GPIO_NUM_6
#define SD_D0 GPIO_NUM_7

static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"


void app_main(void)
{
    esp_err_t ret;

    /* 设置D3输出高 */
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << SD_D3) | (1ULL << GPIO_NUM_14),
        .pull_down_en = 0,
        .pull_up_en   = 1,
    };
    gpio_config(&io_conf);

    gpio_set_level(SD_D3, 1);
    gpio_set_level(GPIO_NUM_14, 1);

    //安装文件系统的选项。
    //如果format_if_mount_failed设置为true，则SD卡将被分区并格式化，以防装载失败。
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

    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = /* 10 */ /* SDMMC_FREQ_PROBING */SDMMC_FREQ_HIGHSPEED; /* SDMMC_FREQ_HIGHSPEED */

    //这会在没有卡检测（CD）和写保护（WP）信号的情况下初始化插槽。
    //修改slot_config.gpio_cd and slot_config.gpio_wp，如果你的电路板有这些信号。
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    //要使用单线SD模式，请将其更改为1：
    slot_config.width = 1;
    //在可以配置SD卡使用的GPIO的芯片上，在插槽配置结构中设置它们：
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
    // slot_config.clk = GPIO_NUM_14;
    // slot_config.cmd = GPIO_NUM_15;
    // slot_config.d0  = GPIO_NUM_2;
    // slot_config.d1  = GPIO_NUM_4;
    // slot_config.d2  = GPIO_NUM_12;
    // slot_config.d3  = GPIO_NUM_13;

    slot_config.clk = SD_CLK;
    slot_config.cmd = SD_CMD;
    slot_config.d0  = SD_D0;
#endif

    //在启用的管脚上启用内部上拉。内部上拉不足，但请确保总线上连接了10k外部上拉。这仅用于调试/示例目的。
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "安装文件系统");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "无法装载文件系统。"
                          "如果要格式化卡，请设置示例 EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig选项。");
        } else {
            ESP_LOGE(TAG, "无法初始化卡 (%s). "
                          "确保SD卡线路上有上拉电阻。",
                     esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "文件系统挂载");

    //卡已初始化，请打印其属性
    sdmmc_card_print_info(stdout, card);

    //使用POSIX和C标准库函数处理文件：

    //首先创建一个文件。
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

    //重命名前检查目标文件是否存在
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        //如果存在，请将其删除
        unlink(file_foo);
    }

    //重命名原始文件
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    //打开重命名的文件进行读取
    ESP_LOGI(TAG, "Reading file %s", file_foo);
    f = fopen(file_foo, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    //从文件中读一行
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

    //全部完成，卸载分区并禁用SDMMC外围设备
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
}
