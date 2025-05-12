#include <stdio.h>
#include "P085_drv.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/gpio.h"//todo该头文件是esp_driver_gpio组件提供的API的一部分 ，要声明组件依赖于esp_driver_gpio，则添加REQUIRES esp_driver_gpio在cmakelist中

//*任务函数必须永不返回，如果确实需要终止任务，则需要调用vTASKDelete（）；

//?#define LED_R GPIO_NUM_14       //PWM_R_LED-IO14
#define LED_G GPIO_NUM_15       //PWM_G_LED-IO15
#define LED_B GPIO_NUM_8        //PWM_B_LED-IO8

#define TAG "哈哈哈"

//开始任务
#define START_TASK_DEPTH     1024//任务栈深
#define START_TASK_PRIOIRIY  1//任务优先级

// //LED任务
//? #define LED_TASK_DEPTH     1024//任务栈深
// #define LED_TASK_PRIOIRIY  1//任务优先级


// ?led任务函数入口
// void led_run_task(void *param)
// {
//     int led_level = 0;
//     while(1){
//         led_level = led_level ? 0 : 1;
//         gpio_set_level(LED_R,led_level);
//         vTaskDelay(500);
//     }
// }

// //开始任务函数入口
// ?void start_task(void *param)
// {
//     //led任务
//     xTaskCreate(led_run_task,"led task",LED_TASK_DEPTH,NULL,LED_TASK_PRIOIRIY,NULL);

//     //删除启动任务
//     vTaskDelete(NULL);
// }

//? void task(void)
// {
//     // 创建开始任务函数
//     //!  pxTaskCode--任务函数（指向任务函数入口的指针，必须实现才能永不返回）
//     //!  pcName--任务名，主要是为了方便调试
//     //!  usStackDepth--任务栈深(以字节为单位)
//     //!  pvParameters--传递给任务函数的参数
//     //!  uxPriority--任务优先级
//     //!  pxCreatedTask--任务句柄，用于保存任务的数据结构
//     xTaskCreate(start_task, "start task", START_TASK_DEPTH, NULL, START_TASK_PRIOIRIY, NULL);
// }