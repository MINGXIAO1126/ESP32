#include <stdio.h>
#include "P085_drv.h"
#include "esp_log.h"
#include "driver/gpio.h" //todo该头文件是esp_driver_gpio组件提供的API的一部分 ，要声明组件依赖于esp_driver_gpio，则添加REQUIRES esp_driver_gpio在cmakelist中
#include "button.h"
#include "led.h"
#include "ens210.h"
#include "id.h"
#include "ntc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "RS485_master.h"
#include "RJ12.h"

#define TAG   "啦啦啦"

//*任务函数必须永不返回，如果确实需要终止任务，则需要调用vTASKDelete（）；

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

// RJ12任务
#define RJ_TASK_DEPTH 4096
#define RJ_TASK_PRIOIRTY 1

// LED灯效任务
#define LED_TASK_DEPTH 4096
#define LED_TASK_PRIOIRTY 1

// MB master任务
#define MB_MASTER_TASK_DEPTH 4096
#define MB_MASTER_TASK_PRIOIRTY 1


led_mode_t led_mode =LED_OFF;
bool led_state = false;
bool is_double_pressing = false;//双击按下标志
bool is_pressing = false;//单击按下标志

// // 按键任务入口函数
// void btn_run_task(void *param)
// {
//     static bool led_state = false; // 灯的开关状态
//     TickType_t last_tick = 0;

//     while (1)
//     {
//         // 等待中断通知
//         ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

//         TickType_t now_tick = xTaskGetTickCount();
//         if (now_tick - last_tick > pdMS_TO_TICKS(50)) // 50ms防抖
//         {
//             // 每次按键释放时，切换灯的状态
//             led_state = !led_state;

//             if (led_state)
//             {
//                 ESP_LOGE(TAG, "开灯");
//                 Rgb_led_flashing(); // 开灯
//             }
//             else
//             {
//                 ESP_LOGE(TAG, "关灯");
//                 turn_of_light(); // 关灯
//             }
//             last_tick = now_tick;
//         }
//     }
// }

