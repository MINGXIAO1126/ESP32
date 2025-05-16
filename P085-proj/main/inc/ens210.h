#ifndef __ENS_H_
#define __ENS_H_

#include "driver/i2c_master.h"


void i2c_master_init(void);//i2c初始化
esp_err_t ens_write_sys_ctrl(uint8_t value);
esp_err_t ens_read_uid(uint8_t *read_buffer);
esp_err_t ens_write_sens_start(uint8_t value);

// void djahd_init(void);
// uint16_t sys_ctrl();
// uint16_t get_uid();

#endif