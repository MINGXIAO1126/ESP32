#include "driver/gpio.h"
#include "id.h"
#include "esp_log.h"

#define TAG  "ID          "

void id_init(void)
{
    gpio_config_t id_cfg = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1 << ID0_GPIO) | (1 << ID1_GPIO) | (1 << ID2_GPIO),
        .pull_down_en = GPIO_PULLUP_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&id_cfg);
}

void get_board_id(void)
{
    esp_err_t ret = gpio_get_level(ID0_GPIO);
    esp_err_t ret1 = gpio_get_level(ID1_GPIO);
    esp_err_t ret2 = gpio_get_level(ID2_GPIO);
    esp_err_t board_id = (ret2 * (1 << 2)) + (ret1 * 2) + ret;
    //ESP_LOGE(TAG, "ID:%d%d%d", ret, ret1, ret2);
    ESP_LOGE(TAG,"Board ID:ID%d", board_id);
}