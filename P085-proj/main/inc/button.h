#ifndef __BUTTON_H_
#define __BUTTON_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BTN GPIO_NUM_9  //sw1

extern TaskHandle_t BtnTask_handler; 

void btn_init(void);

#endif
