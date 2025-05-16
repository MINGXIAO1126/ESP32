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
//#include "driver/i2c.h"

#define TAG "ENS传感器"

#define I2C_PORT      0  // i2c端口
#define I2C_SCL       GPIO_NUM_7 // SCL-IO7
#define I2C_SDA       GPIO_NUM_1 // SDA-IO1
#define I2C_GLITH_CNT 7    // 毛刺周期
i2c_master_bus_handle_t bus_handle;//I2C总线句柄
i2c_master_dev_handle_t dev_handle;//i2c设备句柄

#define I2C_SLAVE_ADDR_LEN I2C_ADDR_BIT_LEN_7 // 7位地址长度
#define I2C_SLAVE_ADDR 0x43                   // ENS210的地址
#define I2C_SCL_SPEED 100000                  // 时钟频率

// ENS210的寄存器地址
#define ENS210_REG_PART_ID      0x00    // 将部件标识为ENS210，2字节，位0:15，默认0x0210
#define ENS210_REG_UID          0x04    // 唯一标识，读取唯一设备ID，8字节
#define ENS210_REG_SYS_CTRL     0x10    // 系统控制寄存器，1字节，位7：写1重置设备，默认为0；位0：控制自动低功耗，0：禁用，设备保持活动状态，1：启用，测量完成后设备进入待机状态
#define ENS210_REG_SYS_STAT     0x11    // 系统状态寄存器，1字节，位0：系统电源状态，默认为0b1，0：系统处于待机状态或启动状态，1：活动状态
#define ENS210_REG_SENS_RUN     0x21    // 配置传感器的运行模式（单发或连续），1字节，位0：温度传感器的运行模式，默认0b0，0：单词模式运行，1：连续模式运行；位1：相对温度传感器的运行模式，默认0b0，0：单次模式，1：连续模式
#define ENS210_REG_SENS_START   0x22    // 开始测量，1字节，位0：默认0b0，写入1开始温度传感器测量，位1：默认0b0，写入1开始相对湿度传感器测量
#define ENS210_REG_SENS_STOP    0x23    // 停止测量，1字节，位0：默认0b0，写入1停止连续温度传感器测量，位1：默认0b0，写入1停止连续相对传感器测量
#define ENS210_REG_SENS_STAT    0x24    // 指示传感器的测量状态（空闲或活动），1字节，位0：默认0b0，指示温度传感器的测量状态，0：温度传感器空闲，1：正在主动测量；位1：默认0b0，指示相对湿度传感器的测量状态，0：相对湿度传感器空闲，1：正在主动测量
#define ENS210_REG_T_VAL        0x30    // 保存测量的温度数据，3字节，位0:15(T_DATA)，最后测得的温度，存储为小端序16位无符号值，以1/64Kelvin为单位，位16：数据有效指示位（1表示T_DATA有效），位17:23：循环冗余校验（CRC）
#define ENS210_REG_H_VAL        0x33    // 保存测量的相对湿度数据，3字节，位0:(H_DATA),最后测得的相对湿度，以1/512%RH为单位存储为小端序16位无符号值，位16：数据有效指示位（1表示H_DATA有效），位17:23：循环冗余校验（CRC）

/*******************************硬件通信层*******************************/

//I2C初始化
void i2c_master_init(void)
{
    // I2C 主机总线配置
    i2c_master_bus_config_t i2c_master_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT, // 选择XTAL作为默认时钟
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL,
        .sda_io_num = I2C_SDA,
        .glitch_ignore_cnt = I2C_GLITH_CNT,
        .flags.enable_internal_pullup = true // 启用内部上拉电阻
    };

    // 分配和初始化I2C主机总线
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_cfg, &bus_handle));

    // I2C主机设备配置
    i2c_device_config_t i2c_device_cfg = {
        .dev_addr_length = I2C_SLAVE_ADDR_LEN,
        .device_address = I2C_SLAVE_ADDR,
        .scl_speed_hz = I2C_SCL_SPEED
    };

    // 分配I2C实例，并将设备挂载到主机总线上
    ESP_ERROR_CHECK( i2c_master_bus_add_device(bus_handle, &i2c_device_cfg, &dev_handle));
}

// //获取设备标识
// esp_err_t ens_read_part_id()
// {

// }

