#include <stdio.h>
#include "P085_drv.h"
#include "esp_log.h"
#include "driver/gpio.h" //todo该头文件是esp_driver_gpio组件提供的API的一部分 ，要声明组件依赖于esp_driver_gpio，则添加REQUIRES esp_driver_gpio在cmakelist中
#include "button.h"
#include "led.h"
#include "ens210.h"
#include "id.h"
#include "ntc.h"

//*任务函数必须永不返回，如果确实需要终止任务，则需要调用vTASKDelete（）；

#define TAG "哈哈哈"

// 开始任务
#define START_TASK_DEPTH 1024 // 任务栈深
#define START_TASK_PRIOIRIY 1 // 任务优先级

// 按键任务
#define BTN_TASK_DEPTH 4096 // 任务栈深
#define BTN_TASK_PRIOIRTY 2 // 任务优先级

// 温湿度任务
#define ENS_TASK_DEPTH 4096
#define ENS_TASK_PRIOIRTY 1

// ID任务
#define ID_TASK_DEPTH 4096
#define ID_TASK_PRIOIRTY 1

// NTC任务
#define NTC_TASK_DEPTH 4096
#define NTC_TASK_PRIOIRTY 1

// 按键任务入口函数
void btn_run_task(void *param)
{
    static bool led_state = false; // 灯的开关状态
    TickType_t last_tick = 0;

    while (1)
    {
        // 等待中断通知
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

        TickType_t now_tick = xTaskGetTickCount();
        if (now_tick - last_tick > pdMS_TO_TICKS(50)) // 50ms防抖
        {
            // 每次按键释放时，切换灯的状态
            led_state = !led_state;

            if (led_state)
            {
                ESP_LOGE(TAG, "开灯");
                Rgb_led_flashing(); // 开灯
            }
            else
            {
                ESP_LOGE(TAG, "关灯");
                turn_of_light(); // 关灯
            }
            last_tick = now_tick;
        }
    }
}

// ID任务入口函数
void id_task(void *param)
{
    get_board_id();
    vTaskDelete(NULL);
}

// 温湿度任务入口函数
void ens_task(void *param)
{
    ens_sys_ctrl(); // 禁用低功耗
    uint8_t id_buf[8] = {0};
    ens_get_uid(id_buf);
    ESP_LOGE(TAG, "UID:%x%x%x%X%x%x%x%x", id_buf[0], id_buf[1], id_buf[2], id_buf[3], id_buf[4], id_buf[5], id_buf[6], id_buf[7]);
    ens_sens_start();
    ens_sens_run();

    while (1)
    {
        uint8_t read_buffer[3] = {0};
        ens_get_temperature(read_buffer);
        // ESP_LOGE(TAG, "T_val:0x%x%x%x", read_buffer[0], read_buffer[1], read_buffer[2]);
        uint32_t raw_temp = (read_buffer[2] << 16) | (read_buffer[1] << 8) | read_buffer[0];
        ens_temperature_conversion(raw_temp);
        ens_get_humidity(read_buffer);
        // ESP_LOGE(TAG, "T_val:0x%x%x%x", read_buffer[0], read_buffer[1], read_buffer[2]);
        raw_temp = (read_buffer[2] << 16) | (read_buffer[1] << 8) | read_buffer[0];
        ens_humidity_conversion(raw_temp);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void ntc_task(void *param)
{
    uint8_t buf[8];
    uint16_t buf_len = 8;

    while (1)
    {
        float ret = get_voltage(buf, buf_len);
        // ESP_LOGE(TAG, "电压值：%.2fV", ret);
        float temp = get_temperature(ret);
        ESP_LOGE(TAG, "热敏电阻--temp:%.2f", temp);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// 开始任务函数入口
void start_task(void *param)
{
    // 按键任务
    xTaskCreate(btn_run_task, "button task", BTN_TASK_DEPTH, NULL, BTN_TASK_PRIOIRTY, &BtnTask_handler);

    // ID 任务
    xTaskCreate(id_task, "id task", ID_TASK_DEPTH, NULL, ID_TASK_PRIOIRTY, NULL);

    // 温湿度任务
    xTaskCreate(ens_task, "ens task", ENS_TASK_DEPTH, NULL, ENS_TASK_PRIOIRTY, NULL);

    // 热敏电阻任务
    xTaskCreate(ntc_task, "ntc task", NTC_TASK_DEPTH, NULL, NTC_TASK_PRIOIRTY, NULL);

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