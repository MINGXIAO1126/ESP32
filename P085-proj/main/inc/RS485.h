#ifndef __RS_UART_H_
#define __RS_UART_H_

#include "driver/gpio.h"

#define RS_RTC_IO       GPIO_NUM_23//数据收发方向
#define RS_TX_ON      do { gpio_set_level(RS_RTC_IO,1) ; } while(0)//发送数据
#define RS_RX_ON      do { gpio_set_level(RS_RTC_IO,0) ; } while(0)//接收数据
void rs485_uart_init(void);
esp_err_t RS_transmit(void);


#endif