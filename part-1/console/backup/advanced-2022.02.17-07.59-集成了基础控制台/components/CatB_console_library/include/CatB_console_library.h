/**
 * @file CatB_console_library.h
 * @author catcatBlue
 * @brief 对console_example_main.c的封装
 * @version 0.1
 * @date 2022-02-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void console_task_start(void *pvParameters);
void console_task_stop(void);

void register_console_exit(void);
void register_console_test(void);

#ifdef __cplusplus
}
#endif
