#ifndef __BUTTON_H_
#define __BUTTON_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define BTN GPIO_NUM_9  //sw1

extern TaskHandle_t BtnTask_handler; 
extern esp_timer_handle_t single_click_timer;

void btn_init(void);//按键初始化
void start_single_timer(void);//按键单击

#endif