// 按键任务入口函数
void btn_run_task(void *param)
{
    TickType_t press_tick = 0;
    TickType_t last_tick = 0;
    const TickType_t long_press = pdMS_TO_TICKS(500);//500ms长按
    const TickType_t double_click = pdMS_TO_TICKS(400);//300ms双击
    
    while (1)
    {
        uint32_t notify_val = 0;
        //! 等待任务通知，每个任务都有一个“通知值数组”，该函数让任务阻塞，直到指定索引的通知处于“待处理”状态
        //!参数:uxIndexToWaitOn--要等待的通知值索引
        //!     ulBitsToClearOnEntry--在通知前，清除通知值中指定的位，例如0xFFFFFFFF将其全部清零
        //!     ulBitsToClearOnExit--在函数退出前，清除通知值中指定的位
        //!     pulNotificationValue--输出参数，用于获取通知值
        //!     xTicksToWait--最长阻塞时间
        xTaskGenericNotifyWait(0x00,0xFFFFFFFF,0xFFFFFFFF,&notify_val,portMAX_DELAY);
 
        TickType_t now_tick = xTaskGetTickCount();//获取当前系统节拍计数

        if (notify_val == 0x01) // 按键按下
        {
            press_tick = now_tick;
            is_pressing = true;
            is_double_pressing = false;
        }
        else if (notify_val == 0x02)//按键松开
        {
            if (! is_pressing)
                continue;
            is_pressing = false;

            TickType_t press_duration = now_tick - press_tick;//按下的时长

            if(press_duration >= long_press)//长按
            {
                //ESP_LOGE(TAG,"长按");
                led_state = true;
                led_mode= LED_FLASH;
            }
            else if((now_tick - last_tick) < double_click)//双击
            {
                //ESP_LOGE(TAG,"双击");
                is_double_pressing = true;
                led_state = true;
                led_mode = LED_LIGHT;
                if(single_click_timer != NULL && esp_timer_is_active(single_click_timer))
                {
                    esp_timer_stop(single_click_timer);
                    esp_timer_delete(single_click_timer);
                    single_click_timer = NULL;
                }
                last_tick = 0;
                is_double_pressing = false;
            }else//短按
            {
                last_tick =now_tick;
                start_single_timer();//100ms内无第二次按下才触发单击
            }
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
    //ESP_LOGE("ENS Sensor  ","UID: %x%x%x%X%x%x%x%x", id_buf[0], id_buf[1], id_buf[2], id_buf[3], id_buf[4], id_buf[5], id_buf[6], id_buf[7]);
    ESP_LOGE("ENS 传感器  ","UID: %x%x%x%X%x%x%x%x", id_buf[0], id_buf[1], id_buf[2], id_buf[3], id_buf[4], id_buf[5], id_buf[6], id_buf[7]);
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

//热敏电阻任务入口函数
void ntc_task(void *param)
{
    while (1)
    {
        float ret = get_resitance();
        //ESP_LOGE("NTC      ","Resistance: %.4fΩ",ret);
        ESP_LOGE("NTC 热敏电阻","电阻值: %.4fΩ",ret);
        ret = get_temperature(ret);
        //ESP_LOGE("NTC      ","Temperature: %.2f℃",ret);
        ESP_LOGE("NTC 热敏电阻","温度: %.2f℃",ret);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

//RJ12任务入口函数
void rj_task(void *param)
{
    while(1)
    {
        int ret = high_gpio_level();
        if(ret == 1)
        {
            b_led();
            vTaskDelay(pdMS_TO_TICKS(100));
            rgb_led_off();
            ESP_LOGE("RJ12 ",      "RST 高" );
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

        ret = low_gpio_level();
        if(ret == 0)
        {
            g_led();
            vTaskDelay(pdMS_TO_TICKS(100));
            rgb_led_off();
            ESP_LOGE("RJ12 ",      "RST 低");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

//led灯效任务
void led_task(void *param)
{
    while(1)
    {
        switch (led_mode)
        {
        case LED_FLASH:
            rgb_led_flash();
            break;
        case LED_LIGHT:
            rgb_led_light();
            break;
        case LED_OFF:
            rgb_led_off();
            break;
        case LED_ON:
            rgb_led_on();
            break;
        default:
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

//Modbus master 任务入口函数
void mb_master(void *param)
{
    master_init();//modbus master 初始化
    vTaskDelay(pdMS_TO_TICKS(5));
    while(1)
    {
        master_operation_func(NULL);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


// 开始任务函数入口
void start_task(void *param)
{
    // 按键任务
    xTaskCreate(btn_run_task, "button task", BTN_TASK_DEPTH, NULL, BTN_TASK_PRIOIRTY, &BtnTask_handler);

    //LED灯效任务
    xTaskCreate(led_task,"led task",LED_TASK_DEPTH,NULL,LED_TASK_PRIOIRTY,NULL);

    // ID 任务
    xTaskCreate(id_task, "id task", ID_TASK_DEPTH, NULL, ID_TASK_PRIOIRTY, NULL);

    // 温湿度任务
    xTaskCreate(ens_task, "ens task", ENS_TASK_DEPTH, NULL, ENS_TASK_PRIOIRTY, NULL);

    // 热敏电阻任务
    xTaskCreate(ntc_task, "ntc task", NTC_TASK_DEPTH, NULL, NTC_TASK_PRIOIRTY, NULL);

    // Modbus master 任务
    xTaskCreate(mb_master,"modbus master task",MB_MASTER_TASK_DEPTH,NULL,MB_MASTER_TASK_PRIOIRTY,NULL);

    //RJ12任务
    xTaskCreate(rj_task," RJ12 task",RJ_TASK_DEPTH,NULL,RJ_TASK_PRIOIRTY,NULL);


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