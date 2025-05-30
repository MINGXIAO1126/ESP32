#include <stdio.h>
#include "esp_log.h"
#include "led.h"
#include "button.h"
#include "P085_drv.h"
#include "ens210.h"
#include "ntc.h"
#include "esp_adc/adc_continuous.h"
#include "id.h"
#include "RS485.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "RJ12.h"

#define TAG "啦啦啦"


void app_main(void)
{
    ESP_LOGE(TAG, "--------------------------------------开始测试--------------------------------------");
    led_init();//LED初始化
    btn_init();//按键初始化
    id_init();//id初始化
    i2c_master_init(); // i2c初始化
    ntc_adc_init();//ntc初始化
    rj_init();//rj初始化

    test_task();

    
    
    // rs485_uart_init();
    // while (1)
    // {
    //     RS_TX_ON;
    //     esp_err_t ret = RS_transmit();
    //     ESP_LOGI(TAG,"%d",ret);
    //     RS_RX_ON;
    //     vTaskDelay(pdMS_TO_TICKS(3000));
    // }

    //     gpio_config_t led_cfg = {
    //     .pin_bit_mask = TEST_GPIO,
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .pull_up_en = GPIO_PULLUP_DISABLE,
    //     .intr_type = GPIO_INTR_DISABLE
    // };

    // gpio_config(&led_cfg);
    // gpio_set_level(TEST_GPIO,1);

}
