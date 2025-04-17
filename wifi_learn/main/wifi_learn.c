#include <stdio.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "string.h"

#define SSID     "esp_yizhihuyao"
#define PASSWORD "123456"
#define TAG      "sta"

//事件回调函数
void wifi_event_handle(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data)
{
    if(event_base == WIFI_EVENT)
    {
        switch(event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
            break;

            case WIFI_EVENT_STA_CONNECTED:
                //ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO,TAG,"WiFi已连接");
                printf("wifi已连接\n");
            break;

            case WIFI_EVENT_STA_DISCONNECTED:
                esp_wifi_connect();
            break;
            default:break;
        }
    }
    else if(event_base == IP_EVENT)
    {
        switch(event_id)
        {
            case IP_EVENT_STA_GOT_IP:
                //ESP_LOGI(TAG,"esp32 get ip addres");
            break;
        }
    }

}

void app_main(void)
{
    //nvs初始化
    nvs_flash_init();

    //初始化TCP/IP协议栈
    esp_netif_init();

    //初始化创建事件循环
    esp_event_loop_create_default();

    //初始化创建WiFista
    esp_netif_create_default_wifi_sta();

    //WiFi初始化
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    //注册WiFi事件和网络事件
    esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event_handle,NULL);
    esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,wifi_event_handle,NULL);

    wifi_config_t wifi_cfg = {
        .sta.threshold.authmode = WIFI_AUTH_WPA2_PSK,//设置WiFi加密方式
        .sta.pmf_cfg.capable    = true,//启用保护管理帧
        .sta.pmf_cfg.required   = false,//是否只和有保护管理帧的设备通信
        .sta.ssid               = SSID,//设置WiFi的名称
        .sta.password           = PASSWORD//设置WiFi的密码
    };

    //设置WiFi名称和密码
    //memset(&wifi_cfg.sta.ssid,0,sizeof(wifi_cfg.sta.ssid));
    //memcpy(wifi_cfg.sta.ssid,SSID,strlen(SSID));

    //memset(&wifi_cfg.sta.password,0,sizeof(wifi_cfg.sta.password));
    //memcpy(wifi_cfg.sta.password,PASSWORD,strlen(wifi_cfg.sta.password));

    //设置WiFi工作模式为STA
    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_set_config(WIFI_IF_STA,&wifi_cfg);

    //启动WiFi
    esp_wifi_start();

    return;
}
