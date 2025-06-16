/*
esp32c6上搭载了一个12位逐次逼进型ADC,2^12=4096
*/
#include "esp_adc/adc_continuous.h"
#include "esp_log.h"
#include "ntc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "math.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG  "NTC"

//#define MAX_BUF_SIZE                 1024                           // 最大缓冲池，保存ADC转换结果
//#define FRAME_SIZE                   256                            // 转换帧大小，一个转换帧包含多个转换结果，一个转换结果包含多个字节，包括ADC单元、ADC通道以及原始数据（要求是SOC_ADC_DIGI_DATA_BYTES_PER_CONV的整数倍）
//#define NTC_ADC_CONV_MODE            ADC_CONV_SINGLE_UNIT_1         // 连续转换模式
#define NTC_ADC_OUTPUT_TYPE          ADC_DIGI_OUTPUT_FORMAT_TYPE2   // 转换模式结果的输出格式
#define NTC_ADC_ATTEN                ADC_ATTEN_DB_12                // ADC衰减,如果需要转换大于ADC内部参考电压（1100mv）的电压，信号输入ADC之前可以进行衰减
#define NTC_ADC_CHANNEL              ADC_CHANNEL_4                  // adc通道
#define NTC_ADC_UINT                 ADC_UNIT_1                     // adc1
#define NTC_ADC_BIT_WIDTH            ADC_BITWIDTH_12                // 转换结果的位宽
//#define NTC_ADC_GET_CHANNEL(p_data)  ((p_data)->type2.channel)
#define NTC_ADC_GET_DATA(P_data)     ((P_data)->type2.data)

adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t adc_cali_handle = NULL;

static bool adc_calibration_init(adc_unit_t unit,adc_channel_t channel,adc_atten_t atten,adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL; 
     bool calibrated = false;   

    adc_cali_curve_fitting_config_t cali_cfg ={
        .unit_id   = unit,
        .chan      = channel,
        .atten     = atten,
        .bitwidth  = NTC_ADC_BIT_WIDTH
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_cfg,&handle);
    if(ret == ESP_OK)
    {
       calibrated = true;
    }

    *out_handle = handle;

    return calibrated;
}

void ntc_adc_init(void)
{ 
    adc_oneshot_unit_init_cfg_t adc_cfg ={
        .unit_id  = NTC_ADC_UINT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg,&adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg ={
        .atten    = NTC_ADC_ATTEN,
        .bitwidth = NTC_ADC_BIT_WIDTH
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,NTC_ADC_CHANNEL,&chan_cfg));

    adc_calibration_init(NTC_ADC_UINT,NTC_ADC_CHANNEL,NTC_ADC_ATTEN,&adc_cali_handle);
}

//获取电阻值
float get_resitance()
{
    int adc_raw = 0;
    int adc_voltage = 0;

    adc_oneshot_read(adc_handle,NTC_ADC_CHANNEL,&adc_raw);
    adc_cali_raw_to_voltage(adc_cali_handle,adc_raw,&adc_voltage);
    //ESP_LOGE(TAG,"原始值：%d",adc_raw);
    //ESP_LOGE(TAG,"校准电压：%d",adc_voltage);
    vTaskDelay(pdMS_TO_TICKS(100));
    float voltage_mv = adc_voltage/1000.0f;

    return 30000.0f * voltage_mv / (3.3f - voltage_mv);
}

//计算温度值
float get_temperature(float NTC_R)
{
    float Rn = 10000.0f;        // NTC 在 25°C 下的电阻值 (Ω)
    float BETA = 3950.0f;         // Beta 值
    float T0 = 298.15f;         // 25°C 对应的开尔文温度

    //根据阻值计算温度
    float temp_k = 1.0f / (1.0f / T0 + (1.0f / BETA) * log(NTC_R / Rn));;

    return temp_k -273.15f;
}

/***************************************************连续采样，不太准************************************************ */
// adc_continuous_handle_t adc_handle = NULL;
// adc_cali_handle_t adc_cali_handle = NULL;

// void ntc_adc_init(void)
// {
//     // 配置ADC连续转换模式
//     adc_continuous_handle_cfg_t adc_cfg = {
//         .max_store_buf_size = MAX_BUF_SIZE,
//         .conv_frame_size    = FRAME_SIZE,
//         .flags.flush_pool   = 1 // 1--缓冲池满了之后自动清空，重新接收数据，0--缓冲池满了会等待用户取数据，可能丢失后续数据
//     };

//     ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_cfg, &adc_handle));

//     // 配置ADCIO
//     adc_digi_pattern_config_t pattern_cfg = {
//         .atten     = NTC_ADC_ATTEN,
//         .channel   = NTC_ADC_CHANNEL,
//         .unit      = NTC_ADC_UINT,
//         .bit_width = NTC_ADC_BIT_WIDTH
//     };

//     adc_continuous_config_t dig_cfg = {
//         .sample_freq_hz = 20 * 1000, // ADC采样频率
//         .conv_mode      = NTC_ADC_CONV_MODE,
//         .format         = NTC_ADC_OUTPUT_TYPE,
//         .pattern_num    = 1, //ADC通道数量
//         .adc_pattern    = &pattern_cfg //ADC通道配置
//     };

//     //创建曲线拟合方案(C6支持ADC_CALI_SCHEME_VER_CURVE_FITTING校准方案)
//     adc_cali_curve_fitting_config_t adc_cali_cfg={
//         .unit_id    = NTC_ADC_UINT,
//         .chan       = NTC_ADC_CHANNEL,
//         .bitwidth   = NTC_ADC_BIT_WIDTH
//     };

//     ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&adc_cali_cfg,&adc_cali_handle)); 

//     ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));

//     ESP_ERROR_CHECK(adc_continuous_start(adc_handle));//启动ADC转换
// }


// //获取电阻值
// float get_voltage (uint8_t *read_buf,uint32_t buf_len)
// {
//     uint32_t result_len = 0;
//     int voltage_mv = 0;
//     uint32_t adc_sum = 0;

//     for(int i = 0;i<10;i++)
//     {    
//         //用于读取转换结果
//         //!参数：handle--ADC连续模式句柄
//         //!      buf--接收缓冲区，用于存储ADC读取的原始采样数据
//         //!      length_max--期望读取的最大字节数(必须是4的倍数)
//         //!      out_length--实际读取到的数据字节数
//         //!      timeout_ms--等待超时时间
//         esp_err_t ret = adc_continuous_read(adc_handle,read_buf,buf_len,&result_len,1000);
//         if (ret != ESP_OK || result_len == 0)
//         {
//             ESP_LOGE(TAG, "Failed to read ADC data, err=0x%x", ret);
//             return 0;
//         }

//         adc_digi_output_data_t *p = (adc_digi_output_data_t *)read_buf;
//         //uint32_t channel_num = NTC_ADC_GET_CHANNEL(p);
//         uint32_t adc_raw = NTC_ADC_GET_DATA(p);
//         adc_sum += adc_raw;
//     }

//     uint32_t adc_avg = adc_sum/10;
//     ESP_LOGE(TAG,"adc原始值：%"PRIu32,adc_avg); 

//     ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_avg, &voltage_mv));
//     float voltage =  voltage_mv/1000.0f;
//     // float voltage = ((float)adc_sum / 4095.0f) * 3.3f;
//     ESP_LOGE(TAG,"电压：%.2f",voltage);   

//     //计算热敏电阻阻值
//     float NTC_R = (adc_avg/(4095.0f - adc_avg))* 30000.0f;
//     //float NTC_R =  30000.0f * voltage / (3.3f - voltage);

//     return NTC_R;
// }