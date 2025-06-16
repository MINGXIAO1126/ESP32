/*
基于esp-idf的ENS210温湿度传感器驱动
需要使能i2c相关驱动：
idf.py menuconfig
→ Component Config
  → Driver configurations
    → Enable I2C Master Driver

*/
#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#define TAG "ENS 传感器  "

#define I2C_PORT        I2C_NUM_0      // i2c端口
#define I2C_SCL         GPIO_NUM_7     // SCL-IO7
#define I2C_SDA         GPIO_NUM_1     // SDA-IO1
#define I2C_GLITH_CNT   7              // 毛刺周期
i2c_master_bus_handle_t bus_handle;    // I2C总线句柄
i2c_master_dev_handle_t dev_handle;    // i2c设备句柄

#define I2C_SLAVE_ADDR_LEN      I2C_ADDR_BIT_LEN_7     // 7位地址长度
#define I2C_SLAVE_ADDR          0x43                   // ENS210的地址
#define I2C_SCL_SPEED           100000                 // 时钟频率

// ENS210的寄存器地址
#define ENS210_REG_PART_ID      0x00   // 将部件标识为ENS210，2字节，位0:15，默认0x0210
#define ENS210_REG_UID          0x04   // 唯一标识，读取唯一设备ID，8字节
#define ENS210_REG_SYS_CTRL     0x10   // 系统控制寄存器，1字节，位7：写1重置设备，默认为0；位0：控制自动低功耗，0：禁用，设备保持活动状态，1：启用，测量完成后设备进入待机状态
#define ENS210_REG_SYS_STAT     0x11   // 系统状态寄存器，1字节，位0：系统电源状态，默认为0b1，0：系统处于待机状态或启动状态，1：活动状态
#define ENS210_REG_SENS_RUN     0x21   // 配置传感器的运行模式（单发或连续），1字节，位0：温度传感器的运行模式，默认0b0，0：单词模式运行，1：连续模式运行；位1：相对温度传感器的运行模式，默认0b0，0：单次模式，1：连续模式
#define ENS210_REG_SENS_START   0x22   // 开始测量，1字节，位0：默认0b0，写入1开始温度传感器测量，位1：默认0b0，写入1开始相对湿度传感器测量
#define ENS210_REG_SENS_STOP    0x23   // 停止测量，1字节，位0：默认0b0，写入1停止连续温度传感器测量，位1：默认0b0，写入1停止连续相对传感器测量
#define ENS210_REG_SENS_STAT    0x24   // 指示传感器的测量状态（空闲或活动），1字节，位0：默认0b0，指示温度传感器的测量状态，0：温度传感器空闲，1：正在主动测量；位1：默认0b0，指示相对湿度传感器的测量状态，0：相对湿度传感器空闲，1：正在主动测量
#define ENS210_REG_T_VAL        0x30   // 保存测量的温度数据，3字节，位0:15(T_DATA)，最后测得的温度，存储为小端序16位无符号值，以1/64Kelvin为单位，位16：数据有效指示位（1表示T_DATA有效），位17:23：循环冗余校验（CRC）
#define ENS210_REG_H_VAL        0x33   // 保存测量的相对湿度数据，3字节，位0:(H_DATA),最后测得的相对湿度，以1/512%RH为单位存储为小端序16位无符号值，位16：数据有效指示位（1表示H_DATA有效），位17:23：循环冗余校验（CRC）

// I2C初始化
void i2c_master_init(void)
{
    // I2C 主机总线配置
    i2c_master_bus_config_t i2c_master_cfg = {
        .clk_source                   = I2C_CLK_SRC_DEFAULT, // 选择XTAL作为默认时钟
        .i2c_port                     = I2C_PORT,
        .scl_io_num                   = I2C_SCL,
        .sda_io_num                   = I2C_SDA,
        .glitch_ignore_cnt            = I2C_GLITH_CNT,
        .flags.enable_internal_pullup = true               // 启用内部上拉电阻
    };

    // 分配和初始化I2C主机总线
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_cfg,&bus_handle));

    // I2C主机设备配置
    i2c_device_config_t i2c_device_cfg = {
        .dev_addr_length   = I2C_SLAVE_ADDR_LEN,
        .device_address    = I2C_SLAVE_ADDR,
        .scl_speed_hz      = I2C_SCL_SPEED
    };

    // 分配I2C实例，并将设备挂载到主机总线上
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &i2c_device_cfg,&dev_handle));

    vTaskDelay(pdMS_TO_TICKS(100));

    //检测从设备是否存在
    esp_err_t ret = i2c_master_probe(bus_handle, I2C_SLAVE_ADDR, 100);
    if (ret != ESP_OK)
    {
        //ESP_LOGE(TAG, "Not Found Device: %s", esp_err_to_name(ret));
        ESP_LOGE(TAG, "没有发现设备: %s", esp_err_to_name(ret));
    }
    else
    {
        //ESP_LOGE(TAG, "Device Addr: 0x%02X", I2C_SLAVE_ADDR);
        ESP_LOGE(TAG, "设备地址: 0x%02X", I2C_SLAVE_ADDR);
    }
}

