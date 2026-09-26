/*
 * da_combine_dsp_msic.h
 *
 * misc driver for da_combine codecdsp
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DA_COMBINE_DSP_MISC_H__
#define __DA_COMBINE_DSP_MISC_H__

#include <sound/soc.h>
#include <linux/types.h>
#include <linux/hisi/da_combine/da_combine_resmgr.h>
#include <linux/hisi/da_combine/da_combine_irq.h>
#include <linux/hisi/da_combine/hi_cdc_ctrl.h>

/* dump da combine dsp manually, debug only */
#define DUMP_DIR_ROOT "/data/hisi_logs/hi64xxdump/" /* length:28 */

#define MSG_SEND_RETRIES 0
#define DSP_IMG_SEC_MAX_NUM 32
#define NOTIFY_DSP_WAIT_CNT 5

#define INT_TO_ADDR(low, high) (void*) (uintptr_t)((unsigned long long)(low) | ((unsigned long long)(high)<<32))
#define GET_LOW32(x) (unsigned int)(((unsigned long long)(unsigned long)(x))&0xffffffffULL)
#define GET_HIG32(x) (unsigned int)((((unsigned long long)(unsigned long)(x))>>32)&0xffffffffULL)
#define UNUSED_PARAMETER(x) (void)(x)

enum {
	HIGH_FREQ_SCENE_PA,
	HIGH_FREQ_SCENE_HOOK,
	HIGH_FREQ_SCENE_GET_PARA,
	HIGH_FREQ_SCENE_SET_PARA,
	HIGH_FREQ_SCENE_OM,
	HIGH_FREQ_SCENE_MAD_TEST,
	HIGH_FREQ_SCENE_FAULT_INJECT,
	HIGH_FREQ_SCENE_PWR_TEST,
	HIGH_FREQ_SCENE_ANC,
	HIGH_FREQ_SCENE_ANC_TEST,
	HIGH_FREQ_SCENE_ANC_DEBUG,
	HIGH_FREQ_SCENE_OM_HOOK,
	HIGH_FREQ_SCENE_DSP_DEBUG,
	HIGH_FREQ_SCENE_MEM_CHECK,
	HIGH_FREQ_SCENE_FASTTRANS,
	HIGH_FREQ_SCENE_IR_LEARN,
	HIGH_FREQ_SCENE_IR_TRANS,
	HIGH_FREQ_SCENE_VIRTUAL_BTN,
	HIGH_FREQ_SCENE_ULTRASONIC,
	HIGH_FREQ_SCENE_BUTT,
};

enum {
	LOW_FREQ_SCENE_WAKE_UP,
	LOW_FREQ_SCENE_SET_PARA,
	LOW_FREQ_SCENE_FAST_TRANS_SET,
	LOW_FREQ_SCENE_PWR_TEST,
	LOW_FREQ_SCENE_MSG_PROC,
	LOW_FREQ_SCENE_MULTI_WAKE_UP,
	LOW_FREQ_SCENE_VIRTUAL_BTN,
	LOW_FREQ_SCENE_DUMP,
	LOW_FREQ_SCENE_BUTT,
};

enum {
	HIFI_STATE_UNINIT,
	HIFI_STATE_INIT,
	HIFI_STATE_BUTT,
};

enum {
	DUMP_TYPE_WHOLE_OCRAM,
	DUMP_TYPE_WHOLE_IRAM,
	DUMP_TYPE_WHOLE_DRAM,
	DUMP_TYPE_PRINT_LOG,
	DUMP_TYPE_PANIC_LOG,
	DUMP_TYPE_REG,
#ifdef ENABLE_DA_COMBINE_HIFI_DEBUG
	DUMP_TYPE_WAKEUP_PCM,
	DUMP_TYPE_TOTAL_LOG,
#endif
};

enum pll_state {
	PLL_PD,
	PLL_HIGH_FREQ,
	PLL_LOW_FREQ,
	PLL_RST,
};

enum low_pll_state {
	DSP_PLL_24M,
	DSP_PLL_48M,
	DSP_PLL_96M,
	DSP_PLL_BUTT
};

enum da_combine_pcm_sample_rate {
	DA_COMBINE_SAMPLE_RATE_INDEX_8K = 0,
	DA_COMBINE_SAMPLE_RATE_INDEX_16K,
	DA_COMBINE_SAMPLE_RATE_INDEX_32K,
	DA_COMBINE_SAMPLE_RATE_INDEX_RESERVED0,
	DA_COMBINE_SAMPLE_RATE_INDEX_48K,
	DA_COMBINE_SAMPLE_RATE_INDEX_96K,
	DA_COMBINE_SAMPLE_RATE_INDEX_192K,
	DA_COMBINE_SAMPLE_RATE_INDEX_RESERVED1,
};

enum da_combine_pcm_direct {
	DA_COMBINE_DSP_PCM_IN = 0,
	DA_COMBINE_DSP_PCM_OUT,
	DA_COMBINE_DSP_PCM_DIRECT_BUTT,
};

enum da_combine_if_port {
	DA_COMBINE_DSP_IF_PORT_0 = 0,
	DA_COMBINE_DSP_IF_PORT_1,
	DA_COMBINE_DSP_IF_PORT_2,
	DA_COMBINE_DSP_IF_PORT_3,
	DA_COMBINE_DSP_IF_PORT_4,
	DA_COMBINE_DSP_IF_PORT_5,
	DA_COMBINE_DSP_IF_PORT_6,
	DA_COMBINE_DSP_IF_PORT_7,
	DA_COMBINE_DSP_IF_PORT_8,
	DA_COMBINE_DSP_IF_PORT_9,
	DA_COMBINE_DSP_IF_PORT_BUTT,
};

