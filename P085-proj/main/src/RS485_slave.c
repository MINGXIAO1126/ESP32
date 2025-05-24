#include "mbcontroller.h"
#include "modbus_params.h"
#include "driver/gpio.h"

#define  TAG   "Modbus 从站"

#define MB_SLAVE_ADDR         1//从站设备地址
#define MB_PORT_NUM           UART_NUM_1//串口编号
#define MB_BAUDRATE           115200//波特率
#define TX_IO_NUM        GPIO_NUM_16
#define RX_IO_NUM        GPIO_NUM_17
#define CTR_IO_NUM       GPIO_NUM_23

#define HOLD_OFFSET(field) ((uint16_t)(offsetof(holding_reg_params_t, field) >> 1))
#define MB_SLAVE_REG_TYPE     MB_PARAM_HOLDING
#define MB_REG_HOLDING_START_AREA0          (HOLD_OFFSET(holding_data0))

static void setup_reg_data(void)
{
    holding_reg_params.holding_data0 = 1.33;
    holding_reg_params.holding_data1 = 2.55;
    holding_reg_params.holding_data2 = 3.77;
    holding_reg_params.holding_data3 = 4.99;

    coil_reg_params.coils_port0 = 0x55;
    coil_reg_params.coils_port1 = 0xAA;
}

void Modbus_slave_init(void)
{
    //初始化从站控制器
    void* mbc_slave_handler = NULL;
    ESP_ERROR_CHECK(mbc_slave_init(MB_PORT_SERIAL_SLAVE,mbc_slave_handler));

    //从站通信参数设置
    mb_communication_info_t comm_info ={
        .mode       = MB_MODE_RTU,
        .slave_addr = MB_SLAVE_ADDR,
        .port       = MB_PORT_NUM,
        .baudrate   = MB_BAUDRATE,
        .parity     = UART_PARITY_DISABLE
    };
    ESP_ERROR_CHECK(mbc_slave_setup((void *)&comm_info));

    //从站寄存器区域描述符
    mb_register_area_descriptor_t reg_area ={
        .type         = MB_SLAVE_REG_TYPE,
        .start_offset = MB_REG_HOLDING_START_AREA0,
        .address      = (void*)&holding_reg_params.holding_data0,
        .size         = (size_t)(HOLD_OFFSET(holding_data4) - HOLD_OFFSET(test_regs))
    };
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    setup_reg_data();//设置从站寄存器初始值

    ESP_ERROR_CHECK(mbc_slave_start());//启动从站控制器

    //设置用于Modbus通信的UART端口的引脚
    uart_set_pin(MB_PORT_NUM,TX_IO_NUM,RX_IO_NUM,CTR_IO_NUM,UART_PIN_NO_CHANGE);

    //设置通信模式
    uart_set_mode(MB_PORT_NUM,UART_MODE_RS485_HALF_DUPLEX);

    ESP_LOGE(TAG,"从站初始化完成");
}