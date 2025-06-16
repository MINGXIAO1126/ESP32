#ifndef __ENS_H_
#define __ENS_H_

#include "driver/i2c_master.h"

void i2c_master_init(void);//i2c初始化
esp_err_t ens_read_register(uint8_t reg_addr,uint8_t *read_buffer);//读寄存器操作
esp_err_t ens_write_register(uint8_t reg_addr,uint8_t data);//写寄存器操作
esp_err_t ens_get_part_id(void);//获取设备标识
esp_err_t ens_get_uid(uint8_t *read_buffer);//获取UID
esp_err_t ens_sys_ctrl(void);//系统控制
esp_err_t ens_sens_start(void);//开始测量
esp_err_t ens_sens_stop(void);//停止测量
esp_err_t ens_sens_run(void);//传感器运行模式（单发或连续）
esp_err_t ens_get_temperature(uint8_t *read_buffer);//获取温度
esp_err_t ens_get_humidity(uint8_t *read_buffer);//获取相对湿度
esp_err_t ens_temperature_conversion(uint32_t t_val);//温度转换
esp_err_t ens_humidity_conversion(uint32_t h_val);//湿度转换

#endif