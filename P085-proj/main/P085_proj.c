#include <stdio.h>
#include "esp_log.h"
#include "led.h"
#include "button.h"
#include "P085_drv.h"
#include "ens210.h"
#include "ntc.h"
#include "esp_adc/adc_continuous.h"
#include "id.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "RJ12.h"
#include "RS485.h"
#include "RS485_master.h"

#define TAG "啦啦啦"


void app_main(void)
{
   
    //ESP_LOGE(TAG, "------------------ TESTING OF THE P085 PROJECT -------------------------");
    ESP_LOGE(TAG, "--------------------   开始测试   -----------------------------------");

    led_init();//LED初始化
    btn_init();//按键初始化
    id_init();//id初始化
    i2c_master_init(); // i2c初始化
    ntc_adc_init();//ntc初始化
    rj_init();//rj初始化

    test_task();


/*********************************通过串口通信测试RS485接口************************************* */
// #define BUF_SIZE  1025 
// rs_uart_init();
//     uint8_t data [BUF_SIZE]={0};
//     while (1)
//     {
//         int len = uart_read_bytes(RS_UART_NUM,data,BUF_SIZE-1,pdMS_TO_TICKS(20));
//         if(len>0)
//         {
//             if(len<BUF_SIZE)
//             {
//                 data[len]='\0';
//             }else
//             {
//                 data[BUF_SIZE-1]= '\0';
//             }
//             ESP_LOGE(TAG,"RX: %s",(char *)data);   
            
//             rs_send_data((char *)data,len);
//         }
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
  
}
