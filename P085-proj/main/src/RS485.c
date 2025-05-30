#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"
#include "esp_log.h"

#define TAG   "难搞"

#define UART_NUM        UART_NUM_1//串口1
#define RS_BAUD_RATE    115200//波特率
#define RS_DATA_BIT     UART_DATA_8_BITS//数据位
#define RS_STOP_BIT     UART_STOP_BITS_1//停止位
#define RS_PARITY       UART_PARITY_DISABLE//奇偶校验位
#define RS_SOURCE_CLK   UART_SCLK_XTAL//时钟源

#define RS_RX_IO        GPIO_NUM_17//RX
#define RS_TX_IO        GPIO_NUM_16//TX
#define RS_RTC_IO       GPIO_NUM_23//数据收发方向

void rs485_uart_init(void)
{
    //串口通信参数配置及初始化
    const int uart_buffer_size = (1024*2);
    QueueHandle_t uart_queue;

    uart_config_t rs_cfg = {
        .baud_rate      = RS_BAUD_RATE,
        .data_bits      = RS_DATA_BIT,
        .stop_bits      = RS_STOP_BIT,
        .parity         = RS_PARITY,
        .source_clk     = RS_SOURCE_CLK,
        .flow_ctrl      = UART_HW_FLOWCTRL_DISABLE
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM,&rs_cfg));

    //配置gpio引脚
    uart_set_pin(UART_NUM,RS_TX_IO,RS_RX_IO,RS_RTC_IO,RS_RTC_IO);

    //安装UART驱动程序
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM,uart_buffer_size,uart_buffer_size,10,&uart_queue,0));
}

esp_err_t RS_transmit(void)
{
    ESP_LOGI(TAG,"开始发送数据");
    char *test = "123456\n";
    
    return uart_write_bytes(UART_NUM,(const char*)test,strlen(test));
}