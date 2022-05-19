/**
 * @file CatB_console_library.c
 * @author catcatBlue (1625237402@qq.com)
 * @brief
 * @version 0.1
 * @date 2022-02-14
 *
 * @copyright Copyright (c) 2022
 *
 */

/* 标准库 */
#include <stdio.h>
#include <string.h>

/* freertos相关库，并不是所有都用 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/ringbuf.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"

#include "driver/uart.h"

#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

#include "cmd_decl.h"

#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "CatB_console_library.h"


#ifdef CONFIG_ESP_CONSOLE_USB_CDC
#error This example is incompatible with USB CDC console. Please try "console_usb" example instead.
#endif  // CONFIG_ESP_CONSOLE_USB_CDC

/* 与控制台任务有关 */
#define CONSOLE_TASK_STACK_DEPTH (1024 * 4)                 /* 控制台任务栈深 */
#define CONSOLE_TASK_PRIORITIES (configMAX_PRIORITIES - 20) /* 控制台任务优先级 */

/* 句柄 */
static xTaskHandle console_task_handle = NULL; /* 控制台任务的句柄 */

/* 变量 */
static bool console_initialised = false; /* 控制台初始化标志 */

static const char *TAG = "console";
#define PROMPT_STR CONFIG_IDF_TARGET

/* 函数 */
static void initialize_filesystem(void);
static void initialize_nvs(void);
static void initialize_console(void);


/* 控制台命令历史记录可以存储到文件中，也可以从文件中加载。
 * 最简单的方法是在“wear_Leveling”库之上使用FATFS文件系统。
 */
#if CONFIG_STORE_HISTORY

#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

/**
 * @brief 初始化文件系统
 *
 */
static void initialize_filesystem(void)
{
    static wl_handle_t               wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
        .max_files              = 4,
        .format_if_mount_failed = true};
    esp_err_t err = esp_vfs_fat_spiflash_mount(MOUNT_PATH, "storage", &mount_config, &wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }
}
#endif  // CONFIG_STORE_HISTORY


/**
 * @brief 控制台任务，初始化nvs、控制台，注册命令，进入控制台
 *
 */
static void console_task(void *pvParameters)
{
    /* 初始化nvs */
    initialize_nvs();

    /* 存储命令历史 */
#if CONFIG_STORE_HISTORY
    initialize_filesystem();
    ESP_LOGI(TAG, "Command history enabled");
#else
    ESP_LOGI(TAG, "Command history disabled");
#endif

    /* 初始化控制台 */
    initialize_console();

    /* 注册命令 */
    esp_console_register_help_command();
    register_system();
    register_wifi();
    register_nvs();

    /* 每行输入命令之前的提示。可以是定制的、动态的等等。*/
    const char *prompt = LOG_COLOR_I PROMPT_STR "> " LOG_RESET_COLOR;

    // printf("\n"
    //        "This is an example of ESP-IDF console component.\n"
    //        "Type 'help' to get the list of commands.\n"
    //        "Use UP/DOWN arrows to navigate through command history.\n"
    //        "Press TAB when typing command name to auto-complete.\n"
    //        "Press Enter or Ctrl+C will terminate the console environment.\n");
    printf("\n"
           "封装ESP-IDF控制台组件及对应例程后的示例。\n"
           "输入\"help\"可获取命令列表。\n"
           "如果上位机支持，可以使用上/下箭头可以选择输入的历史记录。\n"
           "如果上位机支持，支持按TAB键自动补全。\n"
           "按Enter键或Ctrl+C键将终止控制台。\n"
           "清屏：ctrl+l\n");

    /* 测试终端是否支持转义序列 */
    int probe_status = linenoiseProbe();

    /* 零表示成功 */
    if (probe_status) {
        printf("\n\n"
               "终端不支持转义序列。\n"
               "禁用行编辑和历史记录功能。\n"
               "Windows系统可以用Putty。\n\n");
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* 终端不支持转义序列，不要在提示中使用颜色代码。*/
        prompt = PROMPT_STR "> ";
#endif  // CONFIG_LOG_COLORS
    }

    /* 主循环 */
    for (;;) {
        /* 使用linenoise获得一行
         * 按ENTER键时返回该行。
         */
        char *line = linenoise(prompt);
        /* 因EOF或错误而中断 */
        if (line == NULL) {
            // break;/* 只有Enter输入时退出控制台 */
			printf("\n");
            continue;/* 只有Enter输入时进入下一次循环 */
        }
        /* 如果不为空，则将该命令添加到历史记录中 */
        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
            /* 将命令历史记录保存到文件系统 */
            linenoiseHistorySave(HISTORY_PATH);
#endif
        }

        /* 尝试运行命令 */
        int       ret;
        esp_err_t err = esp_console_run(line, &ret);

        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise在堆上分配行缓冲区，所以需要释放它 */
        linenoiseFree(line);

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGE(TAG, "Error or end-of-input, terminating console");
    esp_console_deinit();
    vTaskDelete(NULL);
}


