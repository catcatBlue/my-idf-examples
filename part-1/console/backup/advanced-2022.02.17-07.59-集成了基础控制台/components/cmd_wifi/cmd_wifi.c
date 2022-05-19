/**
 * @file cmd_wifi.c
 * @author catcatBlue (1625237402@qq.com)
 * @brief 对console_example_main.c的封装，使cmd_wifi.c作为组件
 * @version 0.1
 * @date 2022-02-15
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Console example — WiFi commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_console.h"

#include "argtable3/argtable3.h"

// #include "cmd_decl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "cmd_wifi.h"

#define JOIN_TIMEOUT_MS (10000)

/* 句柄 */
static EventGroupHandle_t wifi_event_group; /* wifi事件组句柄 */
const int                 CONNECTED_BIT = BIT0;

/**
 * @brief 事件服务函数
 *
 * @param arg 未使用
 * @param event_base 事件类型
 * @param event_id 事件id
 * @param event_data 事件数据
 */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

/**
 * @brief 初始化wifi
 * 
 */
static void initialise_wifi(void)
{
    /* 只输出警告及以上级别信息 */
    esp_log_level_set("wifi", ESP_LOG_WARN);

    /* 防止重复初始化 */
    static bool initialized = false;
    if (initialized) {
        return;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif);
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
    initialized = true;
}

/**
 * @brief 连接wifi
 * 
 * @param ssid wifi名称
 * @param pass 密码
 * @param timeout_ms 连接超时时间
 * @return true 
 * @return false 
 */
static bool wifi_join(const char *ssid, const char *pass, int timeout_ms)
{
    /* 初始化wifi */
    initialise_wifi();
    /* wifi配置 */
    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    if (pass) {
        strlcpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    }
    /* 站点模式连接wifi */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();

    int bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                                   pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS);
    return (bits & CONNECTED_BIT) != 0;
}

/** Arguments used by 'join' function */
static struct {
    struct arg_int *timeout;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} join_args;

static int connect(int argc, char **argv)
{
    /* 字符串解析对应参数 */
    int nerrors = arg_parse(argc, argv, (void **)&join_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, join_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Connecting to '%s'",
             join_args.ssid->sval[0]);

    /* 设置默认值 */
    if (join_args.timeout->count == 0) {
        join_args.timeout->ival[0] = JOIN_TIMEOUT_MS;
    }

    bool connected = wifi_join(join_args.ssid->sval[0],
                               join_args.password->sval[0],
                               join_args.timeout->ival[0]);
    if (!connected) {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    ESP_LOGI(__func__, "Connected");
    return 0;
}

/**
 * @brief 注册wifi组件
 * 
 */
void register_wifi(void)
{
    join_args.timeout  = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    join_args.ssid     = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    join_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    join_args.end      = arg_end(2);

    const esp_console_cmd_t join_cmd = {
        .command  = "join",
        .help     = "Join WiFi AP as a station",
        .hint     = NULL,
        .func     = &connect,
        .argtable = &join_args};

    ESP_ERROR_CHECK(esp_console_cmd_register(&join_cmd));
}
