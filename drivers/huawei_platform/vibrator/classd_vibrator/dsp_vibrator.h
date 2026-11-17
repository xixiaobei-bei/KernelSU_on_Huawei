#ifndef __DSP_VIBRATOR_H__
#define __DSP_VIBRATOR_H__

#include "drv_mailbox_cfg.h"

enum VIB_POWER_TYPE
{
	VIB_CLASS_D_TYPE = 0x0, // class D
	VIB_DSP_PCM_TYPE = 0x1, // dsp play PCM
};

enum VIB_WAVE_TYPE
{
	VIB_WAV_235HZ = 0x0,
	VIB_WAV_Z1,
	VIB_WAV_Z2,
	VIB_WAV_Z3,
	VIB_WAV_Z4,
	VIB_WAV_Z5,
	VIB_CALIBRATION_0,
	VIB_CALIBRATION_1,
	VIB_CALIBRATION_2,
	VIB_CALIBRATION_3,
	VIB_CALIBRATION_4,
	VIB_CALIBRATION_5,
	VIB_CALIBRATION_6,
	VIB_CALIBRATION_7,
	VIB_CALIBRATION_8,
	VIB_CALIBRATION_9,
	VIB_CALIBRATION_10,
	VIB_WAV_COUNT,
};

enum VIB_MODE
{
	MODE_LDO = 0x0,
	MODE_VSYS,
	MODE_5V,
	MODE_COUNT,
};

#define HISI_AP_AUDIO_VIBRATOR_ADDR		0x195f5c80
#define HISI_AP_AUDIO_VIBRATOR_DATA_SIZE		0xF000	// 0x30C00
#define VIB_FB_PLAY_CN		80000

int play_vibrator_data(void *buffer, int buf_size, int count, int type, mb_msg_cb receive_func);
unsigned char *get_trans_buffer(void);
int start_play_vibrator_data(int type, int count);
void stop_play_vibrator_data(void);
bool is_play_vibrator(void);
void set_effect_power_off(void);
void dsp_vibrator_set_freq(uint32_t freq);
void dsp_vibrator_get_freq(uint32_t *freq);
void dsp_vibrator_register_stop_handler(void (*call_back)(void));
void change_vibrator_mode(uint32_t mode);

#endif /* __DSP_VIBRATOR_H__ */