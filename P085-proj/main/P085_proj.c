#include <stdio.h>
#include "esp_log.h"
#include "led.h"
#include "button.h"
#include "P085_drv.h"

#define TAG "啦啦啦"

void app_main(void)
{
    ESP_LOGI(TAG,"开始测试");

    led_init();//LED初始化

    btn_init();//按键初始化

    test_task();
}