enum da_combine_msg_state {
	DA_COMBINE_MSG_STATE_CLEAR = 0,
	DA_COMBINE_AP_SEND_MSG,
	DA_COMBINE_DSP_RECEIVE_MSG,
	DA_COMBINE_DSP_SEND_MSG_CNF,
	DA_COMBINE_AP_RECEIVE_MSG_CNF,
	DA_COMBINE_DSP_SEND_PLL_SW_OFF_CNF,
	DA_COMBINE_DSP_SEND_PLL_SW_ON_CNF,
	DA_COMBINE_AP_RECEIVE_PLL_SW_CNF,
	DA_COMBINE_DSP_SEND_PWRON_CNF,
	DA_COMBINE_AP_RECEIVE_PWRON_CNF,
	DA_COMBINE_MSG_STATE_BUTT
};

struct da_combine_dump_param_io_buf {
	unsigned int user_buf_l; /* User space allocated memory address */
	unsigned int user_buf_h; /* User space allocated memory address */
	unsigned int clear;      /* clear current log buf */
	unsigned int buf_size;   /* User space allocated memory length */
};

struct da_combine_param_io_buf {
	unsigned char *buf_in;
	unsigned int buf_size_in;
	/* variables below is for sync cmd only */
	unsigned char *buf_out;
	unsigned int buf_size_out;
};

struct da_combine_dsp_ops {
	/* init da_combine dsp regs */
	void (*init)(void);
	/* deinit da_combine dsp regs */
	void (*deinit)(void);
	/* enable da_combine dsp clk */
	void (*clk_enable)(bool);
	/* config da_combine dsp axi */
	void (*ram2axi)(bool);
	/* config runstall */
	void (*runstall)(bool);
	/* config watchdog */
	void (*wtd_enable)(bool);
	/* config uart */
	void (*uart_enable)(bool);
	/* config i2c */
	void (*i2c_enable)(bool);
	/* notify da_combine dsp */
	void (*notify_dsp)(void);
	/* suspend proc */
	int (*suspend)(void);
	/* resume proc */
	int (*resume)(void);
	/* power on/off dsp */
	void (*dsp_power_ctrl)(bool);
	void (*set_dsp_div)(enum pll_state state);
	/* soundtrigger fast channel open/close */
	void (*set_fasttrans_enable)(bool enable);
	void (*set_if_bypass)(unsigned int dsp_if_id, bool enable);
	void (*mad_enable)(void);
	void (*mad_disable)(void);
	/* ir path */
	void (*ir_path_clean)(void);
	bool (*check_i2s2_clk)(void);
	int (*set_sample_rate)(unsigned int dsp_if_id, unsigned int sample_rate_in,
		unsigned int sample_rate_out);
	void (*config_usb_low_power)(void);
};

struct da_combine_dsp_config {
	struct da_combine_dsp_ops dsp_ops;
	unsigned int codec_type;
	unsigned int msg_addr;
	unsigned int rev_msg_addr;
	unsigned int para_addr;
	unsigned int cmd0_addr;
	unsigned int cmd1_addr;
	unsigned int cmd2_addr;
	unsigned int cmd3_addr;
	unsigned int cmd4_addr;
	unsigned int wtd_irq_num;
	unsigned int vld_irq_num;
	unsigned int dump_ocram_addr;
	unsigned int dump_ocram_size;
	unsigned int dump_log_addr;
	unsigned int dump_log_size;
	unsigned int msg_state_addr;
	unsigned int wfi_state_addr;
	unsigned int ocram_start_addr;
	unsigned int ocram_size;
	unsigned int itcm_start_addr;
	unsigned int itcm_size;
	unsigned int dtcm_start_addr;
	unsigned int dtcm_size;
	unsigned int mlib_to_ap_msg_addr;
	unsigned int mlib_to_ap_msg_size;
	enum bustype_select bus_sel;
	enum low_pll_state low_pll_state;
};

int da_combine_dsp_misc_init(struct snd_soc_component *codec,
	struct da_combine_resmgr *resmgr, struct da_combine_irq *irqmgr,
	const struct da_combine_dsp_config *dsp_config);
void da_combine_dsp_misc_deinit(void);
void da_combine_wtdog_send_event(void);
void da_combine_wtdog_process(void);
void da_combine_soundtrigger_close_codec_dma(void);
void da_combine_set_wtdog_state(bool state);
bool da_combine_get_wtdog_state(void);
void da_combine_set_sync_write_state(bool state);
bool da_combine_get_sync_write_state(void);
void da_combine_set_high_freq_status(unsigned int scene_id,
	bool enable);
unsigned int da_combine_get_high_freq_status(void);
void da_combine_set_low_freq_status(unsigned int scene_id,
	bool enable);
unsigned int da_combine_get_low_freq_status(void);
void da_combine_dsp_state_lock(void);
void da_combine_dsp_state_unlock(void);
void da_combine_clr_cmd_status_bit(uint8_t cmd_bit);
bool da_combine_check_i2s2_clk(void);
bool da_combine_get_sample_rate_index(unsigned int sample_rate,
	unsigned char *sel);
bool da_combine_error_detect(void);
int da_combine_dsp_misc_resume(void);
int da_combine_dsp_misc_suspend(void);
int da_combine_sync_write(const void *arg, const unsigned int len);
enum pll_state da_combine_get_pll_state(void);

#endif /* __DA_COMBINE_DSP_MISC_H__ */

