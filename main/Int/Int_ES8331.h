#ifndef __INT_ES8311_H__
#define __INT_ES8311_H__

#include "esp_task.h"
// i2c所需要源文件
#include "driver/i2c.h"
// i2s所需要源文件
#include "driver/i2s_std.h"
#include "driver/i2s_tdm.h"
#include "soc/soc_caps.h"
// 编解码器所需要源文件
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "unity.h"
// i2c所需要宏定义
#define ES8311_I2C_SDA_PIN GPIO_NUM_0
#define ES8311_I2C_SCL_PIN GPIO_NUM_1
// i2s所需要宏定义
#define ES8311_I2S_MCK_PIN GPIO_NUM_3
#define ES8311_I2S_BCK_PIN GPIO_NUM_2
#define ES8311_I2S_DATA_WS_PIN GPIO_NUM_5
#define ES8311_I2S_DATA_OUT_PIN GPIO_NUM_6
#define ES8311_I2S_DATA_IN_PIN GPIO_NUM_4
// 编解码器所需宏定义
#define ES8311_PA_PIN GPIO_NUM_7

// es8311初始化
void int_es8311_init(void);
// es8311反初始化
void int_es8311_deinit(void);
// 从麦克风读取数据
void int_es8311_read_mic(int8_t *data, uint16_t len);
// 给扬声器写数据
void int_es8311_write_speaker(int8_t *data, uint16_t len);
// 设置麦克风音量
void int_es8311_set_volume(int volume);
// 设置麦克风静音
void int_es8311_set_mute(bool mute);

#endif /* __INT_ES8311_H__ */
