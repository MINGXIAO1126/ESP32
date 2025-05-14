#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_R GPIO_NUM_14       //PWM_R_LED-IO14
#define LED_G GPIO_NUM_15       //PWM_G_LED-IO15
#define LED_B GPIO_NUM_8        //PWM_B_LED-IO8

// led GPIO初始化
void led_init(void)
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1 << LED_R) | (1 << LED_G) | (1 << LED_B),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
   
    gpio_config(&led_cfg);
}

//红绿蓝三色循环闪烁
void Rgb_led_flashing(void)
{
    int led_level = 0;
    const char gpio_pins[]={LED_R,LED_G,LED_B};
    while(1)
    {
        for (int i=0;i<3;i++)
        {
            for (int j=0;j<3;j++)
            {
                gpio_set_level(gpio_pins[j],led_level);
            }
            led_level = !led_level;//翻转电平
            gpio_set_level(gpio_pins[i],led_level);
            vTaskDelay(pdMS_TO_TICKS(2000));//延时2000毫秒
            led_level = !led_level;//翻转电平
        }
    }
}

//关灯
void turn_of_light(void)
{
    int turn_off =0;
    const char gpio_pins[]={LED_R,LED_G,LED_B};
    while(1)
    {
        for(int i=0;i<3;i++)
        {
            gpio_set_level(gpio_pins[i],turn_off);
        }
    }
}