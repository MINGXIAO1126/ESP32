// #include "driver/uart.h"
// #include "driver/gpio.h"
// #include "RS485.h"
// #include "esp_log.h"
// #include "rom/ets_sys.h"

// #define RS_BAUD_RATE  115200//波特率
// #define RS_DATA_BIT   UART_DATA_8_BITS//数据位
// #define RS_FLOW_CTRL  UART_HW_FLOWCTRL_DISABLE//硬件流控
// #define RS_PARITY     UART_PARITY_DISABLE//校验位
// #define RS_STOP_BIT   UART_STOP_BITS_1//停止位

// #define RS_UART_TX          GPIO_NUM_16//TX引脚
// #define RS_UART_RX          GPIO_NUM_17//RX引脚
// #define RS_UART_RTS         GPIO_NUM_23//RTS引脚
// #define RS_TX_BUFFER_SIZE   1024//TX环形缓冲区大小
// #define RS_RX_BUFFER_SIZE   1024//RX环形缓冲区大小

// //计算SSP485芯片发送数据所需时间
// #define UART_TIMER_US(baud)  ((uint32_t)((10.0 * 1.2 * 1000000.0)/(baud)))

// //RS485通信初始化
// void rs_uart_init(void)
// {
//     //设置通信参数
//     uart_config_t rs_uart_cfg={
//         .baud_rate      = RS_BAUD_RATE,
//         .data_bits      = RS_DATA_BIT,
//         .flow_ctrl      = RS_FLOW_CTRL,
//         .parity         = RS_PARITY,
//         .stop_bits      = RS_STOP_BIT,
//         .source_clk     = UART_SCLK_DEFAULT
//     };

//     gpio_config_t rs_ctr_cfg={
//         .mode           = GPIO_MODE_OUTPUT,
//         .pin_bit_mask   = 1<<GPIO_NUM_23,
//         .pull_down_en   = GPIO_PULLDOWN_DISABLE,
//         .pull_up_en     = GPIO_PULLUP_DISABLE
//     };
//     gpio_config(&rs_ctr_cfg);

//     //安装驱动程序
//     ESP_ERROR_CHECK(uart_driver_install(RS_UART_NUM,RS_RX_BUFFER_SIZE,RS_TX_BUFFER_SIZE,0,NULL,0));

//     ESP_ERROR_CHECK(uart_param_config(RS_UART_NUM,&rs_uart_cfg));

//     //设置通信管脚
//     ESP_ERROR_CHECK(uart_set_pin(RS_UART_NUM,RS_UART_TX,RS_UART_RX,UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE));

//     //设置UART通信模式
//     ESP_ERROR_CHECK(uart_set_mode(RS_UART_NUM,UART_MODE_UART));

//     //设置RTS低电平（默认进入接收状态）
//     //uart_set_rts(RS_UART_NUM,1);
//     gpio_set_level(GPIO_NUM_23,0);
// }

// //根据波特率进行一定的延时，保证SSP485数据正常发送完成
// static void rs_uart_delay(uint32_t baud)
// {
//     uint32_t delay_us = UART_TIMER_US(baud);
//     ets_delay_us(delay_us);
// }

// //发送数据
// void rs_send_data(const char *str,size_t len)
// {
//     //设置RTS高电平，进入发送状态
//     //uart_set_rts(RS_UART_NUM,0);
//     gpio_set_level(GPIO_NUM_23,1);

//     ets_delay_us(2);//延时2us

//     //发送数据
//     uart_write_bytes(RS_UART_NUM,str,len);

//     //等待发送完成
//     uart_wait_tx_done(RS_UART_NUM,pdMS_TO_TICKS(10));

//     //rs_uart_delay(RS_BAUD_RATE);
//     ets_delay_us(500);//延时2us

//     //拉低RTS电平，回到接收状态
//     //uart_set_rts(RS_UART_NUM,1);
//     gpio_set_level(GPIO_NUM_23,0);
// }