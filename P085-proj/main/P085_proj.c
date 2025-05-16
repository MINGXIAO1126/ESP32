#include <stdio.h>
#include "esp_log.h"
#include "led.h"
#include "button.h"
#include "P085_drv.h"
#include "ens210.h"

#define TAG "啦啦啦"

void app_main(void)
{
    
    uint8_t value = 0b1;
    uint8_t uid[8] = {0};
    ESP_LOGI(TAG,"开始测试");

    // led_init();//LED初始化

    // btn_init();//按键初始化
   
    // test_task();

    i2c_master_init();//i2c初始化
    vTaskDelay(pdMS_TO_TICKS(2000));
    ens_write_sens_start(value);
    ens_read_uid(uid);

    // djahd_init();
    // sys_ctrl();
    // esp_err_t ret = get_uid();
    // ESP_LOGI(TAG, "UID:%x",ret);

}
