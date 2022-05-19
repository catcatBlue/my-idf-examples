/* UART Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/uart.h"
#include "esp_log.h"

#include "uart_events.h"
#include "CatB_console_library.h"

static const char *TAG = "uart_events";

/**
 * This example shows how to use the UART driver to handle special UART events.
 *
 * It also reads data from UART0 directly, and echoes it to console.
 *
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: on
 * - Pin assignment: TxD (default), RxD (default)
 */

#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM (3) /*!< 设置定义UART模式的接收器接收的连续和相同字符数 */

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

/* 串口事件队列 */
static QueueHandle_t uart0_queue;

static xTaskHandle uart_events_handle = NULL;

/**
 * @brief 串口事件处理任务
 *
 * @param pvParameters
 * last_editors
 */
static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t       buffered_size;
    uint8_t     *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
    for (;;) {
        /* 等待UART事件。 */
        if (xQueueReceive(uart0_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE); /* 清空缓冲 */
            ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch (event.type) {
            /* UART接收数据的事件 */
            /* 最好快速处理数据事件，数据事件将比其他类型的事件多得多。
               如果在数据事件上花费太多时间，队列会满。*/
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA SIZE]: %d", event.size);
                uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                ESP_LOGI(TAG, "[DATA EVT]:");
                uart_write_bytes(EX_UART_NUM, (const char *)dtmp, event.size);
                break;
            /* 检测到硬件FIFO溢出事件 */
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                /* 如果FIFO溢出，应该考虑为应用程序添加流量控制。
                   ISR已重置接收FIFO。
                   例如，为了读取更多数据，我们在这里直接刷新rx缓冲区。 */
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            /* UART环缓冲区已满事件 */
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                /*如果缓冲区满了，您应该考虑增加缓冲区大小。
                  例如，为了读取更多数据，我们在这里直接刷新rx缓冲区。*/
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            /* 检测到UART接收中断事件 */
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            /* UART奇偶校验错误事件 */
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            /* UART帧错误事件 */
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            /* Uart模式数据传输 */
            case UART_PATTERN_DET:
                uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                int pos = uart_pattern_pop_pos(EX_UART_NUM);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1) {
                    /* 以前有一个UART_PATTERN_DET事件，但PATTERN position队列已满，因此无法运行
                       记录位置。我们应该设置更大的队列大小。
                       例如，我们在这里直接刷新rx缓冲区。 */
                    uart_flush_input(EX_UART_NUM);
                } else {
                    uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", dtmp);
                    /* 如果模式匹配到只有'c'则进入控制台 */
                    if (/* (strlen((char *)dtmp) == 1) &&  */(strncmp((char *)dtmp, "c", 1))) {
                        console_task_start(NULL);
                        vTaskDelete(uart_events_handle);
                    }
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
            /* 其他 */
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void uart_events_init(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    /* 配置UART驱动程序的参数、通信引脚并安装驱动程序 */
    uart_config_t uart_config = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    /* 安装UART驱动程序，并获取队列。 */
    uart_driver_install(EX_UART_NUM,  /* 串口号 */
                        BUF_SIZE * 2, /* 接收唤醒缓冲大小 */
                        BUF_SIZE * 2, /* 发送唤醒缓冲大小 */
                        20,           /* 队列大小 */
                        &uart0_queue, /* 指向队列 */
                        0             /* 终端标志 */
    );
    /* 配置串口参数 */
    uart_param_config(EX_UART_NUM, &uart_config);

    ///* 设置UART日志级别 */
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //设置UART引脚（使用UART0默认引脚，即无更改。）
    uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    /* 设置uart模式检测功能。 */
    uart_enable_pattern_det_baud_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 9, 0, 0);
    /* 将图案队列长度重置为最多记录20个图案位置。 */
    uart_pattern_queue_reset(EX_UART_NUM, 20);

    /* 从ISR创建任务到处理程序UART事件 */
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, &uart_events_handle);
}
