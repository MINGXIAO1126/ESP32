#ifndef __P085_H_
#define __P085_H_

#include "stdbool.h"

//led灯灯效
typedef enum {
    LED_ON = 0,
    LED_OFF,
    LED_FLASH,
    LED_LIGHT
}led_mode_t;

extern led_mode_t led_mode;//灯效模式
extern bool led_state;//灯当前状态
extern bool is_double_pressing;//双击按下标志
extern bool is_pressing;//单击按下标志


void start_task (void *param);//开始任务函数
void test_task(void);

#endif       
