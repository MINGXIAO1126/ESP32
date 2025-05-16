#include <stdio.h>
#include "P085_drv.h"
#include "esp_log.h"
#include "driver/gpio.h" //todo该头文件是esp_driver_gpio组件提供的API的一部分 ，要声明组件依赖于esp_driver_gpio，则添加REQUIRES esp_driver_gpio在cmakelist中
#include "button.h"
#include "led.h"

//*任务函数必须永不返回，如果确实需要终止任务，则需要调用vTASKDelete（）；

#define TAG "哈哈哈"

// 开始任务
#define START_TASK_DEPTH 1024 // 任务栈深
#define START_TASK_PRIOIRIY 1 // 任务优先级

// 按键任务
#define BTN_TASK_DEPTH 4096 // 任务栈深
#define BTN_TASK_PRIOIRTY 1 // 任务优先级

// 按键任务入口函数
void btn_run_task(void *param)
{
    static bool led_state = false; // 灯的开关状态

    while (1)
    {
        // 等待中断通知
        ulTaskNotifyTake(pdFALSE,portMAX_DELAY);

        // 每次按键释放时，切换灯的状态
        led_state = !led_state;

        if (led_state)
        {
            ESP_LOGI(TAG, "开灯");
            Rgb_led_flashing(); // 开灯
        }
        else
        {
            ESP_LOGI(TAG, "关灯");
            turn_of_light(); // 关灯
        }
    }
}

// 开始任务函数入口
void start_task(void *param)
{
    // 按键任务
    xTaskCreate(btn_run_task, "button task", BTN_TASK_DEPTH, NULL, BTN_TASK_PRIOIRTY, &BtnTask_handler);

    // 删除启动任务
    vTaskDelete(NULL);
}

void test_task(void)
{
    // 创建开始任务函数
    //!  pxTaskCode--任务函数（指向任务函数入口的指针，必须实现才能永不返回）
    //!  pcName--任务名，主要是为了方便调试
    //!  usStackDepth--任务栈深(以字节为单位)
    //!  pvParameters--传递给任务函数的参数
    //!  uxPriority--任务优先级
    //!  pxCreatedTask--任务句柄，用于保存任务的数据结构
    xTaskCreate(start_task, "start task", START_TASK_DEPTH, NULL, START_TASK_PRIOIRIY, NULL);
}