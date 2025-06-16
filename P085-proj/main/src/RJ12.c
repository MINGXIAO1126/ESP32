#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#define RJ_GPIO_RST_MODE    GPIO_MODE_OUTPUT//输出模式(RST)  
#define RJ_GPIO_RS_MODE     GPIO_MODE_INPUT//输入模式（RS）
#define RJ_GPIO_RST         GPIO_NUM_20//RST_UART_C引脚
#define RJ_GPIO_RS          GPIO_NUM_19//RS_UART_C引脚

//RJ12引脚初始化
void rj_init(void)
{
    //RST引脚配置
    gpio_config_t rj_rst_cfg={
        .mode           = RJ_GPIO_RST_MODE,
        .pin_bit_mask   = 1<< RJ_GPIO_RST,
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .pull_up_en     = GPIO_PULLUP_DISABLE
    };

    //RS引脚配置
    gpio_config_t rj_rs_cfg={
        .mode           = RJ_GPIO_RS_MODE,
        .pin_bit_mask   = 1<< RJ_GPIO_RS,
        .pull_down_en   = GPIO_PULLDOWN_ENABLE,
        .pull_up_en     = GPIO_PULLUP_DISABLE
    };

    gpio_config(&rj_rst_cfg);
    gpio_config(&rj_rs_cfg);
}

int high_gpio_level(void)
{
    gpio_set_level(RJ_GPIO_RST,1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    int ret = gpio_get_level(RJ_GPIO_RS);
    
    return ret;
}

int low_gpio_level(void)
{
    gpio_set_level(RJ_GPIO_RST,0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    int ret = gpio_get_level(RJ_GPIO_RS);

    return ret;
}