/**
 * @brief 初始化并启动console_task
 *
 * @param pvParameters 传参到任务，没用
 */
void console_task_start(void *pvParameters)
{
    ESP_LOGI(TAG, "创建任务");
    if (!console_initialised) {
        console_initialised = true;
        xTaskCreate(console_task,             /* 指向需要创建的任务 */
                    "console_task",           /* 任务的文本名字，用于调试 */
                    CONSOLE_TASK_STACK_DEPTH, /* 栈深，单位字 */
                    pvParameters,             /* 传入任务的参数 */
                    CONSOLE_TASK_PRIORITIES,  /* 任务优先级 */
                    &console_task_handle      /* 控制该任务的句柄，注意要'&' */
        );
    }
}
/**
 * @brief 删除console_task_
 *
 */
void console_task_stop(void)
{
    ESP_LOGI(TAG, "删除任务");
    if (console_initialised != false) {
        if (console_task_handle != NULL) {
            vTaskDelete(console_task_handle);
        }
        console_initialised = false;
    }
}


/**
 * @description: 非易失性存储 (NVS) 库初始化
 * @param {*}
 * @return {*}
 */
static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
/**
 * @description: 初始化控制台
 * @param {*}
 * @return {*}
 */
static void initialize_console(void)
{
    /* 在重新配置stdout之前清除它，清空文件缓冲区（或标准输入输出缓冲区） */
    fflush(stdout);
    fsync(fileno(stdout));

    /* 禁用stdin上的缓冲 */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* 按下回车键时，“Minicom”、“screen”、“idf_monitor”发送CR */
    esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    /* 将插入符号移动到“\n”上下一行的开头 */
    esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

    /*配置UART。
      请注意，使用REF_TICK是为了在轻睡眠模式下APB频率变化时波特率保持正确。
     */
    const uart_config_t uart_config = {
        .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
        .source_clk = UART_SCLK_REF_TICK,
#else
        .source_clk = UART_SCLK_XTAL,
#endif
    };
    /*为中断驱动的读写安装UART驱动程序*/
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, /* 串口号 */
                                        256,                         /* 接收环形缓冲大小 */
                                        0,                           /* 发送环形缓冲大小 */
                                        0,                           /* 串口事件队列深度 */
                                        NULL,                        /* 串口队列句柄 */
                                        0                            /* 中断标志 */
                                        ));
    /* 配置串口参数 */
    ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, /* 串口号 */
                                      &uart_config                 /* 串口配置结构体 */
                                      ));

    /* 告诉VFS（虚拟文件系统）使用UART驱动程序 */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* 初始化控制台 */
    esp_console_config_t console_config = {
        .max_cmdline_args   = 8,   /* 命令行最大参数数 */
        .max_cmdline_length = 256, /* 命令行缓冲区的长度（字节） */
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN) /* 提示文本的ASCII颜色代码 */
#endif
    };
    /* 初始化命令行 */
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    /* 配置linenoise行补全库 */
    /* 启用多行编辑。如果未设置，长命令将在单行内滚动。*/
    linenoiseSetMultiLine(1);

    /* 告诉linenoise从哪里获取命令完成和提示 */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback *)&esp_console_get_hint);

    /* 设置命令历史记录大小 */
    linenoiseHistorySetMaxLen(100);

    /* 设置命令最大长度 */
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    /* 不要返回空行 */
    linenoiseAllowEmpty(false);

#if CONFIG_STORE_HISTORY
    /* 从文件系统加载命令历史记录 */
    linenoiseHistoryLoad(HISTORY_PATH);
#endif
}
