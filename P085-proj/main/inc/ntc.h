#ifndef __NTC_H_
#define __NTC_H_

#include "stdint.h"
#include "esp_adc/adc_continuous.h"

void ntc_adc_init(void);//ADC初始化
// float get_voltage (uint8_t *read_buf,uint32_t buf_len);//读取电压值

float get_temperature(float V_out);//计算温度值
float get_resitance();//获取电阻值

#endif