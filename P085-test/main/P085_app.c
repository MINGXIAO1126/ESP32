#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h" 
#include "freertos/FreeRTOS.h"
#include "P085_drv.h"
//todo 烧录：根据原理图，为什么不用去改变IO8和9的引脚电平将其设置为下载模式？bootloader里面已经设置了？
 
#define TAG "啦啦啦"
#define LED_R GPIO_NUM_14       //PWM_R_LED-IO14
//LED任务
#define LED_TASK_DEPTH     4096*4//任务栈深
#define LED_TASK_PRIOIRIY  1//任务优先级


void led_run_task(void *param)
{
    int led_level = 1;
    while (1)
    {
        ESP_LOGI(TAG,"成功");
        led_level = led_level ? 0 : 1;
        gpio_set_level(LED_R, led_level);
        vTaskDelay(500);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG,"开始测试");

    //led GPIO初始化
    gpio_config_t led_cfg ={
        .pin_bit_mask  = (1<<GPIO_NUM_14) | (1<<GPIO_NUM_15) | (1<<GPIO_NUM_8),
        .mode          = GPIO_MODE_OUTPUT,
        .pull_down_en  = GPIO_PULLDOWN_DISABLE,
        .pull_up_en    = GPIO_PULLUP_DISABLE,
        .intr_type     = GPIO_INTR_DISABLE
    };
    gpio_config(&led_cfg);

    xTaskCreate(led_run_task,"led task",LED_TASK_DEPTH,NULL,LED_TASK_PRIOIRIY,NULL);
    //?task();
    while(1){
        vTaskDelay(100);
        ESP_LOGI(TAG,"好烦啊");
    }
}
