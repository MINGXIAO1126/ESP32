#include "driver/gpio.h"
#include "button.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "led.h"
#include "P085_drv.h"

TaskHandle_t BtnTask_handler = NULL;
esp_timer_handle_t single_click_timer = NULL;

#define TAG  "按键"

// // 按键中断回调函数
// void btn_isr_callback(void *arg)
// {
//     BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

//     int level = gpio_get_level(BTN);

//     if (level == 1) // 松开时任务通知（松开：高电平）
//     {
//         //! 该函数是用于在中断服务函数（ISR）中给任务发送通知的函数，通知发送到任务的通知数组中的指定索引（index）
//         //! 参数： xTaskToNotify-- 被通知任务的句柄，通常通过调用xTaskCreate（）创建任务时返回，也可以通过调用xTaskGetCurrentTaskHandle()获取当前正在运行任务的句柄
//         //!       uxIndexToNotify--指定目标任务中通知值数组的索引，即将要发送通知的具体位置。
//         //!       pxHigherPriorityTaskWoken--当通过 vTaskNotifyGiveFromISR()向任务发送通知时，如果该通知导致被通知任务从阻塞态切换为就绪态，并且该任务的优先级高于当前正在运行的任务，
//         //!       那么：vTaskNotifyGiveFromISR()会将pxHigherPriorityTaskWoken设置为pdTRUE。如果被设置为pdTRUE，则应在中断退出前请求一次上下文切换（使高优先级任务能尽快得到调度）
//         vTaskNotifyGiveFromISR(BtnTask_handler, &pxHigherPriorityTaskWoken);
//     }
//     //! 作用是在中断服务函数（ISR）退出前，根据抢占条件，强制触发一次任务切换，仅在ISR中使用
//     portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
// }


// 按键中断回调函数
void btn_isr_callback(void *arg)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

    int level = gpio_get_level(BTN);

    uint32_t notify_val = (level == 0 ) ? 0x01 : 0x02;//0x01 表示按下，0 x02 表示松开

        //! 该函数是用于在中断服务函数（ISR）中给任务发送通知的函数，通知发送到任务的通知数组中的指定索引（index）
        //! 参数： xTaskToNotify-- 被通知任务的句柄，通常通过调用xTaskCreate（）创建任务时返回，也可以通过调用xTaskGetCurrentTaskHandle()获取当前正在运行任务的句柄
        //!       ulValue--通知值，具体含义取决于eAction
        //!       eAction--告知通知值的处理方式，eAction的值：eSetBits，将通知值按位或上ulValue  eIncrement，通知值自增1，忽略ulValue  eSetValueWithOverwrite--覆盖通知值为ulvalue，不管是否有未处理通知
        //!       pxHigherPriorityTaskWoken--用于通知后是否引发上下文的切换，值为pdFALSE时，若该通知唤醒了一个比当前中断任务优先级更高的任务，则需要手动切换上下文
        xTaskNotifyFromISR(BtnTask_handler,notify_val,eSetValueWithOverwrite,&pxHigherPriorityTaskWoken);

    //! 作用是在中断服务函数（ISR）退出前，根据抢占条件，强制触发一次任务切换，仅在ISR中使用
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

//定时器回调函数
void single_click_timer_cb(void *arg)
{
    // if(is_double_pressing)
    // {
    //     //双击已经处理过了，不在响应单击
    //     is_double_pressing = false;
    //     return;
    // }

    //ESP_LOGE(TAG,"短按");
    if(!led_state)
    {
        //开灯
        led_state = true;
        led_mode = LED_ON;
    }else
    {
        //关灯
        led_state = false;
        led_mode = LED_OFF;
    }
}

//按键单击
void start_single_timer(void)
{
    if(single_click_timer == NULL)
    {
        //软件定时配置
        static esp_timer_create_args_t timer_arg={
        .callback  = &single_click_timer_cb,
        .name      = "single cilck timer"
        };

        //创建定时器
        esp_timer_create(&timer_arg,&single_click_timer);
    }

    //启动一次性定时器
    esp_timer_start_once(single_click_timer,30*1000);
}


// 按键初始化
void btn_init(void)
{
    gpio_config_t btn_cfg = {
        .pin_bit_mask = 1 << BTN,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE // 任意边沿触发中断
    };

    gpio_config(&btn_cfg);

    gpio_install_isr_service(0);                       // 注册中断服务
    gpio_isr_handler_add(BTN, btn_isr_callback, NULL); // 绑定中断回调函数
}