//读寄存器操作
esp_err_t ens_read_register(uint8_t reg_addr,uint8_t *read_buffer)
{
    return i2c_master_transmit_receive(dev_handle,&reg_addr,1,read_buffer,sizeof(read_buffer),-1);
}

//写寄存器操作
esp_err_t ens_write_register(uint8_t reg_addr,uint8_t data)
{
    uint8_t write_buffer[2] = {reg_addr,data};
    //! 向从机设备写入数据
    //! 参数：i2c_dev--由i2c_master_bus_add_device（）创建并返回的设备句柄，代表某个具体i2c从设备
    //!       write_buffer--要发送的数据缓冲区，指要发送到i2c从设备的数据数组指针
    //!       write_size--发送的数据长度，以字节为单位
    //!       xfer_timeout_ms--超时时间，指定本次数据传输的超时时间，传输过程中若超时，则返回ESP_ERR_TIMEOUT，设为-1，表示一直等待到完成，永不超时
    return i2c_master_transmit(dev_handle,write_buffer,sizeof(write_buffer),-1);
}

//获取设备标识
esp_err_t ens_get_part_id(void)
{
    uint8_t reg_addr = ENS210_REG_PART_ID;// 寄存器地址
    uint8_t read_buffer = 0;
    ens_read_register(reg_addr,&read_buffer);

    return read_buffer;
}

// 获取UID
esp_err_t ens_get_uid(uint8_t *read_buffer)
{
    uint8_t reg_addr = ENS210_REG_UID; // 寄存器地址
    
    return ens_read_register(reg_addr,read_buffer);
}

// 系统控制
// value的值：0b10000000--重置设备，0b0--禁用低功耗
esp_err_t ens_sys_ctrl(void)
{
    uint8_t reg_addr= ENS210_REG_SYS_CTRL; // 寄存器地址+写入的数据
    uint8_t value = 0b0;

    return ens_write_register(reg_addr,value);
}

// 开始测量
// value的值：0b1--开始温度传感器测量，0b10--开始相对湿度传感器测量
esp_err_t ens_sens_start(void)
{
    uint8_t reg_addr= ENS210_REG_SENS_START; // 寄存器地址
    uint8_t value = 0b11;

    return ens_write_register(reg_addr,value);
}

//停止测量
//value的值：0b1--停止温度传感器测量，0b10--停止相对湿度传感器测量
esp_err_t ens_sens_stop(void)
{
    uint8_t reg_addr= ENS210_REG_SENS_STOP; // 寄存器地址
    uint8_t value = 0b11;

    return ens_write_register(reg_addr,value);
}

//传感器运行模式（单发或连续）
//value的值：
//温度传感器:0b0--单次模式，0b1--连续模式
//相对湿度传感器：0b00--单次模式，0b10--连续模式
esp_err_t ens_sens_run(void)
{
    uint8_t reg_addr= ENS210_REG_SENS_RUN; // 寄存器地址
    uint8_t value = 0b11;

    return ens_write_register(reg_addr,value);
}

//获取温度
esp_err_t ens_get_temperature(uint8_t *read_buffer)
{
    uint8_t reg_addr = ENS210_REG_T_VAL; // 寄存器地址

    return ens_read_register(reg_addr,read_buffer);
}

//获取相对湿度
esp_err_t ens_get_humidity(uint8_t *read_buffer)
{
    uint8_t reg_addr = ENS210_REG_H_VAL; // 寄存器地址
  
    return ens_read_register(reg_addr,read_buffer);
}

//温度转换
esp_err_t ens_temperature_conversion(uint32_t t_val)
{
    uint16_t t_data = (t_val>>0) & 0xffff;
    float TinK = (float)t_data/64;
    float TinC = TinK-273.15;

    //ESP_LOGE(TAG, "Temperature: %.2f℃", TinC);
    ESP_LOGE(TAG, "温度: %.2f℃", TinC);

    return ESP_OK;
}

//相对湿度转换
esp_err_t ens_humidity_conversion(uint32_t h_val)
{
    uint16_t h_data = (h_val>>0) & 0xffff;
    float HinRH = (float)h_data/512;

    //ESP_LOGE(TAG, "Relative Humidity: %.2f%%", HinRH);
    ESP_LOGE(TAG, "相对湿度: %.2f%%", HinRH);

    return ESP_OK;
}




