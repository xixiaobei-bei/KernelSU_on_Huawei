/*
 * dsp_utils.h
 *
 * misc utils driver for hi64xx codecdsp
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

#ifndef __DSP_UTILS_H__
#define __DSP_UTILS_H__

#include <sound/soc.h>
#include <linux/hisi/hi64xx_dsp/hi64xx_dsp_misc.h>

union hi64xx_low_freq_status {
	unsigned int val;
	struct {
		unsigned int wake_up: 1;
		unsigned int set_para: 1;
		unsigned int fast_trans_set: 1;
		unsigned int pwr_test: 1;
		unsigned int msg_proc: 1;
		unsigned int multi_wake_up: 1;
		unsigned int virtual_bin: 1;
		unsigned int dump: 1;
	};
};

union hi64xx_high_freq_status {
	unsigned int val;
	struct {
		unsigned int pa: 1;
		unsigned int hook: 1;
		unsigned int get_para: 1;
		unsigned int set_para: 1;
		unsigned int om: 1;
		unsigned int mad_test: 1;
		unsigned int fault_inject: 1;
		unsigned int pwr_test: 1;
		unsigned int anc: 1;
		unsigned int anc_test: 1;
		unsigned int anc_debug: 1;
		unsigned int om_hook: 1;
		unsigned int dsp_debug: 1;
		unsigned int mem_check: 1;
		unsigned int fast_trans: 1;
		unsigned int ir_learn: 1;
		unsigned int ir_trans: 1;
		unsigned int virtual_btn: 1;
		unsigned int ultrasonic: 1;
	};
};

void hi64xx_dsp_write_reg(unsigned int reg, unsigned int val);
unsigned int hi64xx_dsp_read_reg(unsigned int reg);
void hi64xx_dsp_reg_set_bit(unsigned int reg, unsigned int offset);
void hi64xx_dsp_reg_clr_bit(unsigned int reg, unsigned int offset);
void hi64xx_dsp_reg_write_bits(unsigned int reg,
	unsigned int value, unsigned int mask);
void hi64xx_memset(uint32_t dest, size_t n);
void hi64xx_memcpy(uint32_t dest, uint32_t *src, size_t n);
void hi64xx_read(const unsigned int start_addr,
	unsigned char *arg, const unsigned int len);
void hi64xx_write(const unsigned int start_addr,
	const unsigned int *buf, const unsigned int len);
int hi64xx_sync_write(const void *arg, const unsigned int len);
int hi64xx_dsp_utils_init(const struct hi64xx_dsp_config *config,
	struct snd_soc_codec *codec, struct hi64xx_resmgr *resmgr);
void hi64xx_dsp_utils_deinit(void);
void hi64xx_dsp_set_pll(bool enable);
void hi64xx_dsp_set_low_pll(bool enable);
void hi64xx_reset_dsp(void);
void hi64xx_pause_dsp(void);
bool hi64xx_get_dsp_state(void);
void hi64xx_resume_dsp(enum pll_state state);
void hi64xx_run_dsp(void);
int hi64xx_request_pll_resource(unsigned int scene_id);
void hi64xx_release_pll_resource(unsigned int scene_id);
int hi64xx_request_low_pll_resource(unsigned int scene_id);
void hi64xx_release_low_pll_resource(unsigned int scene_id);
void hi64xx_misc_dump_bin(const unsigned int addr, char *buf,
	const size_t len);
void hi64xx_release_requested_pll(void);
int hi64xx_check_msg_para(const struct hi64xx_param_io_buf *param,
	unsigned int in_size);
int hi64xx_save_log_file(const char *dump_ram_path, unsigned int type,
	unsigned int dump_addr, unsigned int dump_size);

#endif

