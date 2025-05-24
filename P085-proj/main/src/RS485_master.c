#include "esp_err.h"
#include "mbcontroller.h"
#include "modbus_params.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define TAG   "Modbus master"

#define MB_PORT_NUM      UART_NUM_1//串口编号
#define MB_MODE          MB_MODE_RTU//协议模式（modbus协议有许多变体：modbus RTU，用于串行通信，使用二进制进行协议通信，modbus ASCII，用于串行通信，使用ASCII字符进行协议通信，modbus TCP/IP，用于通过TCP/IP网络进行通信）
#define MB_BAUDRATE      115200//波特率
#define MB_PARITY        UART_PARITY_DISABLE//校验方式
#define TX_IO_NUM        GPIO_NUM_16
#define RX_IO_NUM        GPIO_NUM_17
#define CTR_IO_NUM       GPIO_NUM_23

#define CID0                   0//参数唯一标识符
#define STR(fieldname)         ((const char*)( fieldname ))
#define MB_DEVICE_ADDR1        1//从站设备地址
#define HOLD_OFFSET(field)     ((uint16_t)(offsetof(holding_reg_params_t, field) + 1))//保持寄存器地址计算
#define OPTS(min_val, max_val, step_val) { .opt1 = min_val, .opt2 = max_val, .opt3 = step_val }//参数限制设定

#define CID_RELAY_P1            1//参数唯一标识符
#define COIL_OFFSET(field) ((uint16_t)(offsetof(coil_reg_params_t, field) + 1))//线圈寄存器映射

//设备参数描述符表
//!参数：CID--参数唯一标识符                                     Param Name--参数名称，字符串格式，便于调试和显示
//!      Units--参数单位（如Volts、C），说明数据含义              Modbus Slave Addr--目标从机地址，表明该参数属于哪个modbus从站设备
//!      Modbus Reg Type--Modbus寄存器类型，如输入寄存器（MB_PARAM_INPUT）、保持寄存器（MB_PARAM_HOLDING）
//!      Reg Start--Modbus起始寄存器地址                        Reg Size--参数占用的寄存器数量（每个寄存器2字节）
//!      Instance offset--该参数在程序内数据结构中的偏移量，用于定位数据存储位置
//!      Data Type--参数数据类型（如PARAM_TYPE_FLOAT），用于数据转换
//!      Data Size--数据大小，单位为字节                        Parameter Options--参数处理选项，比如数值上下限、步进或位掩码
//!      Access Mode--访问权限（只读、读写、触发），控制参数的访问和操作规则
const mb_parameter_descriptor_t device_parameters[] ={
    {CID0, STR("古月方源"),STR("C"),MB_DEVICE_ADDR1,MB_PARAM_HOLDING,0,2,
            HOLD_OFFSET(holding_data0),PARAM_TYPE_U16,4,OPTS(0,50,1),PAR_PERMS_READ_WRITE_TRIGGER},
    { CID_RELAY_P1, STR("RelayP1"), STR("on/off"), MB_DEVICE_ADDR1, MB_PARAM_COIL, 2, 6,
            COIL_OFFSET(coils_port0), PARAM_TYPE_U8, 1, OPTS( 0xAA, 0x15, 0 ), PAR_PERMS_READ_WRITE_TRIGGER }
};

const uint16_t num_device_parameters = (sizeof(device_parameters)/sizeof(device_parameters[0]));

static esp_err_t master_init(void)
{
    //配置modbus控制器
    mb_communication_info_t comm = {
        .port      = MB_PORT_NUM,
        .mode      = MB_MODE,
        .baudrate  = MB_BAUDRATE,
        .parity    = MB_PARITY
    };

    //初始化modbus控制器
    void *master_handle = NULL;
    mbc_master_init(MB_PORT_SERIAL_MASTER,master_handle);

    mbc_master_setup((void *)&comm);

    //设置UART引脚
    uart_set_pin(MB_PORT_NUM,TX_IO_NUM,RX_IO_NUM,CTR_IO_NUM,UART_PIN_NO_CHANGE);

    //启动modbus主站控制器
    mbc_master_start();

    //设置UART为RS485半双工模式
    uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX);

    vTaskDelay(5);
    //设置设备寄存器描述符(设置从站设备的参数，让主站知道从站的寄存器地址、类型、数据长度，方便主站读写)
    mbc_master_set_descriptor(&device_parameters[0],num_device_parameters);

    return ESP_OK;
} 

static void* master_get_param_data(const mb_parameter_descriptor_t* param_descriptor)
{
    assert(param_descriptor != NULL);
    void* instance_ptr = NULL;
    if (param_descriptor->param_offset != 0) {
       switch(param_descriptor->mb_param_type)
       {
           case MB_PARAM_HOLDING:
               instance_ptr = ((void*)&holding_reg_params + param_descriptor->param_offset - 1);
               break;
           case MB_PARAM_INPUT:
               instance_ptr = ((void*)&input_reg_params + param_descriptor->param_offset - 1);
               break;
           case MB_PARAM_COIL:
               instance_ptr = ((void*)&coil_reg_params + param_descriptor->param_offset - 1);
               break;
           case MB_PARAM_DISCRETE:
               instance_ptr = ((void*)&discrete_reg_params + param_descriptor->param_offset - 1);
               break;
           default:
               instance_ptr = NULL;
               break;
       }
    } else {
        ESP_LOGE(TAG, "Wrong parameter offset for CID #%u", (unsigned)param_descriptor->cid);
        assert(instance_ptr != NULL);
    }
    return instance_ptr;
}