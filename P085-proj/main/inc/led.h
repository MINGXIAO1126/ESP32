#ifndef __LED_H_
#define __LED_H_

void led_init(void);//led初始化
void rgb_led_on(void);//开灯
void rgb_led_off(void);//关灯
void rgb_led_flash(void);//流水灯
void rgb_led_light(void);//红绿蓝三色灯

void b_led(void);
void g_led(void);

#endif
