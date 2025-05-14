#include <stdio.h>
#include "P085_drv.h"
#include "esp_log.h"
#include "driver/gpio.h"//todo该头文件是esp_driver_gpio组件提供的API的一部分 ，要声明组件依赖于esp_driver_gpio，则添加REQUIRES esp_driver_gpio在cmakelist中
#include "button.h"
#include "led.h"

//*任务函数必须永不返回，如果确实需要终止任务，则需要调用vTASKDelete（）；

#define TAG "哈哈哈"
#define LONG_PRESS_TIME  pdMS_TO_TICKS(2000)//长按计时,2秒

//开始任务
#define START_TASK_DEPTH     1024//任务栈深
#define START_TASK_PRIOIRIY  1//任务优先级

//按键任务
#define BTN_TASK_DEPTH  4096//任务栈深
#define BTN_TASK_PRIOIRTY 1//任务优先级

volatile bool long_sign = false; //长按标记
TimerHandle_t long_press_timer =NULL;//长按定时器句柄

//按键任务入口函数
void btn_run_task(void *param)
{
    static int last_level =1;//初始时按键未按下
    while(1)
    {
        //等待中断通知
        //! 该函数用于等待任务通知类似于“获取一个计数信号量”，它可以让任务进入阻塞态，等待通知值不为0时被唤醒
        //! 参数：uxIndexToWaitOn--等待通知值数组下标（第几个通知槽位）
        //!       xClearCountOnExit--pdFALSE:退出函数时将通知值减1，非pdFALSE：退出时函数将通知值清零
        //!       xTicksToWait--任务在没有通知值时最多阻塞时间（tick数），可用pdMS_TO_TICKS()转换毫秒为tick
        ulTaskNotifyTake(pdFALSE,pdMS_TO_TICKS(10));

        int ret = gpio_get_level(BTN);

        //按键按下
        if(ret == 0 && last_level == 1)
        {
            if(xTimerIsTimerActive(long_press_timer) == pdFALSE)
            {
                if(xTimerStart(long_press_timer,0) != pdPASS)
                {
                    ESP_LOGI(TAG,"启动定时器失败");
                }
            }
        }

        //按键释放
        if(ret == 1 && last_level == 0)
        {
            xTimerStop(long_press_timer,0);//松手时关闭长按定时器
            if(long_sign == true)
            {
                ESP_LOGI(TAG,"长按");
                turn_of_light();//长按熄灯
                long_sign = false;//标志清零
            }
            else
            {
                ESP_LOGI(TAG,"短按");
                Rgb_led_flashing();//短按闪灯
            }
        }
        last_level = ret;//更新上次状态
    }
}

//按键长按回调
void long_press_cb(TimerHandle_t xTimer)
{
    long_sign = true;
}

//开始任务函数入口
void start_task(void *param)
{
    //创建软件定时器
    //! 软件定时器创建函数，用于创建一个定时器句柄，方便后续控制。创建时是休眠状态，需要调用xTimerStart（）才会激活。
    //! 参数：pcTimerName--定时器名称
    //!       xTimerPeriodInTicks--定时周期，单位为TICk（需要用portTICK_PERIOD_MS转换毫秒）
    //!       xAutoReload--是否重装载，pdTRUE--回调后自动重装载，再次开始，PDFALSE--回调后停止（一次性定时器）
    //!       pvTimerID--用户自定义ID（可传任意数据，在回调函数里可以识别）
    //!       pxCallbackFunction--超时后调用的回调函数
    long_press_timer = xTimerCreate("long press timer",LONG_PRESS_TIME,pdFALSE,NULL,long_press_cb);

    //按键任务
    xTaskCreate(btn_run_task,"button task",BTN_TASK_DEPTH,NULL,BTN_TASK_PRIOIRTY,&BtnTask_handler);

    //删除启动任务
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