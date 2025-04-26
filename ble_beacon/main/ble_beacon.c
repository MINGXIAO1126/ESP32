#include "common.h"
#include "gap.h"


void ble_store_config_init(void);
/*
*需要先在menuconfig里面配置一下NimBLE组件（怎么知道menuconfig里面每一个选项对应的代码是在哪，就算搜到了选项对应的宏，但是也只能在sdkconfig里面看到宏定义的值）
*ble_store_config_init(),这个函数编译的时候报错，隐式声明：先声明，再引用，不太懂啊，不是在库里面已经声明定义了？
*/

//蓝牙协议栈重置回调函数
static void on_stack_reset(int reason)
{
    ESP_LOGI(TAG,"nimble 栈重置，重置原因%d",reason);
}

//蓝牙协议栈同步回调函数
static void on_tack_sync(void)
{
    adv_init();//广播操作的初始化
}

static void nimble_host_config_init(void)
{
    ble_hs_cfg.reset_cb         = on_stack_reset;
    ble_hs_cfg.sync_cb          = on_tack_sync;
    ble_hs_cfg.store_status_cb  = ble_store_util_status_rr;//存储状态回调，这里是调用了循环状态回调函数用于演示

    ble_store_config_init();//主机存储配置
}

//NimBLE主机层任务函数入口
static void nimble_host_task(void *param)
{
    ESP_LOGI(TAG,"nimble 主机层任务启动");

    nimble_port_run();

    vTaskDelete(NULL);
}

void app_main(void)
{
    //初始化NVS Flash，esp32的蓝牙协议栈使用NVS Flash存储相关配置
    esp_err_t ret = nvs_flash_init();
    if(ret ==  ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NOT_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if(ret != ESP_OK){
        ESP_LOGI(TAG,"nvs flash初始化失败了啊%d",ret);
        return;
    }

    //初始化NimBLE主机层协议栈
    esp_err_t ret1 = nimble_port_init();
    if(ret1 != ESP_OK){
        ESP_LOGI(TAG,"NimBLE主机层协议栈初始化失败了啊%d",ret1);
        return;
    }

    //初始化GAP服务，GAP定义了设备之间的连接行为，以及设备连接中所扮演的角色
    int rc = gap_init();
    if(rc != 0){
        ESP_LOGI(TAG,"GAP服务初始化失败了啊%d",rc);
        return;
    }

    //NimBLE主机层协议栈的配置
    nimble_host_config_init();

    //启动NimBLE主机层的FreeRTOS线程
    /*
    *创建NimBLE主机层任务函数
    *参数：pxTaskCode--指向任务入口函数的指针
    *pcName--任务名称（主要是方便调试）
    *usStackDepth--任务堆栈的大小（以字节数表示）
    *pvParameters--
    *uxPriority--任务优先级
    *pxCreatedTask--用于传回可引用所创建任务的句柄
    */
    xTaskCreate(nimble_host_task,"NimBLE Host",4*1024,NULL,5,NULL);
}
