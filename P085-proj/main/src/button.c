#include "driver/gpio.h"
#include "button.h"

TaskHandle_t BtnTask_handler = NULL;
// #define DEBOUNCE_TIME_MS 50  // 防抖时间50ms
// static uint32_t last_trigger_tick = 0;

// 按键中断回调函数
void btn_isr_callback(void *arg)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

    int level = gpio_get_level(BTN);

    if (level == 1) // 松开时任务通知（松开：高电平）
    {
        //! 该函数是用于在中断服务函数（ISR）中给任务发送通知的函数，通知发送到任务的通知数组中的指定索引（index）
        //! 参数： xTaskToNotify-- 被通知任务的句柄，通常通过调用xTaskCreate（）创建任务时返回，也可以通过调用xTaskGetCurrentTaskHandle()获取当前正在运行任务的句柄
        //!       uxIndexToNotify--指定目标任务中通知值数组的索引，即将要发送通知的具体位置。
        //!       pxHigherPriorityTaskWoken--当通过 vTaskNotifyGiveFromISR()向任务发送通知时，如果该通知导致被通知任务从阻塞态切换为就绪态，并且该任务的优先级高于当前正在运行的任务，
        //!       那么：vTaskNotifyGiveFromISR()会将pxHigherPriorityTaskWoken设置为pdTRUE。如果被设置为pdTRUE，则应在中断退出前请求一次上下文切换（使高优先级任务能尽快得到调度）
        vTaskNotifyGiveFromISR(BtnTask_handler, &pxHigherPriorityTaskWoken);
    }

    //! 作用是在中断服务函数（ISR）退出前，根据抢占条件，强制触发一次任务切换，仅在ISR中使用
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

// 按键初始化
void btn_init(void)
{
    gpio_config_t btn_cfg = {
        .pin_bit_mask = 1 << BTN,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE // 任意触发中断
    };

    gpio_config(&btn_cfg);

    gpio_install_isr_service(0);                       // 注册中断服务
    gpio_isr_handler_add(BTN, btn_isr_callback, NULL); // 绑定中断回调函数
}