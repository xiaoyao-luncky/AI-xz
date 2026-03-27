#include "int_es8311.h"

i2s_chan_handle_t mic_handler = NULL;
i2s_chan_handle_t speaker_handler = NULL;

esp_codec_dev_handle_t codec_dev=NULL;

static void int_es8311_i2c_init(void)
{
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_cfg.sda_io_num = ES8311_I2C_SDA_PIN;
    i2c_cfg.scl_io_num = ES8311_I2C_SCL_PIN;
    i2c_param_config(I2C_NUM_0, &i2c_cfg);
    i2c_driver_install(I2C_NUM_0, i2c_cfg.mode, 0, 0, 0);
}

static void int_es8311_dev_init(void)
{
    /* 1 创建数据接口配置结构体
     * 音频数据传输通道，通常用于传输PCM音频数据
     */
    audio_codec_i2s_cfg_t i2s_cfg = {
        .rx_handle = mic_handler,     /* 接收通道句柄：用于音频输入（录音） */
        .tx_handle = speaker_handler, /* 发送通道句柄：用于音频输出（播放） */
    };
    /* 创建I2S数据接口实例
     * 此接口负责音频数据的收发，通常连接到I2S外设
     * 返回的数据接口指针用于后续的音频设备创建
     */
    const audio_codec_data_if_t *data_if = audio_codec_new_i2s_data(&i2s_cfg);

    /* 2 创建I2C控制接口配置结构体
     * 用于控制音频编解码器的寄存器（如ES8311）
     */
    audio_codec_i2c_cfg_t i2c_cfg = {
        .addr = ES8311_CODEC_DEFAULT_ADDR, /* I2C从机地址：ES8311默认地址0x18 */
    };
    /* 创建I2C控制接口实例
     * 此接口用于通过I2C总线控制音频编解码器
     * 可执行的操作包括：设置音量、采样率、电源管理等
     */
    const audio_codec_ctrl_if_t *out_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);

    /* 3 创建GPIO控制接口实例
     */
    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();

    // 4. 创建编码器接口示例
    /* 创建ES8311音频编解码器配置结构体
     * ES8311是一款立体声音频编解码器芯片，包含ADC和DAC功能
     */
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
        .ctrl_if = out_ctrl_if,
        .gpio_if = gpio_if,
        .pa_pin = ES8311_PA_PIN,
        .use_mclk = true,
    };
    /* 创建ES8311编解码器接口实例*/
    const audio_codec_if_t *out_codec_if = es8311_codec_new(&es8311_cfg);

    // 5. 拿到编码器接口示例和编码器数据实例，创建编码器设备对象
    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = out_codec_if,             // es8311_codec_new 获取到的接口实现
        .data_if = data_if,                   // audio_codec_new_i2s_data 获取到的数据接口实现
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT // 设备同时支持录制和播放
    };
    codec_dev = esp_codec_dev_new(&dev_cfg);

    // 6. 设置播放音量和输入增益
    // 修改播放音量
    esp_codec_dev_set_out_vol(codec_dev, 50);
    // 设置输入增益
    esp_codec_dev_set_in_gain(codec_dev, 20);

    // 7. 启动设备
    esp_codec_dev_sample_info_t fs = {
        .sample_rate = 16000,
        .channel = 1,
        .bits_per_sample = 16,
    };
    esp_codec_dev_open(codec_dev, &fs);
}

static void int_es8311_dev_deinit(void)
{
    esp_codec_dev_close(codec_dev);
}

static void int_es8311_i2c_deinit(void)
{
    i2c_driver_delete(I2C_NUM_0);
}

static void int_es8311_i2s_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(16, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = ES8311_I2S_MCK_PIN,
            .bclk = ES8311_I2S_BCK_PIN,
            .ws = ES8311_I2S_DATA_WS_PIN,
            .dout = ES8311_I2S_DATA_OUT_PIN,
            .din = ES8311_I2S_DATA_IN_PIN
        },
    };
    i2s_new_channel(&chan_cfg, &speaker_handler, &mic_handler);
    i2s_channel_init_std_mode(speaker_handler, &std_cfg);
    i2s_channel_init_std_mode(mic_handler, &std_cfg);

    // For tx master using duplex mode
    i2s_channel_enable(speaker_handler);
    i2s_channel_enable(mic_handler);
}

static void int_es8311_i2s_deinit(void)
{
    i2s_channel_disable(speaker_handler);
    i2s_channel_disable(mic_handler);
    i2s_del_channel(speaker_handler);
    i2s_del_channel(mic_handler);
}

// es8311初始化
void int_es8311_init(void)
{
    // i2c初始化
    int_es8311_i2c_init();
    // i2s初始化
    int_es8311_i2s_init();
    //初始化编解码器
    int_es8311_dev_init();
}

// es8311反初始化
void int_es8311_deinit(void)
{
    //编解码器反初始化
    int_es8311_dev_deinit();
    // i2s反初始化
    int_es8311_i2s_deinit();
    // i2c反初始化
    int_es8311_i2c_deinit();
}

//从麦克风读取数据
void int_es8311_read_mic(int8_t *data,uint16_t len)
{
    if(codec_dev!=NULL)
    {
        esp_codec_dev_read(codec_dev,data,len);
    }
}

//给扬声器写数据
void int_es8311_write_speaker(int8_t *data,uint16_t len)
{
    if(codec_dev !=NULL)
    {
        esp_codec_dev_write(codec_dev,data,len);
    }
}

//设置麦克风音量
void int_es8311_set_volume(int volume)
{
    esp_codec_dev_set_out_vol(codec_dev,volume);
}

//设置麦克风静音
void int_es8311_set_mute(bool mute)
{
    esp_codec_dev_set_out_mute(codec_dev,mute);
}

