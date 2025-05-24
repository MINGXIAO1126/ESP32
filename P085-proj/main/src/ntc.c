/*
esp32c6上搭载了一个12位逐次逼进型ADC,2^12=4096
*/
#include "esp_adc/adc_continuous.h"
#include "esp_log.h"
#include "ntc.h"
#include "esp_adc/adc_cali.h"
#include "adc_cali_schemes.h"
#include "math.h"


#define TAG  "热敏电阻"

adc_continuous_handle_t adc_handle = NULL;

#define MAX_BUF_SIZE                 1024     // 最大缓冲池，保存ADC转换结果
#define FRAME_SIZE                   256        // 转换帧大小，一个转换帧包含多个转换结果，一个转换结果包含多个字节，包括ADC单元、ADC通道以及原始数据（要求是SOC_ADC_DIGI_DATA_BYTES_PER_CONV的整数倍）
#define NTC_ADC_CONV_MODE            ADC_CONV_SINGLE_UNIT_1      // 连续转换模式
#define NTC_ADC_OUTPUT_TYPE          ADC_DIGI_OUTPUT_FORMAT_TYPE2 // 转换模式结果的输出格式
#define NTC_ADC_ATTEN                ADC_ATTEN_DB_12             // ADC衰减,如果需要转换大于ADC内部参考电压（1100mv）的电压，信号输入ADC之前可以进行衰减
#define NTC_ADC_CHANNEL              ADC_CHANNEL_4              // adc通道
#define NTC_ADC_UINT                 ADC_UNIT_1                    // adc1
#define NTC_ADC_BIT_WIDTH            ADC_BITWIDTH_12      // 转换结果的位宽
#define NTC_ADC_GET_CHANNEL(p_data)  ((p_data)->type2.channel)
#define NTC_ADC_GET_DATA(P_data)     ((P_data)->type2.data)

#define NTC_R0      30000.0f        //基准电阻
#define Rn          10000.0f       // NTC 在 25°C 下的电阻值 (Ω)
#define BETA        3950.0f        // Beta 值
#define T0_K        298.15f        // 25°C 对应的开尔文温度

void ntc_adc_init(void)
{
    // 配置ADC连续转换模式
    adc_continuous_handle_cfg_t adc_cfg = {
        .max_store_buf_size = MAX_BUF_SIZE,
        .conv_frame_size    = FRAME_SIZE,
        .flags.flush_pool   = 1 // 1--缓冲池满了之后自动清空，重新接收数据，0--缓冲池满了会等待用户取数据，可能丢失后续数据
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_cfg, &adc_handle));

    // 配置ADCIO
    adc_digi_pattern_config_t pattern_cfg = {
        .atten     = NTC_ADC_ATTEN,
        .channel   = NTC_ADC_CHANNEL,
        .unit      = NTC_ADC_UINT,
        .bit_width = NTC_ADC_BIT_WIDTH
    };

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000, // ADC采样频率
        .conv_mode      = NTC_ADC_CONV_MODE,
        .format         = NTC_ADC_OUTPUT_TYPE,
        .pattern_num    = 1, //ADC通道数量
        .adc_pattern    = &pattern_cfg //ADC通道配置
    };

    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));

    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));//启动ADC转换
}

//创建校准方案(C6支持ADC_CALI_SCHEME_VER_CURVE_FITTING校准方案)
// void adc_cali(void)
// {
//     adc_cali_curve_fitting_config_t 
    
// } 





//读取电压值
float get_voltage (uint8_t *read_buf,uint32_t buf_len)
{
    uint32_t result_len = 0;

    //用于读取转换结果
    //!参数：handle--ADC连续模式句柄
    //!      buf--接收缓冲区，用于存储ADC读取的原始采样数据
    //!      length_max--期望读取的最大字节数(必须是4的倍数)
    //!      out_length--实际读取到的数据字节数
    //!      timeout_ms--等待超时时间
    esp_err_t ret = adc_continuous_read(adc_handle,read_buf,buf_len,&result_len,1000);
    if (ret != ESP_OK || result_len == 0)
    {
        ESP_LOGE(TAG, "Failed to read ADC data, err=0x%x", ret);
        return 0;
    }

    adc_digi_output_data_t *p = (adc_digi_output_data_t *)read_buf;
    uint32_t channel_num = NTC_ADC_GET_CHANNEL(p);
    uint32_t adc_raw = NTC_ADC_GET_DATA(p); 

    //ESP_LOGE(TAG,"Channel:%"PRIu32  "原始值:%"PRIx32,channel_num,adc_raw);

    //计算电压值
    float V_out = (float)adc_raw * 3.3/4095;

    return V_out;
}

//计算温度值
float get_temperature(float V_out)
{
    //计算热敏电阻阻值
    float NTC_R =NTC_R0 * V_out/(3.3-V_out);

    //计算温度（开尔文）
    float temp_K = 1.0f / ( (1.0f / T0_K) + (1.0f / BETA) * log(NTC_R / Rn) );

    return temp_K -273.15f;
}