//获取UID
esp_err_t ens_read_uid(uint8_t *read_buffer)
{
    ESP_LOGI(TAG,"唉");
    uint8_t reg_addr = ENS210_REG_UID;//寄存器地址
    esp_err_t ret = i2c_master_transmit_receive(dev_handle,&reg_addr,1,read_buffer,8,-1);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UID读取失败: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "UID:%x",*(read_buffer));
    }

    return ret;
}

//系统控制
esp_err_t ens_write_sys_ctrl(uint8_t value)
{
    uint8_t reg_data[2] = {ENS210_REG_SYS_CTRL,value};//寄存器地址+写入的数据
    //!向从机设备写入数据
    //!参数：i2c_dev--由i2c_master_bus_add_device（）创建并返回的设备句柄，代表某个具体i2c从设备
    //!      write_buffer--要发送的数据缓冲区，指要发送到i2c从设备的数据数组指针
    //!      write_size--发送的数据长度，以字节为单位
    //!      xfer_timeout_ms--超时时间，指定本次数据传输的超时时间，传输过程中若超时，则返回ESP_ERR_TIMEOUT，设为-1，表示一直等待到完成，永不超时
    esp_err_t ret = i2c_master_transmit(dev_handle,reg_data,sizeof(reg_data),-1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write SYS_CTRL: %d", ret);
    }

    return ret;
}

//开始测量
//value的值：0b1--开始温度传感器测量，0b10--开始相对湿度传感器测量
esp_err_t ens_write_sens_start(uint8_t value)
{
    uint8_t reg_addr[2] = {ENS210_REG_SENS_START,value};//寄存器地址+写入的数据
    esp_err_t ret = i2c_master_transmit(dev_handle,reg_addr,sizeof(reg_addr),-1);
    if(ret != ESP_OK)
    {
        ESP_LOGI(TAG,"开始测量失败：%s",esp_err_to_name(ret));
    }
    return ret;
}

// /*************旧版本i2c*****************/

// #define I2C_SCL GPIO_NUM_7  // SCL-IO7
// #define I2C_SDA GPIO_NUM_1  // SDA-IO1
// #define ENS210_REG_UID 0x04 // 唯一标识，读取唯一设备ID，8字节
// #define ENS210_REG_SYS_CTRL 0x10
// #define I2C_MASTER_NUM  I2C_NUM_0
// #define I2C_SLAVE_ADDR 0x43                   // ENS210的地址

// void djahd_init(void)
// {
//     esp_err_t ret;
//     i2c_config_t conf = {
//         .mode = I2C_MODE_MASTER,
//         .sda_io_num = I2C_SDA, // select GPIO specific to your project
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_io_num = I2C_SCL, // select GPIO specific to your project
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .master.clk_speed = 100000, // select frequency specific to your project
//         // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
//     };

//     ret = i2c_param_config(I2C_MASTER_NUM,&conf);
//         if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "I2C parameter configuration error: %s", esp_err_to_name(ret));
//         return;
//     }

//     ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "I2C driver installation error: %s", esp_err_to_name(ret));
//         return;
//     }
// }

// uint16_t sys_ctrl() {
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     esp_err_t ret;
//     uint8_t temp = 0b0;
//     uint8_t reg_add=0x10;

//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, (I2C_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, true);
//     i2c_master_write_byte(cmd, reg_add, true);
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, (I2C_SLAVE_ADDR << 1) | I2C_MASTER_READ, true);
//     i2c_master_write_byte(cmd, temp, true);
//     i2c_master_stop(cmd);

//     ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 2000 / portTICK_PERIOD_MS);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "sys_ctrl failed: %s", esp_err_to_name(ret));
//     }
//     i2c_cmd_link_delete(cmd);
//     return ret;
// }

// uint8_t *get_uid() {
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     esp_err_t ret;
//     static uint8_t uid[8];
//     uint8_t reg_add=0x04;

//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, (I2C_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, true);
//     i2c_master_write_byte(cmd, reg_add, true);
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, (I2C_SLAVE_ADDR << 1) | I2C_MASTER_READ, true);
//     i2c_master_read(cmd, uid, sizeof(uid), I2C_MASTER_LAST_NACK);
//     i2c_master_stop(cmd);

//     ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 2000 / portTICK_PERIOD_MS);
//     i2c_cmd_link_delete(cmd);

//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "I2C communication error: %s", esp_err_to_name(ret));
//         return 0;
//     }
//     return  uid;
// }