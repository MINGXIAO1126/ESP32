#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_R GPIO_NUM_14 // PWM_R_LED-IO14
#define LED_G GPIO_NUM_15 // PWM_G_LED-IO15
#define LED_B GPIO_NUM_8  // PWM_B_LED-IO8

//灯的颜色
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
}rgb_color_t;

//设置灯的颜色
void set_rgb_led(rgb_color_t color)
{
    gpio_set_level(LED_R,color.r);
    gpio_set_level(LED_G,color.g);
    gpio_set_level(LED_B,color.b);
}


// led GPIO初始化
void led_init(void)
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1 << LED_R) | (1 << LED_G) | (1 << LED_B),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&led_cfg);
}

// 亮灯
void Rgb_led_flashing(void)
{
    rgb_color_t rgb ={0,1,1};
    set_rgb_led(rgb);
}

// 关灯
void turn_of_light(void)
{
    rgb_color_t rgb = {0,0,0};
    set_rgb_led(rgb);
}