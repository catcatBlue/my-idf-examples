/**
 * @file console_example_main.c
 * @author catcatBlue (1625237402@qq.com)
 * @brief
 * @version 0.1
 * @date 2022-02-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/ringbuf.h"
#include "freertos/event_groups.h"

#include "uart_events.h"

#include "CatB_console_library.h"

/**
 * @brief 主函数
 *
 */
void app_main(void)
{
    // console_task_start(NULL);
    uart_events_init();
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
