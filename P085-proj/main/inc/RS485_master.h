#ifndef __RS_MASTER_H_
#define __RS_MASTER_H_

#include "esp_err.h"

esp_err_t master_init(void);//主站初始化
void master_operation_func(void *arg);

#endif