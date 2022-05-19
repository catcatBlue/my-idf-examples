/* Console example

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
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/ringbuf.h"
#include "freertos/event_groups.h"

#include "CatB_console_library.h"

/**
 * @brief 主函数
 *
 */
void app_main(void)
{
    console_task_start(NULL);
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
