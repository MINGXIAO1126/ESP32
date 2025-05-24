#include <stdio.h>
#include "esp_log.h"
#include "led.h"
#include "button.h"
#include "P085_drv.h"
#include "ens210.h"
#include "ntc.h"
#include "esp_adc/adc_continuous.h"
#include "id.h"

#define TAG "啦啦啦"

void app_main(void)
{
    ESP_LOGE(TAG, "--------------------------------------开始测试--------------------------------------");
    led_init();//LED初始化
    btn_init();//按键初始化
    id_init();//id初始化
    i2c_master_init(); // i2c初始化
    ntc_adc_init();//ntc初始化

    test_task();

    // uint8_t buf [8];
    // uint16_t buf_len = 8;
    // ntc_adc_init();
    // float ret = get_voltage(buf,buf_len);
    // ESP_LOGE(TAG,"电压值：%.2fV",ret);
    // float temp = get_temperature(ret);
    // ESP_LOGE(TAG,"temp:%.2f",temp);


}
