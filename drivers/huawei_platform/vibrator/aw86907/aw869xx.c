/*
 * X aw869x.c
 *
 * code for vibrator
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/power_supply.h>
#include <linux/pm_qos.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/control.h>
#include <sound/soc.h>
#include <linux/pm_wakeup.h>

#include <aw869xx_reg.h>
#include <aw869xx.h>

#define AW869XX_VERSION "v0.1.2"
#define AW869XX_HAPTIC_NAME               "haptics"
#define MAX_WRITE_BUF_LEN                 16
struct aw869xx *g_aw869xx;

#define null_pointer_err_check(a) \
	do { \
		if (!a) \
			return 0; \
	} while (0)

#define LONG_HAPTIC_RTP_MAX_ID                  2999
#define LONG_HAPTIC_RTP_MIN_ID                  1010

#define SHORT_HAPTIC_RAM_MAX_ID                 999
#define SHORT_HAPTIC_RTP_MAX_ID                 9999

#define SHORT_HAPTIC_RAM_MIN_IDX                1
#define SHORT_HAPTIC_RAM_MAX_IDX                30
#define SHORT_HAPTIC_RTP_MAX_IDX                9999

#define SHORT_HAPTIC_AMP_DIV_COFF               10
#define LONG_TIME_AMP_DIV_COFF                  100
#define BASE_INDEX                              100
#define AW869XX_LONG_MAX_AMP_CFG                9
#define AW869XX_SHORT_MAX_AMP_CFG               6

#define  DEFAULT_RTP_D2S_GAIN                   0x13
#define  DEFAULT_RTP_BRK_GAIN                   0x04
#define  DEFAULT_RTP_BRK_TIME                   0x06
#define  DEFAULT_RAM_SHORT_D2S_GAIN             0x13
#define  DEFAULT_RAM_SHORT_BRK_GAIN             0x04
#define  DEFAULT_RAM_SHORT_BRK_TIME             0x06
#define  DEFAULT_RAM_LONG_D2S_GAIN              0x15
#define  DEFAULT_RAM_LONG_BRK_GAIN              0x08
#define  DEFAULT_RAM_LONG_BRK_TIME              0x06

#define  HAPTIC_BST_VOL_9V                      0x26
#define  LONG_HAPTIC_DRIVER_WAV_CNT             20
#define  LONG_HAPTIC_LOOP_MODE_CNT              15
#define  LONG_HAPTIC_WAV_SEQ_CNT                2
#define  MAX_LONG_HAPTIC_AMP                    1
#define  HAPTIC_MIN_SPEC_CMD_ID                 123450
#define  HAPTIC_MAX_SPEC_CMD_ID                 123460
#define  HAPTIC_NULL_WAVEFORM_ID                123451
#define  HAPTIC_CHARGING_CALIB_ID               123456
#define  HAPTIC_CHARGING_IS_CALIB               1
#define  HAPTIC_WAKE_LOCK_GAP                   200

static const unsigned char long_amp_again_val[AW869XX_LONG_MAX_AMP_CFG] =
	{ 0x80, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10 };

static const unsigned char short_amp_again_val[AW869XX_SHORT_MAX_AMP_CFG] =
	{ 0x80, 0x80, 0x66, 0x4c, 0x33, 0x19 };
#ifdef AW_KERNEL_VER_OVER_4_19
static const struct aw_componet_codec_ops aw_componet_codec_ops = {
	.aw_snd_soc_kcontrol_codec = snd_soc_kcontrol_component,
	.aw_snd_soc_codec_get_drvdata = snd_soc_component_get_drvdata,
	.aw_snd_soc_add_codec_controls = snd_soc_add_component_controls,
	.aw_snd_soc_unregister_codec = snd_soc_unregister_component,
	.aw_snd_soc_register_codec = snd_soc_register_component,
};
#else
static const struct aw_componet_codec_ops aw_componet_codec_ops = {
	.aw_snd_soc_kcontrol_codec = snd_soc_kcontrol_codec,
	.aw_snd_soc_codec_get_drvdata = snd_soc_codec_get_drvdata,
	.aw_snd_soc_add_codec_controls = snd_soc_add_codec_controls,
	.aw_snd_soc_unregister_codec = snd_soc_unregister_codec,
	.aw_snd_soc_register_codec = snd_soc_register_codec,
};
#endif

static aw_snd_soc_codec_t *aw_get_codec(struct snd_soc_dai *dai)
{
#ifdef AW_KERNEL_VER_OVER_4_19
	return dai->component;
#else
	return dai->codec;
#endif
}

static char *aw869xx_ram_name = "aw8690_haptic.bin";
static char aw869xx_rtp_name[][AW869XX_RTP_NAME_MAX] = {
	{"aw869xx_osc_rtp_24K_5s.bin"},
	{"aw869xx_rtp.bin"},
	{"aw869xx_rtp_lighthouse.bin"},
	{"aw869xx_rtp_silk.bin"},
};

static char aw869xx_rtp_idx_name[AW869XX_RTP_NAME_MAX] = {0};
static struct pm_qos_request pm_qos_req_vb;
struct aw869xx_container *aw869xx_rtp;
static struct wakeup_source lra_lock;

static void aw869xx_interrupt_clear(struct aw869xx *aw869xx);
static int aw869xx_i2c_write(struct aw869xx *aw869xx,
	unsigned char reg_addr, unsigned char reg_data)
{
	int ret = -1;
	unsigned char cnt = 0;

	while (cnt < AW_I2C_RETRIES) {
		ret = i2c_smbus_write_byte_data(aw869xx->i2c, reg_addr, reg_data);
		if (ret < 0)
			pr_err("%s: addr=0x%02X, data=0x%02X, cnt=%d, error=%d\n",
				__func__, reg_addr, reg_data, cnt, ret);
		else
			break;

		cnt++;
		usleep_range(AW_I2C_RETRY_DELAY * 1000,
			AW_I2C_RETRY_DELAY * 1000 + 500);
	}
	return ret;
}

static int aw869xx_i2c_read(struct aw869xx *aw869xx,
	unsigned char reg_addr, unsigned char *reg_data)
{
	int ret = -1;
	unsigned char cnt = 0;

	while (cnt < AW_I2C_RETRIES) {
		ret = i2c_smbus_read_byte_data(aw869xx->i2c, reg_addr);
		if (ret < 0) {
			pr_err("%s: i2c_read addr=0x%02X, cnt=%d error=%d\n",
				__func__, reg_addr, cnt, ret);
		} else {
			*reg_data = ret;
			break;
		}
		cnt++;
		usleep_range(AW_I2C_RETRY_DELAY * 1000,
			AW_I2C_RETRY_DELAY * 1000 + 500);
	}
	return ret;
}

static int aw869xx_i2c_write_bits(struct aw869xx *aw869xx,
	unsigned char reg_addr, unsigned int mask, unsigned char reg_data)
{
	int ret;
	unsigned char reg_val = 0;

	ret = aw869xx_i2c_read(aw869xx, reg_addr, &reg_val);
	if (ret < 0) {
		pr_err( "%s: i2c read error, ret=%d\n", __func__, ret);
		return ret;
	}
	reg_val &= mask;
	reg_val |= reg_data;
	ret = aw869xx_i2c_write(aw869xx, reg_addr, reg_val);
	if (ret < 0) {
		pr_err( "%s: i2c write error, ret=%d\n", __func__, ret);
		return ret;
	}
	return 0;
}

static int aw869xx_i2c_writes(struct aw869xx *aw869xx,
	unsigned char reg_addr, unsigned char *buf, unsigned int len)
{
	int ret;
	unsigned char *data = NULL;
	if ((buf == NULL) || (len >= KMALLOC_MAX_CACHE_SIZE))
		return -EINVAL;
	data = kmalloc(len + 1, GFP_KERNEL);
	if (!data) {
		pr_err( "%s: can not allocate memory\n", __func__);
		return -ENOMEM;
	}
	data[0] = reg_addr;
	memcpy(&data[1], buf, len);
	ret = i2c_master_send(aw869xx->i2c, data, len + 1);
	if (ret < 0)
		pr_err( "%s: i2c master send error\n", __func__);
	kfree(data);
	return ret;
}

static void aw869xx_haptic_raminit(struct aw869xx *aw869xx, bool flag)
{
	if (flag)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL1,
			AW869XX_BIT_SYSCTRL1_RAMINIT_MASK,
			AW869XX_BIT_SYSCTRL1_RAMINIT_ON);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL1,
			AW869XX_BIT_SYSCTRL1_RAMINIT_MASK,
			AW869XX_BIT_SYSCTRL1_RAMINIT_OFF);

}

static void aw869xx_haptic_play_go(struct aw869xx *aw869xx)
{
	aw869xx_i2c_write(aw869xx, AW869XX_REG_PLAYCFG4, 0x01);
}

static int aw869xx_haptic_stop(struct aw869xx *aw869xx)
{
	unsigned char cnt = MAX_HAPTIC_STOP_CNT;
	unsigned char reg_val = 0;
	bool force_flag = true;

	pr_info("%s enter\n", __func__);
	aw869xx->play_mode = AW869XX_HAPTIC_STANDBY_MODE;
	aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &reg_val);
	if ((reg_val & 0x0f) == 0x00 || (reg_val & 0x0f) == 0x0A) {
		force_flag = false;
		pr_info("%s already in standby mode! glb_state=0x%02X\n",
			__func__, reg_val);
	} else {
		aw869xx_i2c_write(aw869xx, AW869XX_REG_PLAYCFG4, 0x02);
		while (cnt) {
			aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &reg_val);
			if ((reg_val & 0x0f) == 0x00 || (reg_val & 0x0f) == 0x0A) {
				cnt = 0;
				force_flag = false;
				pr_info("%s entered standby! glb_state=0x%02X\n",
					__func__, reg_val);
			} else {
				cnt--;
				pr_debug("%s wait for standby, glb_state=0x%02X\n",
					__func__, reg_val);
			}
			usleep_range(2000, 2500);
		}
	}
	if (force_flag) {
		pr_err("%s force to enter standby mode\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
			AW869XX_BIT_SYSCTRL2_STANDBY_MASK,
			AW869XX_BIT_SYSCTRL2_STANDBY_ON);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
			AW869XX_BIT_SYSCTRL2_STANDBY_MASK,
			AW869XX_BIT_SYSCTRL2_STANDBY_OFF);
		aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSCTRL2, &reg_val);
		pr_info("%s AW869XX_REG_SYSCTRL2 =0x%02X\n", __func__, reg_val);
	}
	return 0;
}

static void aw869xx_container_update(struct aw869xx *aw869xx,
	struct aw869xx_container *aw869xx_cont)
{
	int i;
	unsigned int shift;
	unsigned char reg_val = 0;
	unsigned int temp;

	pr_info("%s enter\n", __func__);
	mutex_lock(&aw869xx->lock);
	aw869xx->ram.baseaddr_shift = 2;
	aw869xx->ram.ram_shift = 4;
	aw869xx_haptic_raminit(aw869xx, true); /* RAMINIT Enable */
	aw869xx_haptic_stop(aw869xx); /* Enter standby mode */

	shift = aw869xx->ram.baseaddr_shift; /* base addr */
	aw869xx->ram.base_addr =
		(unsigned int)((aw869xx_cont->data[0 + shift] << 8) |
		(aw869xx_cont->data[1 + shift]));
	pr_info("%s: base_addr = %d\n", __func__, aw869xx->ram.base_addr);

	aw869xx_i2c_write(aw869xx, AW869XX_REG_RTPCFG1, /*ADDRH*/
		aw869xx_cont->data[0 + shift]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_RTPCFG2, /*ADDRL*/
		aw869xx_cont->data[1 + shift]);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_RTPCFG3, /* FIFO_AEH */
		AW869XX_BIT_RTPCFG3_FIFO_AEH_MASK,
		(unsigned char)(((aw869xx->ram.base_addr >> 1) >> 4) & 0xF0));
	aw869xx_i2c_write(aw869xx, AW869XX_REG_RTPCFG4, /* FIFO AEL */
		(unsigned char)(((aw869xx->ram.base_addr >> 1) & 0x00FF)));
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_RTPCFG3, /* FIFO_AFH */
		AW869XX_BIT_RTPCFG3_FIFO_AFH_MASK,
		(unsigned char)(((aw869xx->ram.base_addr -
		(aw869xx->ram.base_addr >> 2)) >> 8) & 0x0F));
	aw869xx_i2c_write(aw869xx, AW869XX_REG_RTPCFG5, /* FIFO_AFL */
		(unsigned char)((aw869xx->ram.base_addr -
		(aw869xx->ram.base_addr >> 2)) & 0x00FF));
	aw869xx_i2c_read(aw869xx, AW869XX_REG_RTPCFG3, &reg_val);
	temp = ((reg_val & 0x0f) << 24) | ((reg_val & 0xf0) << 4);
	aw869xx_i2c_read(aw869xx, AW869XX_REG_RTPCFG4, &reg_val);
	temp = temp | reg_val;
	pr_info("%s: almost_empty_threshold = %d\n", __func__, (unsigned short)temp);
	aw869xx_i2c_read(aw869xx, AW869XX_REG_RTPCFG5, &reg_val);
	temp = temp | (reg_val << 16);
	pr_info("%s: almost_full_threshold = %d\n", __func__, temp >> 16);
	/* ram */
	shift = aw869xx->ram.baseaddr_shift;
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_RAMADDRH,
		AW869XX_BIT_RAMADDRH_MASK, aw869xx_cont->data[0 + shift]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_RAMADDRL, aw869xx_cont->data[1 + shift]);
	shift = aw869xx->ram.ram_shift;
	for (i = shift; i < aw869xx_cont->len; i++) {
		aw869xx->ram_update_flag = aw869xx_i2c_write(aw869xx,
			AW869XX_REG_RAMDATA, aw869xx_cont->data[i]);
	}
#ifdef AW_CHECK_RAM_DATA
	shift = aw869xx->ram.baseaddr_shift;
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_RAMADDRH,
		AW869XX_BIT_RAMADDRH_MASK, aw869xx_cont->data[0 + shift]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_RAMADDRL,
		aw869xx_cont->data[1 + shift]);
	shift = aw869xx->ram.ram_shift;
	for (i = shift; i < aw869xx_cont->len; i++) {
		aw869xx_i2c_read(aw869xx, AW869XX_REG_RAMDATA, &reg_val);
		if (reg_val != aw869xx_cont->data[i]) {
			pr_err( "%s: ram check error addr=0x%04x, file_data=0x%02X, ram_data=0x%02X\n",
				__func__, i, aw869xx_cont->data[i], reg_val);
			return;
		}
	}
#endif

	aw869xx_haptic_raminit(aw869xx, false); /* RAMINIT Disable */
	mutex_unlock(&aw869xx->lock);
	pr_info("%s exit\n", __func__);
}

static void aw869xx_ram_loaded(const struct firmware *cont, void *context)
{
	struct aw869xx *aw869xx = context;
	struct aw869xx_container *aw869xx_fw = NULL;
	int i = 0;
	unsigned short check_sum = 0;
#ifdef AW_READ_BIN_FLEXBALLY
	static unsigned char load_cont = 0;
	int ram_timer_val = 1000; // max ram timer

	load_cont++;
#endif
	pr_info("%s enter\n", __func__);
	if (!aw869xx)
		return;
	if (!cont) {
		pr_err( "%s: failed to read %s\n", __func__, aw869xx_ram_name);
		release_firmware(cont);
#ifdef AW_READ_BIN_FLEXBALLY
		if (load_cont <= MAX_RAM_LOAD_COUNT) {
			schedule_delayed_work(&aw869xx->ram_work,
				msecs_to_jiffies(ram_timer_val));
			pr_info("%s:start hrtimer: load_cont=%d\n",
				__func__, load_cont);
		}
#endif
		return;
	}
	pr_info("%s: loaded %s - size: %zu bytes\n", __func__,
		aw869xx_ram_name, cont ? cont->size : 0);

	for (i = 2; i < cont->size; i++)
		check_sum += cont->data[i];
	if (check_sum !=
		(unsigned short)((cont->data[0] << 8) | (cont->data[1]))) {
		pr_err("%s: check sum err: check_sum=0x%04x\n", __func__, check_sum);
		return;
	} else {
		pr_info("%s: check sum pass: 0x%04x\n", __func__, check_sum);
		aw869xx->ram.check_sum = check_sum;
	}

	/* aw869xx ram update less then 128kB*/
	aw869xx_fw = kzalloc(cont->size + sizeof(int), GFP_KERNEL);
	if (!aw869xx_fw) {
		release_firmware(cont);
		pr_err( "%s: Error allocating memory\n", __func__);
		return;
	}
	aw869xx_fw->len = cont->size;
	memcpy(aw869xx_fw->data, cont->data, cont->size);
	release_firmware(cont);
	aw869xx_container_update(aw869xx, aw869xx_fw);
	aw869xx->ram.len = aw869xx_fw->len;
	kfree(aw869xx_fw);
	aw869xx->ram_init = 1;
	pr_info("%s: ram firmware update complete!\n", __func__);
}

static int aw869xx_ram_update(struct aw869xx *aw869xx)
{
	aw869xx->ram_init = 0;
	aw869xx->rtp_init = 0;

	return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
		aw869xx_ram_name, aw869xx->dev,
		GFP_KERNEL, aw869xx, aw869xx_ram_loaded);
}

static void aw869xx_ram_work_routine(struct work_struct *work)
{
	struct aw869xx *aw869xx = NULL;
	if (!work)
		return;
	aw869xx = container_of(work, struct aw869xx, ram_work.work);

	if (!aw869xx)
		return;
	pr_info("%s enter\n", __func__);
	aw869xx_ram_update(aw869xx);
}

static int aw869xx_ram_work_init(struct aw869xx *aw869xx)
{
	int ram_timer_val = 8000; // max ram time

	pr_info("%s enter\n", __func__);
	INIT_DELAYED_WORK(&aw869xx->ram_work, aw869xx_ram_work_routine);
	schedule_delayed_work(&aw869xx->ram_work, msecs_to_jiffies(ram_timer_val));
	return 0;
}

static int aw869xx_haptic_play_mode(struct aw869xx *aw869xx,
	unsigned char play_mode)
{
	pr_info("%s enter\n", __func__);

	switch (play_mode) {
	case AW869XX_HAPTIC_STANDBY_MODE:
		pr_info("%s: enter standby mode\n", __func__);
		aw869xx->play_mode = AW869XX_HAPTIC_STANDBY_MODE;
		aw869xx_haptic_stop(aw869xx);
		break;
	case AW869XX_HAPTIC_RAM_MODE:
		pr_info("%s: enter ram mode\n", __func__);
		aw869xx->play_mode = AW869XX_HAPTIC_RAM_MODE;
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_MASK,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_RAM);
		break;
	case AW869XX_HAPTIC_RAM_LOOP_MODE:
		pr_info("%s: enter ram loop mode\n", __func__);
		aw869xx->play_mode = AW869XX_HAPTIC_RAM_LOOP_MODE;
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_MASK,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_RAM);
		break;
	case AW869XX_HAPTIC_RTP_MODE:
		pr_info("%s: enter rtp mode\n", __func__);
		aw869xx->play_mode = AW869XX_HAPTIC_RTP_MODE;
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_MASK,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_RTP);
		break;
	case AW869XX_HAPTIC_TRIG_MODE:
		pr_info("%s: enter trig mode\n", __func__);
		aw869xx->play_mode = AW869XX_HAPTIC_TRIG_MODE;
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_MASK,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_RAM);
		break;
	case AW869XX_HAPTIC_CONT_MODE:
		pr_info("%s: enter cont mode\n", __func__);
		aw869xx->play_mode = AW869XX_HAPTIC_CONT_MODE;
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_MASK,
			AW869XX_BIT_PLAYCFG3_PLAY_MODE_CONT);
		break;
	default:
		pr_err( "%s: play mode %d error", __func__, play_mode);
		break;
	}
	return 0;
}

static int aw869xx_haptic_set_wav_seq(struct aw869xx *aw869xx,
	unsigned char wav, unsigned char seq)
{
	aw869xx_i2c_write(aw869xx, AW869XX_REG_WAVCFG1 + wav, seq);
	return 0;
}

static int aw869xx_haptic_set_wav_loop(struct aw869xx *aw869xx,
	unsigned char wav, unsigned char loop)
{
	unsigned char tmp;

	if (wav % 2) {
		tmp = loop << 0;
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_WAVCFG9 + (wav / 2),
			AW869XX_BIT_WAVLOOP_SEQ_EVEN_MASK, tmp);
	} else {
		tmp = loop << 4;
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_WAVCFG9 + (wav / 2),
			AW869XX_BIT_WAVLOOP_SEQ_ODD_MASK, tmp);
	}
	return 0;
}

static int aw869xx_haptic_set_repeat_wav_seq(struct aw869xx *aw869xx,
	unsigned char seq)
{
	aw869xx_haptic_set_wav_seq(aw869xx, 0x00, seq);
	aw869xx_haptic_set_wav_loop(aw869xx, 0x00,
		AW869XX_BIT_WAVLOOP_INIFINITELY);
	return 0;
}

static int aw869xx_haptic_set_bst_vol(struct aw869xx *aw869xx,
	unsigned char bst_vol)
{
	if (bst_vol & 0xc0) // modify bst vol from 0xc0 to 0x3f
		bst_vol = 0x3f;
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG1,
		AW869XX_BIT_PLAYCFG1_BST_VOUT_RDA_MASK, bst_vol);
	return 0;
}

static int aw869xx_haptic_set_bst_peak_cur(struct aw869xx *aw869xx)
{
	switch (aw869xx->bst_pc) {
	case AW869XX_HAPTIC_BST_PC_L1:
		pr_info("%s bst pc = L1\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_BSTCFG1,
			AW869XX_BIT_BSTCFG1_BST_PC_MASK, (0 << 1));
		return 0;
	case AW869XX_HAPTIC_BST_PC_L2:
		pr_info("%s bst pc = L2\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_BSTCFG1,
			AW869XX_BIT_BSTCFG1_BST_PC_MASK, (5 << 1));
		return 0;
	default:
		pr_info("%s bst pc = L1\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_BSTCFG1,
			AW869XX_BIT_BSTCFG1_BST_PC_MASK, (0 << 1));
		break;
	}
	return 0;
}

static int aw869xx_haptic_set_gain(struct aw869xx *aw869xx,
	unsigned char gain)
{
	aw869xx_i2c_write(aw869xx, AW869XX_REG_PLAYCFG2, gain);
	return 0;
}

static int aw869xx_haptic_set_pwm(struct aw869xx *aw869xx, unsigned char mode)
{
	switch (mode) {
	case AW869XX_PWM_48K:
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
			AW869XX_BIT_SYSCTRL2_WAVDAT_MODE_MASK,
			AW869XX_BIT_SYSCTRL2_RATE_48K);
		break;
	case AW869XX_PWM_24K:
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
			AW869XX_BIT_SYSCTRL2_WAVDAT_MODE_MASK,
			AW869XX_BIT_SYSCTRL2_RATE_24K);
		break;
	case AW869XX_PWM_12K:
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
			AW869XX_BIT_SYSCTRL2_WAVDAT_MODE_MASK,
			AW869XX_BIT_SYSCTRL2_RATE_12K);
		break;
	default:
		break;
	}
	return 0;
}

static int aw869xx_haptic_play_wav_seq(struct aw869xx *aw869xx,
	unsigned char flag)
{
#ifdef AW_RAM_STATE_OUTPUT
	int i;
	unsigned char reg_val = 0;
#endif

	pr_info("%s enter\n", __func__);
	if (flag) {
		aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_RAM_MODE);
		aw869xx_haptic_play_go(aw869xx);
#ifdef AW_RAM_STATE_OUTPUT
		for (i = 0; i < 100; i++) {
			aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &reg_val);
			if ((reg_val & 0x0f) == 0x07) {
				pr_info("%s RAM_GO! glb_state=0x07\n", __func__);
			} else {
				pr_debug("%s ram stopped, glb_state=0x%02X\n",
					__func__, reg_val);
			}
			usleep_range(2000, 2500);

		}
#endif
	}
	return 0;
}

static int aw869xx_haptic_swicth_motor_protect_config(struct aw869xx *aw869xx,
	unsigned char addr, unsigned char val)
{
	pr_info("%s enter\n", __func__);
	if (addr == 1) {
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_DETCFG1,
			AW869XX_BIT_DETCFG1_PRCT_MODE_MASK,
			AW869XX_BIT_DETCFG1_PRCT_MODE_VALID);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PWMCFG1,
			AW869XX_BIT_PWMCFG1_PRC_EN_MASK,
			AW869XX_BIT_PWMCFG1_PRC_ENABLE);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PWMCFG3,
			AW869XX_BIT_PWMCFG3_PR_EN_MASK,
			AW869XX_BIT_PWMCFG3_PR_ENABLE);
	} else if (addr == 0) {
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_DETCFG1,
			AW869XX_BIT_DETCFG1_PRCT_MODE_MASK,
			AW869XX_BIT_DETCFG1_PRCT_MODE_INVALID);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PWMCFG1,
			AW869XX_BIT_PWMCFG1_PRC_EN_MASK,
			AW869XX_BIT_PWMCFG1_PRC_DISABLE);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PWMCFG3,
			AW869XX_BIT_PWMCFG3_PR_EN_MASK,
			AW869XX_BIT_PWMCFG3_PR_DISABLE);
	} else if (addr == 0x2d) {
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PWMCFG1,
			AW869XX_BIT_PWMCFG1_PRCTIME_MASK, val);
	} else if (addr == 0x3e) {
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PWMCFG3,
			AW869XX_BIT_PWMCFG3_PRLVL_MASK, val);
	} else if (addr == 0x3f) {
		aw869xx_i2c_write(aw869xx, AW869XX_REG_PWMCFG4, val);
	}
	return 0;
}

static int aw869xx_haptic_offset_calibration(struct aw869xx *aw869xx)
{
	unsigned int cont = 2000;
	unsigned char reg_val = 0;

	pr_info("%s enter\n", __func__);

	aw869xx_haptic_raminit(aw869xx, true);

	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_DETCFG2,
		AW869XX_BIT_DETCFG2_DIAG_GO_MASK,
		AW869XX_BIT_DETCFG2_DIAG_GO_ON);
	while (1) {
		aw869xx_i2c_read(aw869xx, AW869XX_REG_DETCFG2, &reg_val);
		if ((reg_val & 0x01) == 0 || cont == 0)
			break;
		cont--;
	}
	if (cont == 0)
		pr_err( "%s calibration offset failed!\n", __func__);
	aw869xx_haptic_raminit(aw869xx, false);
	return 0;
}

static int aw869xx_haptic_trig_param_init(struct aw869xx *aw869xx)
{
	pr_info("%s enter\n", __func__);

	aw869xx->trig[0].trig_level = aw869xx->dts_info.trig_config[0];
	aw869xx->trig[0].trig_polar = aw869xx->dts_info.trig_config[1];
	aw869xx->trig[0].pos_enable = aw869xx->dts_info.trig_config[2];
	aw869xx->trig[0].pos_sequence = aw869xx->dts_info.trig_config[3];
	aw869xx->trig[0].neg_enable = aw869xx->dts_info.trig_config[4];
	aw869xx->trig[0].neg_sequence = aw869xx->dts_info.trig_config[5];
	aw869xx->trig[0].trig_brk = aw869xx->dts_info.trig_config[6];
	aw869xx->trig[0].trig_bst = aw869xx->dts_info.trig_config[7];
	if (aw869xx->dts_info.is_enabled_i2s)
		return 0;

	pr_info("%s i2s is disabled!\n", __func__);
	aw869xx->trig[1].trig_level = aw869xx->dts_info.trig_config[8 + 0];
	aw869xx->trig[1].trig_polar = aw869xx->dts_info.trig_config[8 + 1];
	aw869xx->trig[1].pos_enable = aw869xx->dts_info.trig_config[8 + 2];
	aw869xx->trig[1].pos_sequence = aw869xx->dts_info.trig_config[8 + 3];
	aw869xx->trig[1].neg_enable = aw869xx->dts_info.trig_config[8 + 4];
	aw869xx->trig[1].neg_sequence = aw869xx->dts_info.trig_config[8 + 5];
	aw869xx->trig[1].trig_brk = aw869xx->dts_info.trig_config[8 + 6];
	aw869xx->trig[1].trig_bst = aw869xx->dts_info.trig_config[8 + 7];

	aw869xx->trig[2].trig_level = aw869xx->dts_info.trig_config[16 + 0];
	aw869xx->trig[2].trig_polar = aw869xx->dts_info.trig_config[16 + 1];
	aw869xx->trig[2].pos_enable = aw869xx->dts_info.trig_config[16 + 2];
	aw869xx->trig[2].pos_sequence = aw869xx->dts_info.trig_config[16 + 3];
	aw869xx->trig[2].neg_enable = aw869xx->dts_info.trig_config[16 + 4];
	aw869xx->trig[2].neg_sequence = aw869xx->dts_info.trig_config[16 + 5];
	aw869xx->trig[2].trig_brk = aw869xx->dts_info.trig_config[16 + 6];
	aw869xx->trig[2].trig_bst = aw869xx->dts_info.trig_config[16 + 7];

	return 0;
}

static int aw869xx_haptic_trig_param_config(struct aw869xx *aw869xx)
{
	pr_info("%s enter\n", __func__);

	if (aw869xx->trig[0].trig_level)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG1_MODE_MASK,
			AW869XX_BIT_TRGCFG7_TRG1_MODE_LEVEL);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG1_MODE_MASK,
			AW869XX_BIT_TRGCFG7_TRG1_MODE_EDGE);

	if (aw869xx->trig[1].trig_level)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG2_MODE_MASK,
			AW869XX_BIT_TRGCFG7_TRG2_MODE_LEVEL);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG2_MODE_MASK,
			AW869XX_BIT_TRGCFG7_TRG2_MODE_EDGE);

	if (aw869xx->trig[2].trig_level)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG8,
			AW869XX_BIT_TRGCFG8_TRG3_MODE_MASK,
			AW869XX_BIT_TRGCFG8_TRG3_MODE_LEVEL);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG8,
			AW869XX_BIT_TRGCFG8_TRG3_MODE_MASK,
			AW869XX_BIT_TRGCFG8_TRG3_MODE_EDGE);


	if (aw869xx->trig[0].trig_polar)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG1_POLAR_MASK,
			AW869XX_BIT_TRGCFG7_TRG1_POLAR_NEG);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG1_POLAR_MASK,
			AW869XX_BIT_TRGCFG7_TRG1_POLAR_POS);

	if (aw869xx->trig[1].trig_polar)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG2_POLAR_MASK,
			AW869XX_BIT_TRGCFG7_TRG2_POLAR_NEG);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG2_POLAR_MASK,
			AW869XX_BIT_TRGCFG7_TRG2_POLAR_POS);

	if (aw869xx->trig[2].trig_polar)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG8_TRG3_POLAR_MASK,
			AW869XX_BIT_TRGCFG8_TRG3_POLAR_NEG);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG8_TRG3_POLAR_MASK,
			AW869XX_BIT_TRGCFG8_TRG3_POLAR_POS);


	if (aw869xx->trig[0].pos_enable)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG1,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG1,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_DISABLE);

	if (aw869xx->trig[1].pos_enable)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG2,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG2,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_DISABLE);

	if (aw869xx->trig[2].pos_enable)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG3,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG3,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_DISABLE);


	if (aw869xx->trig[0].neg_enable)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG4,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG4,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_DISABLE);

	if (aw869xx->trig[1].neg_enable)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG5,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG5,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_DISABLE);

	if (aw869xx->trig[2].neg_enable)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG6,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG6,
			AW869XX_BIT_TRG_ENABLE_MASK,
			AW869XX_BIT_TRG_DISABLE);

	if (aw869xx->trig[0].pos_sequence)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG1,
			AW869XX_BIT_TRG_SEQ_MASK,
			aw869xx->trig[0].pos_sequence);

	if (aw869xx->trig[0].neg_sequence)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG4,
			AW869XX_BIT_TRG_SEQ_MASK,
			aw869xx->trig[0].neg_sequence);

	if (aw869xx->trig[1].pos_sequence)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG2,
			AW869XX_BIT_TRG_SEQ_MASK,
			aw869xx->trig[1].pos_sequence);

	if (aw869xx->trig[1].neg_sequence)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG5,
			AW869XX_BIT_TRG_SEQ_MASK,
			aw869xx->trig[1].neg_sequence);

	if (aw869xx->trig[2].pos_sequence)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG3,
			AW869XX_BIT_TRG_SEQ_MASK,
			aw869xx->trig[2].pos_sequence);

	if (aw869xx->trig[2].neg_sequence)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG6,
			AW869XX_BIT_TRG_SEQ_MASK,
			aw869xx->trig[2].neg_sequence);

	if (aw869xx->trig[0].trig_brk)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG1_AUTO_BRK_MASK,
			AW869XX_BIT_TRGCFG7_TRG1_AUTO_BRK_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG1_AUTO_BRK_MASK,
			AW869XX_BIT_TRGCFG7_TRG1_AUTO_BRK_DISABLE);

	if (aw869xx->trig[1].trig_brk)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG2_AUTO_BRK_MASK,
			AW869XX_BIT_TRGCFG7_TRG2_AUTO_BRK_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG2_AUTO_BRK_MASK,
			AW869XX_BIT_TRGCFG7_TRG2_AUTO_BRK_DISABLE);

	if (aw869xx->trig[2].trig_brk)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG8,
			AW869XX_BIT_TRGCFG8_TRG3_AUTO_BRK_MASK,
			AW869XX_BIT_TRGCFG8_TRG3_AUTO_BRK_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG8,
			AW869XX_BIT_TRGCFG8_TRG3_AUTO_BRK_MASK,
			AW869XX_BIT_TRGCFG8_TRG3_AUTO_BRK_DISABLE);

	if (aw869xx->trig[0].trig_bst)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG1_BST_MASK,
			AW869XX_BIT_TRGCFG7_TRG1_BST_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG1_BST_MASK,
			AW869XX_BIT_TRGCFG7_TRG1_BST_DISABLE);

	if (aw869xx->trig[1].trig_bst)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG2_BST_MASK,
			AW869XX_BIT_TRGCFG7_TRG2_BST_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG7,
			AW869XX_BIT_TRGCFG7_TRG2_BST_MASK,
			AW869XX_BIT_TRGCFG7_TRG2_BST_DISABLE);

	if (aw869xx->trig[2].trig_bst)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG8,
			AW869XX_BIT_TRGCFG8_TRG3_BST_MASK,
			AW869XX_BIT_TRGCFG8_TRG3_BST_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRGCFG8,
			AW869XX_BIT_TRGCFG8_TRG3_BST_MASK,
			AW869XX_BIT_TRGCFG8_TRG3_BST_DISABLE);
	return 0;
}

static void aw869xx_haptic_bst_mode_config(struct aw869xx *aw869xx,
	unsigned char boost_mode)
{
	aw869xx->boost_mode = boost_mode;

	switch (boost_mode) {
	case AW869XX_HAPTIC_BST_MODE_BOOST:
		pr_info("%s haptic boost mode = boost\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG1,
			AW869XX_BIT_PLAYCFG1_BST_MODE_MASK,
			AW869XX_BIT_PLAYCFG1_BST_MODE_BOOST);
		break;
	case AW869XX_HAPTIC_BST_MODE_BYPASS:
		pr_info("%s haptic boost mode = bypass\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG1,
			AW869XX_BIT_PLAYCFG1_BST_MODE_MASK,
			AW869XX_BIT_PLAYCFG1_BST_MODE_BYPASS);
		break;
	default:
		pr_err( "%s: boost_mode = %d error", __func__, boost_mode);
		break;
	}
}

static int aw869xx_haptic_auto_bst_enable(struct aw869xx *aw869xx,
	unsigned char flag)
{
	aw869xx->auto_boost = flag;
	if (flag)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
			AW869XX_BIT_PLAYCFG3_AUTO_BST_MASK,
			AW869XX_BIT_PLAYCFG3_AUTO_BST_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
			AW869XX_BIT_PLAYCFG3_AUTO_BST_MASK,
			AW869XX_BIT_PLAYCFG3_AUTO_BST_DISABLE);
	return 0;
}

static int aw869xx_haptic_vbat_mode_config(struct aw869xx *aw869xx,
	unsigned char flag)
{
	if (flag == AW869XX_HAPTIC_CONT_VBAT_HW_ADJUST_MODE)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL1,
			AW869XX_BIT_SYSCTRL1_VBAT_MODE_MASK,
			AW869XX_BIT_SYSCTRL1_VBAT_MODE_HW);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL1,
			AW869XX_BIT_SYSCTRL1_VBAT_MODE_MASK,
			AW869XX_BIT_SYSCTRL1_VBAT_MODE_SW);
	return 0;
}

static int aw869xx_haptic_get_vbat(struct aw869xx *aw869xx)
{
	unsigned char reg_val = 0;
	unsigned int vbat_code = 0;

	aw869xx_haptic_stop(aw869xx);
	aw869xx_haptic_raminit(aw869xx, true);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_DETCFG2,
		AW869XX_BIT_DETCFG2_VBAT_GO_MASK, AW869XX_BIT_DETCFG2_VABT_GO_ON);
	usleep_range(20000, 25000);
	aw869xx_i2c_read(aw869xx, AW869XX_REG_DET_VBAT, &reg_val);
	vbat_code = (vbat_code | reg_val) << 2;
	aw869xx_i2c_read(aw869xx, AW869XX_REG_DET_LO, &reg_val);
	vbat_code = vbat_code | ((reg_val & 0x30) >> 4);
	aw869xx->vbat = 6100 * vbat_code / 1024;
	if (aw869xx->vbat > AW869XX_VBAT_MAX) {
		aw869xx->vbat = AW869XX_VBAT_MAX;
		pr_info("%s vbat max limit = %dmV\n", __func__, aw869xx->vbat);
	}
	if (aw869xx->vbat < AW869XX_VBAT_MIN) {
		aw869xx->vbat = AW869XX_VBAT_MIN;
		pr_info("%s vbat min limit = %dmV\n", __func__, aw869xx->vbat);
	}
	pr_info("%s aw869xx->vbat=%dmV, vbat_code=0x%02X\n",
		__func__, aw869xx->vbat, vbat_code);
	aw869xx_haptic_raminit(aw869xx, false);
	return 0;
}

static int aw869xx_haptic_ram_vbat_compensate(struct aw869xx *aw869xx,
	bool flag)
{
	int temp_gain = 0;

	if (!flag) {
		aw869xx_haptic_set_gain(aw869xx, aw869xx->gain);
		return 0;
	}

	if (aw869xx->ram_vbat_compensate == AW869XX_HAPTIC_RAM_VBAT_COMP_ENABLE) {
		aw869xx_haptic_get_vbat(aw869xx);
		temp_gain = aw869xx->gain * AW869XX_VBAT_REFER / aw869xx->vbat;
		if (temp_gain > (128 * AW869XX_VBAT_REFER / AW869XX_VBAT_MIN)) {
			temp_gain = 128 * AW869XX_VBAT_REFER / AW869XX_VBAT_MIN;
			pr_debug("%s gain limit=%d\n", __func__, temp_gain);
		}
		aw869xx_haptic_set_gain(aw869xx, temp_gain);
	} else {
		aw869xx_haptic_set_gain(aw869xx, aw869xx->gain);
	}

	return 0;
}

static int aw869xx_haptic_get_lra_resistance(struct aw869xx *aw869xx)
{
	unsigned char reg_val = 0;
	unsigned char d2s_gain_temp;
	unsigned int lra_code = 0;

	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_stop(aw869xx);
	aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSCTRL7, &reg_val);
	d2s_gain_temp = 0x07 & reg_val;
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL7,
		AW869XX_BIT_SYSCTRL7_D2S_GAIN_MASK,
		aw869xx->dts_info.d2s_gain);
	aw869xx_haptic_raminit(aw869xx, true);
	/* enter standby mode */
	aw869xx_haptic_stop(aw869xx);
	usleep_range(2000, 2500);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
		AW869XX_BIT_SYSCTRL2_STANDBY_MASK,
		AW869XX_BIT_SYSCTRL2_STANDBY_OFF);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_DETCFG1,
		AW869XX_BIT_DETCFG1_RL_OS_MASK,
		AW869XX_BIT_DETCFG1_RL);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_DETCFG2,
		AW869XX_BIT_DETCFG2_DIAG_GO_MASK,
		AW869XX_BIT_DETCFG2_DIAG_GO_ON);
	usleep_range(30000, 35000);
	aw869xx_i2c_read(aw869xx, AW869XX_REG_DET_RL, &reg_val);
	lra_code = (lra_code | reg_val) << 2;
	aw869xx_i2c_read(aw869xx, AW869XX_REG_DET_LO, &reg_val);
	lra_code = lra_code | (reg_val & 0x03);
	/* 2num */
	aw869xx->lra = (lra_code * 678 * 1000) / (1024 * 10); // lra calc algo
	aw869xx_haptic_raminit(aw869xx, false);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL7,
		AW869XX_BIT_SYSCTRL7_D2S_GAIN_MASK, d2s_gain_temp);
	mutex_unlock(&aw869xx->lock);
	return 0;
}

static void aw869xx_haptic_misc_para_init(struct aw869xx *aw869xx)
{
	pr_info("%s enter\n", __func__);
	aw869xx->cont_drv1_lvl = aw869xx->dts_info.cont_drv1_lvl_dt;
	aw869xx->cont_drv2_lvl = aw869xx->dts_info.cont_drv2_lvl_dt;
	aw869xx->cont_drv1_time = aw869xx->dts_info.cont_drv1_time_dt;
	aw869xx->cont_drv2_time = aw869xx->dts_info.cont_drv2_time_dt;
	aw869xx->cont_brk_time = aw869xx->dts_info.cont_brk_time_dt;
	aw869xx->cont_wait_num = aw869xx->dts_info.cont_wait_num_dt;
	aw869xx_i2c_write(aw869xx, AW869XX_REG_BSTCFG1,
		aw869xx->dts_info.bstcfg[0]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_BSTCFG2,
		aw869xx->dts_info.bstcfg[1]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_BSTCFG3,
		aw869xx->dts_info.bstcfg[2]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_BSTCFG4,
		aw869xx->dts_info.bstcfg[3]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_BSTCFG5,
		aw869xx->dts_info.bstcfg[4]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_SYSCTRL3,
		aw869xx->dts_info.sine_array[0]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_SYSCTRL4,
		aw869xx->dts_info.sine_array[1]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_SYSCTRL5,
		aw869xx->dts_info.sine_array[2]);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_SYSCTRL6,
		aw869xx->dts_info.sine_array[3]);
	/* d2s_gain */
	if (!aw869xx->dts_info.d2s_gain)
		pr_err( "%s aw869xx->dts_info.d2s_gain = 0!\n", __func__);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL7,
			AW869XX_BIT_SYSCTRL7_D2S_GAIN_MASK,
			aw869xx->dts_info.d2s_gain);

	/* cont_tset */
	if (!aw869xx->dts_info.cont_tset)
		pr_err("%s aw869xx->dts_info.cont_tset = 0\n", __func__);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG13,
			AW869XX_BIT_CONTCFG13_TSET_MASK,
			aw869xx->dts_info.cont_tset << 4);

	/* cont_bemf_set */
	if (!aw869xx->dts_info.cont_bemf_set)
		pr_err("%s aw869xx->dts_info.cont_bemf_set = 0!\n", __func__);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG13,
			AW869XX_BIT_CONTCFG13_BEME_SET_MASK,
			aw869xx->dts_info.cont_bemf_set);

	/* cont_brk_time */
	if (!aw869xx->cont_brk_time)
		pr_err( "%s aw869xx->cont_brk_time = 0!\n", __func__);
	else
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG10,
			aw869xx->cont_brk_time);

	if (!aw869xx->dts_info.cont_bst_brk_gain) /* cont_bst_brk_gain */
		pr_err("%s aw869xx->dts_info.cont_bst_brk_gain = 0!\n", __func__);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG5,
			AW869XX_BIT_CONTCFG5_BST_BRK_GAIN_MASK,
			aw869xx->dts_info.cont_bst_brk_gain);

	if (!aw869xx->dts_info.cont_brk_gain) /* cont_brk_gain */
		pr_err("%s aw869xx->dts_info.cont_brk_gain = 0\n", __func__);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG5,
			AW869XX_BIT_CONTCFG5_BRK_GAIN_MASK,
			aw869xx->dts_info.cont_brk_gain);

	/* i2s enbale */
	if (aw869xx->dts_info.is_enabled_i2s) {
		pr_info("%s i2s enabled!\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
			AW869XX_BIT_SYSCTRL2_I2S_PIN_MASK,
			AW869XX_BIT_SYSCTRL2_I2S_PIN_I2S);
	} else {
		pr_info("%s i2s disabled!\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
			AW869XX_BIT_SYSCTRL2_I2S_PIN_MASK,
			AW869XX_BIT_SYSCTRL2_I2S_PIN_TRIG);
	}
}

static void aw869xx_haptic_set_rtp_aei(struct aw869xx *aw869xx, bool flag)
{
	if (flag)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSINTM,
			AW869XX_BIT_SYSINTM_FF_AEM_MASK,
			AW869XX_BIT_SYSINTM_FF_AEM_ON);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSINTM,
			AW869XX_BIT_SYSINTM_FF_AEM_MASK,
			AW869XX_BIT_SYSINTM_FF_AEM_OFF);
}

static unsigned char aw869xx_haptic_rtp_get_fifo_afs(struct aw869xx *aw869xx)
{
	unsigned char ret;
	unsigned char reg_val = 0;

	aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSST, &reg_val);
	reg_val &= AW869XX_BIT_SYSST_FF_AFS;
	ret = reg_val >> 3;
	return ret;
}

static int aw869xx_haptic_rtp_init(struct aw869xx *aw869xx)
{
	unsigned int buf_len = 0;
	unsigned char glb_state_val = 0;

	pr_info("%s enter\n", __func__);
	pm_qos_add_request(&pm_qos_req_vb, PM_QOS_CPU_DMA_LATENCY,
		PM_QOS_VALUE_VB);
	aw869xx->rtp_cnt = 0;
	mutex_lock(&aw869xx->rtp_lock);
	while ((!aw869xx_haptic_rtp_get_fifo_afs(aw869xx))
		&& (aw869xx->play_mode == AW869XX_HAPTIC_RTP_MODE)) {
		pr_info("%s rtp cnt = %d\n", __func__, aw869xx->rtp_cnt);
		if (!aw869xx_rtp) {
			pr_info("%s:aw869xx_rtp is null, break!\n", __func__);
			break;
		}
		if (aw869xx->rtp_cnt < (aw869xx->ram.base_addr)) {
			if ((aw869xx_rtp->len - aw869xx->rtp_cnt) <
				(aw869xx->ram.base_addr)) {
				buf_len = aw869xx_rtp->len - aw869xx->rtp_cnt;
			} else {
				buf_len = aw869xx->ram.base_addr;
			}
		} else if ((aw869xx_rtp->len - aw869xx->rtp_cnt) <
				(aw869xx->ram.base_addr >> 2)) {
			buf_len = aw869xx_rtp->len - aw869xx->rtp_cnt;
		} else {
			buf_len = aw869xx->ram.base_addr >> 2;
		}
		pr_info("%s buf_len = %d\n", __func__, buf_len);
		aw869xx_i2c_writes(aw869xx, AW869XX_REG_RTPDATA,
			&aw869xx_rtp->data[aw869xx->rtp_cnt], buf_len);
		aw869xx->rtp_cnt += buf_len;
		aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &glb_state_val);
		if ((aw869xx->rtp_cnt == aw869xx_rtp->len) ||
			((glb_state_val & 0x0f) == 0x00)) {
			pr_info("%s: rtp update complete!\n", __func__);
			aw869xx->rtp_cnt = 0;
			pm_qos_remove_request(&pm_qos_req_vb);
			mutex_unlock(&aw869xx->rtp_lock);
			return 0;
		}
	}
	mutex_unlock(&aw869xx->rtp_lock);

	if (aw869xx->play_mode == AW869XX_HAPTIC_RTP_MODE)
		aw869xx_haptic_set_rtp_aei(aw869xx, true);

	pr_info("%s exit\n", __func__);
	pm_qos_remove_request(&pm_qos_req_vb);
	return 0;
}

static void aw869xx_haptic_upload_lra(struct aw869xx *aw869xx,
	unsigned int flag)
{
	switch (flag) {
	case WRITE_ZERO:
		pr_info("%s write zero to trim_lra!\n", __func__);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRIMCFG3,
			AW869XX_BIT_TRIMCFG3_TRIM_LRA_MASK, 0x00);
		break;
	case F0_CALI:
		pr_info("%s write f0_cali_data to trim_lra = 0x%02X\n",
			__func__, aw869xx->f0_cali_data);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRIMCFG3,
			AW869XX_BIT_TRIMCFG3_TRIM_LRA_MASK,
			(char)aw869xx->f0_cali_data);
		break;
	case OSC_CALI:
		pr_info("%s write osc_cali_data to trim_lra = 0x%02X\n",
			__func__, aw869xx->osc_cali_data);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRIMCFG3,
			AW869XX_BIT_TRIMCFG3_TRIM_LRA_MASK,
			(char)aw869xx->osc_cali_data);
		break;
	default:
		break;
	}
}

static int aw869xx_osc_trim_calculation(unsigned long int theory_time,
	unsigned long int real_time)
{
	unsigned int real_code = 0;
	unsigned int lra_code = 0;
	unsigned int DFT_LRA_TRIM_CODE = 0;
	/*0.1 percent below no need to calibrate */
	unsigned int osc_cali_threshold = 10;

	pr_info("%s enter\n", __func__);
	if (theory_time == real_time) {
		pr_info("%s theory_time == real_time: %ld,"
			" no need to calibrate!\n", __func__, real_time);
		return 0;
	} else if (theory_time < real_time) {
		if ((real_time - theory_time) > (theory_time / 50)) {
			pr_info("%s (real_time - theory_time) >"
				" (theory_time/50), can't calibrate!\n",
				__func__);
			return DFT_LRA_TRIM_CODE;
		}

		if ((real_time - theory_time) <
			(osc_cali_threshold * theory_time / 10000)) {
			pr_info("%s real_time: %ld, theory_time: %ld,"
				" no need to calibrate!\n", __func__,
				real_time, theory_time);
			return DFT_LRA_TRIM_CODE;
		}

		real_code = ((real_time - theory_time) * 4000) / theory_time;
		real_code = ((real_code % 10 < 5) ? 0 : 1) + real_code / 10;
		real_code = 32 + real_code;
	} else if (theory_time > real_time) {
		if ((theory_time - real_time) > (theory_time / 50)) {
			pr_info("%s (theory_time - real_time) >"
				" (theory_time / 50), can't calibrate!\n",
				__func__);
			return DFT_LRA_TRIM_CODE;
		}
		if ((theory_time - real_time) <
			(osc_cali_threshold * theory_time / 10000)) {
			pr_info("%s real_time: %ld, theory_time: %ld,"
				" no need to calibrate!\n", __func__,
				real_time, theory_time);
			return DFT_LRA_TRIM_CODE;
		}
		real_code = ((theory_time - real_time) * 4000) / theory_time;
		real_code = ((real_code % 10 < 5) ? 0 : 1) + real_code / 10;
		real_code = 32 - real_code;
	}
	if (real_code > 31)
		lra_code = real_code - 32;
	else
		lra_code = real_code + 32;
	pr_info("%s real_time: %ld, theory_time: %ld\n",__func__, real_time,
		theory_time);
	pr_info("%s real_code: %02X, trim_lra: 0x%02X\n", __func__, real_code,
		lra_code);
	return lra_code;
}

static int aw869xx_rtp_trim_lra_calibration(struct aw869xx *aw869xx)
{
	unsigned char reg_val = 0;
	unsigned int fre_val = 0;
	unsigned int theory_time = 0;
	unsigned int lra_trim_code = 0;

	aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSCTRL2, &reg_val);
	fre_val = (reg_val & 0x03) >> 0;

	if (fre_val == 2 || fre_val == 3)
		theory_time = (aw869xx->rtp_len / 12000) * 1000000; /*12K */
	if (fre_val == 0)
		theory_time = (aw869xx->rtp_len / 24000) * 1000000; /*24K */
	if (fre_val == 1)
		theory_time = (aw869xx->rtp_len / 48000) * 1000000; /*48K */

	pr_info("%s microsecond:%ld  theory_time = %d\n",
		__func__, aw869xx->microsecond, theory_time);
	if (theory_time == 0) {
		pr_err( "%s: error theory_time can not to be zero\n", __func__);
		return -EINVAL;
	}
	lra_trim_code = aw869xx_osc_trim_calculation(theory_time,
						     aw869xx->microsecond);
	if (lra_trim_code >= 0) {
		aw869xx->osc_cali_data = lra_trim_code;
		aw869xx_haptic_upload_lra(aw869xx, OSC_CALI);
	}
	return 0;
}

static unsigned char aw869xx_haptic_osc_read_status(struct aw869xx *aw869xx)
{
	unsigned char reg_val = 0;

	aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSST2, &reg_val);
	return reg_val;
}

static int aw869xx_rtp_osc_calibration(struct aw869xx *aw869xx)
{
	const struct firmware *rtp_file = NULL;
	int ret = -1;
	unsigned int buf_len = 0;
	unsigned char osc_int_state = 0;

	aw869xx->rtp_cnt = 0;
	aw869xx->timeval_flags = 1;

	pr_info("%s enter\n", __func__);
	/* fw loaded */
	ret = request_firmware(&rtp_file, aw869xx_rtp_name[0], aw869xx->dev);
	if (ret < 0) {
		pr_err( "%s: failed to read %s\n", __func__, aw869xx_rtp_name[0]);
		return ret;
	}
	/*awinic add stop,for irq interrupt during calibrate */
	aw869xx_haptic_stop(aw869xx);
	aw869xx->rtp_init = 0;
	mutex_lock(&aw869xx->rtp_lock);
	vfree(aw869xx_rtp);
	aw869xx_rtp = vmalloc(rtp_file->size + sizeof(int));
	if (!aw869xx_rtp) {
		release_firmware(rtp_file);
		mutex_unlock(&aw869xx->rtp_lock);
		pr_err( "%s: error allocating memory\n", __func__);
		return -ENOMEM;
	}
	aw869xx_rtp->len = rtp_file->size;
	aw869xx->rtp_len = rtp_file->size;
	pr_info("%s: rtp file:[%s] size = %dbytes\n", __func__,
		aw869xx_rtp_name[0], aw869xx_rtp->len);

	memcpy(aw869xx_rtp->data, rtp_file->data, rtp_file->size);
	release_firmware(rtp_file);
	mutex_unlock(&aw869xx->rtp_lock);
	/* gain */
	aw869xx_haptic_ram_vbat_compensate(aw869xx, false);
	/* rtp mode config */
	aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_RTP_MODE);
	/* bst mode */
	aw869xx_haptic_bst_mode_config(aw869xx, AW869XX_HAPTIC_BST_MODE_BYPASS);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL7,
		AW869XX_BIT_SYSCTRL7_INT_MODE_MASK,
		AW869XX_BIT_SYSCTRL7_INT_MODE_EDGE);
	disable_irq(gpio_to_irq(aw869xx->irq_gpio));
#ifdef AW_OSC_COARSE_CALI
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRIMCFG3,
		AW869XX_BIT_TRIMCFG3_OSC_TRIM_SRC_MASK,
		AW869XX_BIT_TRIMCFG3_OSC_TRIM_SRC_REG);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_TRIMCFG4, 0xF4);
#endif
	/* haptic go */
	aw869xx_haptic_play_go(aw869xx);
	/* require latency of CPU & DMA not more then PM_QOS_VALUE_VB us */
	pm_qos_add_request(&pm_qos_req_vb, PM_QOS_CPU_DMA_LATENCY,
		PM_QOS_VALUE_VB);
	while (1) {
		if (!aw869xx_haptic_rtp_get_fifo_afs(aw869xx)) {
			pr_info("%s not almost_full,"
				" aw869xx->rtp_cnt= %d\n", __func__,
				aw869xx->rtp_cnt);
			mutex_lock(&aw869xx->rtp_lock);
			if ((aw869xx_rtp->len - aw869xx->rtp_cnt) <
				(aw869xx->ram.base_addr >> 2))
				buf_len = aw869xx_rtp->len - aw869xx->rtp_cnt;
			else
				buf_len = (aw869xx->ram.base_addr >> 2);

			if (aw869xx->rtp_cnt != aw869xx_rtp->len) {
				if (aw869xx->timeval_flags == 1) {
					do_gettimeofday(&aw869xx->start);
					aw869xx->timeval_flags = 0;
				}
				aw869xx->rtp_update_flag =
					aw869xx_i2c_writes(aw869xx,
						AW869XX_REG_RTPDATA,
						&aw869xx_rtp->data[aw869xx->rtp_cnt],
						buf_len);
				aw869xx->rtp_cnt += buf_len;
			}
			mutex_unlock(&aw869xx->rtp_lock);
		}
		osc_int_state = aw869xx_haptic_osc_read_status(aw869xx);
		if (osc_int_state & AW869XX_BIT_SYSST2_FF_EMPTY) {
			do_gettimeofday(&aw869xx->end);
			pr_info("%s osc trim playback done aw869xx->rtp_cnt= %d\n",
				__func__, aw869xx->rtp_cnt);
			break;
		}
		do_gettimeofday(&aw869xx->end);
		aw869xx->microsecond =
			(aw869xx->end.tv_sec - aw869xx->start.tv_sec) * 1000000 +
			(aw869xx->end.tv_usec - aw869xx->start.tv_usec);
		if (aw869xx->microsecond > OSC_CALI_MAX_LENGTH) {
			pr_info("%s osc trim time out!"
				" aw869xx->rtp_cnt %d osc_int_state %02x\n",
				__func__, aw869xx->rtp_cnt, osc_int_state);
			break;
		}
	}
	pm_qos_remove_request(&pm_qos_req_vb);
	enable_irq(gpio_to_irq(aw869xx->irq_gpio));

	aw869xx->microsecond =
		(aw869xx->end.tv_sec - aw869xx->start.tv_sec) * 1000000 +
		(aw869xx->end.tv_usec - aw869xx->start.tv_usec);
	/*calibration osc */
	pr_info("%s awinic_microsecond: %ld\n", __func__, aw869xx->microsecond);
	pr_info("%s exit\n", __func__);
	return 0;
}

static void aw869xx_rtp_work_routine(struct work_struct *work)
{
	const struct firmware *rtp_file = NULL;
	int ret = -1;
	unsigned int cnt = 200;
	unsigned char reg_val = 0;
	bool rtp_work_flag = false;
	struct aw869xx *aw869xx = container_of(work, struct aw869xx, rtp_work);

	pr_info("%s enter\n", __func__);
	mutex_lock(&aw869xx->rtp_lock);
	/* fw loaded */
	ret = request_firmware(&rtp_file, aw869xx_rtp_idx_name, aw869xx->dev);
	if (ret < 0) {
		pr_err( "%s: failed to read %s\n", __func__, aw869xx_rtp_idx_name);
		mutex_unlock(&aw869xx->rtp_lock);
		return;
	}
	aw869xx->rtp_init = 0;
	vfree(aw869xx_rtp);
	aw869xx_rtp = vmalloc(rtp_file->size + sizeof(int));
	if (!aw869xx_rtp) {
		release_firmware(rtp_file);
		pr_err( "%s: error allocating memory\n", __func__);
		mutex_unlock(&aw869xx->rtp_lock);
		return;
	}
	aw869xx_rtp->len = rtp_file->size;
	pr_info("%s: rtp file:[%s] size = %dbytes\n", __func__,
		aw869xx_rtp_idx_name, aw869xx_rtp->len);
	memcpy(aw869xx_rtp->data, rtp_file->data, rtp_file->size);
	mutex_unlock(&aw869xx->rtp_lock);
	release_firmware(rtp_file);
	mutex_lock(&aw869xx->lock);
	aw869xx->rtp_init = 1;
	aw869xx_haptic_set_pwm(aw869xx, AW869XX_PWM_24K);
	aw869xx_haptic_upload_lra(aw869xx, F0_CALI);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_SYSCTRL7, aw869xx->rtp_d2s_gain);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG5, aw869xx->rtp_brk_gain);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG10, aw869xx->rtp_brk_time);

	/* gain */
	aw869xx_haptic_ram_vbat_compensate(aw869xx, false);
	/* rtp mode config */
	aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_RTP_MODE);
	/* bst mode config */
	aw869xx_haptic_bst_mode_config(aw869xx, AW869XX_HAPTIC_BST_MODE_BOOST);
	/* haptic go */
	aw869xx_haptic_play_go(aw869xx);
	mutex_unlock(&aw869xx->lock);
	usleep_range(2000, 2500);
	while (cnt) {
		aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &reg_val);
		if ((reg_val & 0x0f) == 0x08) {
			cnt = 0;
			rtp_work_flag = true;
			pr_info("%s RTP_GO! glb_state=0x08\n", __func__);
		} else {
			cnt--;
			pr_debug("%s wait for RTP_GO, glb_state=0x%02X\n",
				__func__, reg_val);
		}
		usleep_range(2000, 2500);
	}
	if (rtp_work_flag) {
		aw869xx_haptic_rtp_init(aw869xx);
	} else {
		/* enter standby mode */
		aw869xx_haptic_stop(aw869xx);
		pr_err( "%s failed to enter RTP_GO status!\n", __func__);
	}
}

static int aw869xx_haptic_cont_config(struct aw869xx *aw869xx)
{
	pr_info("%s enter\n", __func__);

	/* work mode */
	aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_CONT_MODE);
	/* bst mode */
	aw869xx_haptic_bst_mode_config(aw869xx, AW869XX_HAPTIC_BST_MODE_BYPASS);

	/* cont config */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG6,
		AW869XX_BIT_CONTCFG6_TRACK_EN_MASK,
		AW869XX_BIT_CONTCFG6_TRACK_ENABLE);
	/* f0 driver level */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG6,
		AW869XX_BIT_CONTCFG6_DRV1_LVL_MASK,
		aw869xx->cont_drv1_lvl);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG7,
		aw869xx->cont_drv2_lvl);
	/* DRV1_TIME */
	/* aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG8, 0xFF); */
	/* DRV2_TIME */
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG9, 0xFF);
	/* cont play go */
	aw869xx_haptic_play_go(aw869xx);
	return 0;
}

/*****************************************************
 *
 * haptic f0 cali
 *
 *****************************************************/
static int aw869xx_haptic_read_lra_f0(struct aw869xx *aw869xx)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int f0_reg = 0;
	unsigned long f0_tmp = 0;

	pr_info("%s enter\n", __func__);
	/* F_LRA_F0_H */
	ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_CONTRD14, &reg_val);
	f0_reg = (f0_reg | reg_val) << 8;
	/* F_LRA_F0_L */
	ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_CONTRD15, &reg_val);
	f0_reg |= (reg_val << 0);
	if (!f0_reg) {
		aw869xx->f0_val_err = -1;
		pr_err( "%s didn't get lra f0 because f0_reg value is 0!\n",
			__func__);
		aw869xx->f0 = aw869xx->dts_info.f0_ref;
		return -1;
	} else {
		f0_tmp = 384000 * 10 / f0_reg;
		aw869xx->f0 = (unsigned int)f0_tmp;
		pr_info("%s lra_f0=%d\n", __func__, aw869xx->f0);
	}

	return 0;
}

static int aw869xx_haptic_read_cont_f0(struct aw869xx *aw869xx)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int f0_reg = 0;
	unsigned long f0_tmp = 0;

	pr_info("%s enter\n", __func__);
	ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_CONTRD16, &reg_val);
	f0_reg = (f0_reg | reg_val) << 8;
	ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_CONTRD17, &reg_val);
	f0_reg |= (reg_val << 0);
	if (!f0_reg) {
		pr_err("%s didn't get cont f0 because f0_reg value is 0!\n",
			__func__);
		aw869xx->f0_val_err = -1;
		aw869xx->cont_f0 = aw869xx->dts_info.f0_ref;
		return -1;
	} else {
		f0_tmp = 384000 * 10 / f0_reg;
		aw869xx->cont_f0 = (unsigned int)f0_tmp;
		pr_info("%s cont_f0=%d\n", __func__, aw869xx->cont_f0);
	}
	return 0;
}

static int aw869xx_haptic_ram_get_f0(struct aw869xx *aw869xx)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int cnt = 200;
	bool get_f0_flag = false;
	unsigned char brk_en_temp = 0;

	pr_info("%s enter\n", __func__);
	aw869xx->f0 = aw869xx->dts_info.f0_ref;
	/* enter standby mode */
	aw869xx_haptic_stop(aw869xx);
	/* f0 calibrate work mode */
	aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_RAM_MODE);
	/* bst mode */
	aw869xx_haptic_bst_mode_config(aw869xx, AW869XX_HAPTIC_BST_MODE_BYPASS);
	/* enable f0 detect */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG1,
		AW869XX_BIT_CONTCFG1_EN_F0_DET_MASK,
		AW869XX_BIT_CONTCFG1_F0_DET_ENABLE);
	/* cont config */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG6,
		AW869XX_BIT_CONTCFG6_TRACK_EN_MASK,
		AW869XX_BIT_CONTCFG6_TRACK_ENABLE);
	/* enable auto break */
	aw869xx_i2c_read(aw869xx, AW869XX_REG_PLAYCFG3, &reg_val);
	brk_en_temp = 0x04 & reg_val;
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
		AW869XX_BIT_PLAYCFG3_BRK_EN_MASK,
		AW869XX_BIT_PLAYCFG3_BRK_ENABLE);
	aw869xx_haptic_set_wav_seq(aw869xx, 0x00, 0x01);
	aw869xx_haptic_set_wav_loop(aw869xx, 0x00, 0x0A);
	/* ram play go */
	aw869xx_haptic_play_go(aw869xx);
	/* 300ms */
	while (cnt) {
		aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &reg_val);
		if ((reg_val & 0x0f) == 0x00) {
			cnt = 0;
			get_f0_flag = true;
			pr_info("%s entered standby mode! glb_state=0x%02X\n",
				__func__, reg_val);
		} else {
			cnt--;
			pr_debug("%s waitting for standby, glb_state=0x%02X\n",
				__func__, reg_val);
		}
		usleep_range(10000, 10500);
	}
	if (get_f0_flag) {
		aw869xx_haptic_read_lra_f0(aw869xx);
	} else {
		pr_err("%s enter standby mode failed, stop reading f0!\n",
			__func__);
	}
	/* restore default config */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG1,
		AW869XX_BIT_CONTCFG1_EN_F0_DET_MASK,
		AW869XX_BIT_CONTCFG1_F0_DET_DISABLE);
	/* recover auto break config */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
		AW869XX_BIT_PLAYCFG3_BRK_EN_MASK,
		brk_en_temp << AW869XX_BREAK_EN);
	return ret;
}

static int aw869xx_haptic_cont_get_f0(struct aw869xx *aw869xx)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int cnt = 200;
	bool get_f0_flag = false;
	unsigned char brk_en_temp = 0;

	pr_info("%s enter\n", __func__);
	aw869xx->f0 = aw869xx->dts_info.f0_ref;
	/* enter standby mode */
	aw869xx_haptic_stop(aw869xx);
	/* f0 calibrate work mode */
	aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_CONT_MODE);
	/* bst mode */
	aw869xx_haptic_bst_mode_config(aw869xx, AW869XX_HAPTIC_BST_MODE_BYPASS);
	/* enable f0 detect */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG1,
		AW869XX_BIT_CONTCFG1_EN_F0_DET_MASK,
		AW869XX_BIT_CONTCFG1_F0_DET_ENABLE);
	/* cont config */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG6,
		AW869XX_BIT_CONTCFG6_TRACK_EN_MASK,
		AW869XX_BIT_CONTCFG6_TRACK_ENABLE);
	/* enable auto break */
	aw869xx_i2c_read(aw869xx, AW869XX_REG_PLAYCFG3, &reg_val);
	brk_en_temp = 0x04 & reg_val;
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
		AW869XX_BIT_PLAYCFG3_BRK_EN_MASK,
		AW869XX_BIT_PLAYCFG3_BRK_ENABLE);
	/* f0 driver level */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG6,
		AW869XX_BIT_CONTCFG6_DRV1_LVL_MASK,
		aw869xx->dts_info.cont_drv1_lvl_dt);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG7,
		aw869xx->dts_info.cont_drv2_lvl_dt);
	/* DRV1_TIME */
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG8,
		aw869xx->dts_info.cont_drv1_time_dt);
	/* DRV2_TIME */
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG9,
		aw869xx->dts_info.cont_drv2_time_dt);
	/* TRACK_MARGIN */
	if (!aw869xx->dts_info.cont_track_margin)
		pr_err("%s aw869xx->dts_info.cont_track_margin = 0\n", __func__);
	else
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG11,
			(unsigned char)aw869xx->dts_info.cont_track_margin);
	aw869xx_haptic_play_go(aw869xx);
	/* 300ms */
	while (cnt) {
		aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &reg_val);
		if ((reg_val & 0x0f) == 0x00) {
			cnt = 0;
			get_f0_flag = true;
			pr_info("%s entered standby mode! glb_state=0x%02X\n",
				__func__, reg_val);
		} else {
			cnt--;
			pr_debug("%s waitting for standby, glb_state=0x%02X\n",
				__func__, reg_val);
		}
		usleep_range(10000, 10500);
	}
	if (get_f0_flag) {
		aw869xx_haptic_read_lra_f0(aw869xx);
		aw869xx_haptic_read_cont_f0(aw869xx);
	} else {
		pr_err("%s enter standby mode failed, stop reading f0\n", __func__);
	}
	/* restore default config */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG1,
		AW869XX_BIT_CONTCFG1_EN_F0_DET_MASK,
		AW869XX_BIT_CONTCFG1_F0_DET_DISABLE);
	/* recover auto break config */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
		AW869XX_BIT_PLAYCFG3_BRK_EN_MASK, brk_en_temp << AW869XX_BREAK_EN);
	return ret;
}

static int aw869xx_haptic_f0_calibration(struct aw869xx *aw869xx)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int f0_limit;
	char f0_cali_lra;
	int f0_cali_step;
	unsigned int f0_cali_min = aw869xx->dts_info.f0_ref *
		(100 - aw869xx->dts_info.f0_cali_percent) / 100;
	unsigned int f0_cali_max = aw869xx->dts_info.f0_ref *
		(100 + aw869xx->dts_info.f0_cali_percent) / 100;

	pr_info("%s enter\n", __func__);

	if (aw869xx_haptic_cont_get_f0(aw869xx)) {
		pr_err( "%s get f0 error, user defafult f0\n", __func__);
		goto haptic_stop;
	}

	/* max and min limit */
	f0_limit = aw869xx->f0;
	pr_info("%s f0_ref = %d, f0_cali_min = %d, f0_cali_max = %d, f0 = %d\n",
		__func__, aw869xx->dts_info.f0_ref,
		f0_cali_min, f0_cali_max, aw869xx->f0);

	if ((aw869xx->f0 < f0_cali_min) || aw869xx->f0 > f0_cali_max) {
		pr_err("%s f0 calibration out of range = %d!\n", __func__, aw869xx->f0);
		f0_limit = aw869xx->dts_info.f0_ref;
		return -ERANGE;
	}
	pr_info("%s f0_limit = %d\n", __func__, (int)f0_limit);
	/* calculate cali step */
	f0_cali_step = 100000 * ((int)f0_limit - (int)aw869xx->dts_info.f0_ref) /
		((int)f0_limit * 24);
	pr_info("%s f0_cali_step = %d\n", __func__, f0_cali_step);
	if (f0_cali_step >= 0) { /* f0_cali_step >= 0 */
		if (f0_cali_step % 10 >= 5)
			f0_cali_step = 32 + (f0_cali_step / 10 + 1);
		else
			f0_cali_step = 32 + f0_cali_step / 10;
	} else { /* f0_cali_step < 0 */
		if (f0_cali_step % 10 <= -5)
			f0_cali_step = 32 + (f0_cali_step / 10 - 1);
		else
			f0_cali_step = 32 + f0_cali_step / 10;
	}
	if (f0_cali_step > 31)
		f0_cali_lra = (char)f0_cali_step - 32;
	else
		f0_cali_lra = (char)f0_cali_step + 32;
	/* update cali step */
	aw869xx_i2c_read(aw869xx, AW869XX_REG_TRIMCFG3, &reg_val);
	aw869xx->f0_cali_data = ((int)f0_cali_lra + (int)(reg_val & 0x3f)) & 0x3f;

	pr_info("%s origin trim_lra = 0x%02X, f0_cali_lra = 0x%02X,"
		" final f0_cali_data = 0x%02X\n",
		__func__, (reg_val & 0x3f), f0_cali_lra, aw869xx->f0_cali_data);
	aw869xx_haptic_upload_lra(aw869xx, F0_CALI);
	/* restore standby work mode */
haptic_stop:
	aw869xx_haptic_stop(aw869xx);
	return ret;
}

static int aw869xx_haptic_i2s_init(struct aw869xx *aw869xx)
{
	pr_info("%s: enter\n", __func__);

	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
		AW869XX_BIT_SYSCTRL2_I2S_PIN_MASK,
		AW869XX_BIT_SYSCTRL2_I2S_PIN_I2S);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_IOCFG1,
		AW869XX_BIT_IOCFG1_IO_FAST_MASK,
		AW869XX_BIT_IOCFG1_IIS_IO_FAST_ENABLE);
	return 0;
}

static void aw869xx_adapt_amp_again(struct aw869xx *aw869xx, int haptic_type)
{
	switch (haptic_type) {
	case LONG_VIB_RAM_MODE:
		if (aw869xx->amplitude >= AW869XX_LONG_MAX_AMP_CFG || aw869xx->amplitude < 0)
			return;
		aw869xx->gain = long_amp_again_val[aw869xx->amplitude];
		pr_info("%s long gain = %u\n", __func__, aw869xx->gain);
		break;
	case SHORT_VIB_RAM_MODE:
		if (aw869xx->amplitude >= AW869XX_SHORT_MAX_AMP_CFG || aw869xx->amplitude < 0)
			return;
		aw869xx->gain = short_amp_again_val[aw869xx->amplitude];
		pr_info("%s long gain = %u\n", __func__, aw869xx->gain);
		break;
	case RTP_VIB_MODE:
		aw869xx->gain = 0x80; // define max amp
		break;
	default:
		break;
	}
}

static int aw869xx_file_open(struct inode *inode, struct file *file)
{
	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	file->private_data = (void *)g_aw869xx;

	return 0;
}

static int aw869xx_file_release(struct inode *inode, struct file *file)
{
	if (!file)
		return 0;

	file->private_data = (void *)NULL;

	module_put(THIS_MODULE);

	return 0;
}

static long aw869xx_file_unlocked_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	return 0;
}

static ssize_t aw869xx_file_read(struct file *filp, char *buff, size_t len,
	loff_t *offset)
{
	return len;
}

static void aw869xx_special_type_process(struct aw869xx *aw869xx,
	uint64_t type)
{
	switch (type) {
	case HAPTIC_CHARGING_CALIB_ID:
		aw869xx->f0_is_cali = HAPTIC_CHARGING_IS_CALIB;
		schedule_work(&aw869xx->long_vibrate_work);
		break;
	case HAPTIC_NULL_WAVEFORM_ID:
		pr_info("null wave id\n");
		break;
	default:
		break;
	}
}

static ssize_t aw869xx_file_write(struct file *filp, const char *buff,
	size_t len, loff_t *off)
{
	struct aw869xx *aw869xx = NULL;
	char write_buf[MAX_WRITE_BUF_LEN] = {0};
	uint64_t type = 0;

	if (!buff || !filp || (len > (MAX_WRITE_BUF_LEN - 1)))
		return len;

	aw869xx = (struct aw869xx *)filp->private_data;
	if (!aw869xx)
		return len;

	if (copy_from_user(write_buf, buff, len)) {
		pr_err("[haptics_write] copy_from_user failed\n");
		return len;
	}
	if (strict_strtoull(write_buf, 10, &type)) {
		pr_err("[haptics_write] read value error\n");
		return len;
	}
	pr_info("%s: effect_id = %d\n",__func__, type);

	mutex_lock(&aw869xx->lock);
	aw869xx->cust_boost_on = AW869XX_SEL_BOOST_CFG_ON;
	if ((type < HAPTIC_MAX_SPEC_CMD_ID) && (type > HAPTIC_MIN_SPEC_CMD_ID)) {
		aw869xx_special_type_process(aw869xx, type);
		mutex_unlock(&aw869xx->lock);
		return len;
	}
	if (type > LONG_HAPTIC_RTP_MAX_ID) { // long time
		aw869xx->effect_mode = LONG_VIB_RAM_MODE;
		aw869xx->index = AW869XX_LONG_VIB_EFFECT_ID;
		aw869xx->duration = type / LONG_TIME_AMP_DIV_COFF;
		aw869xx->amplitude = type % LONG_TIME_AMP_DIV_COFF;
		aw869xx_adapt_amp_again(aw869xx, LONG_VIB_RAM_MODE);
		aw869xx->state = 1;
		__pm_wakeup_event(&lra_lock, aw869xx->duration + HAPTIC_WAKE_LOCK_GAP);
		pr_info("%s long index = %d, amp = %d\n", __func__, aw869xx->index, aw869xx->amplitude);
		schedule_work(&aw869xx->long_vibrate_work);
	} else if ((type > 0) && (type <= SHORT_HAPTIC_RAM_MAX_ID)) { // short ram haptic
		aw869xx->effect_mode = SHORT_VIB_RAM_MODE;
		aw869xx->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		aw869xx->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		pr_info("%s short index = %d, amp = %d\n", __func__, aw869xx->index, aw869xx->amplitude);
		aw869xx->state = 1;
		aw869xx_adapt_amp_again(aw869xx, SHORT_VIB_RAM_MODE);
		schedule_work(&aw869xx->long_vibrate_work);
	} else { // long and short rtp haptic
		aw869xx->effect_mode = RTP_VIB_MODE;
		aw869xx->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		aw869xx->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		aw869xx_haptic_stop(aw869xx);
		aw869xx_haptic_set_rtp_aei(aw869xx, false);
		aw869xx_interrupt_clear(aw869xx);
		aw869xx_adapt_amp_again(aw869xx, RTP_VIB_MODE);
		aw869xx->state = 1;
		aw869xx->rtp_idx = aw869xx->index - BASE_INDEX;
		len += snprintf(aw869xx_rtp_idx_name, AW869XX_RTP_NAME_MAX - 1,
			"aw869xx_%d.bin", aw869xx->rtp_idx);
		pr_info("%s get rtp name = %s, index = %d, len = %d\n", __func__,
			aw869xx_rtp_idx_name, aw869xx->rtp_idx, len);
		schedule_work(&aw869xx->rtp_work);
	}
	mutex_unlock(&aw869xx->lock);

	return len;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = aw869xx_file_read,
	.write = aw869xx_file_write,
	.unlocked_ioctl = aw869xx_file_unlocked_ioctl,
	.open = aw869xx_file_open,
	.release = aw869xx_file_release,
};

static struct miscdevice aw869xx_haptic_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = AW869XX_HAPTIC_NAME,
	.fops = &fops,
};

static int aw869xx_haptic_init(struct aw869xx *aw869xx)
{
	int ret;
	unsigned char i;
	unsigned char reg_val = 0;

	pr_info("%s enter\n", __func__);
	/* haptic init */
	ret = misc_register(&aw869xx_haptic_misc);
	if (ret) {
		pr_err("%s: misc fail: %d\n", __func__, ret);
		return ret;
	}

	mutex_lock(&aw869xx->lock);
	aw869xx->activate_mode = aw869xx->dts_info.mode;
	ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_WAVCFG1, &reg_val);
	aw869xx->index = reg_val & 0x7F;
	ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_PLAYCFG2, &reg_val);
	aw869xx->gain = reg_val & 0xFF;
	pr_info("%s aw869xx->gain =0x%02X\n", __func__, aw869xx->gain);
	ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_PLAYCFG1, &reg_val);
	aw869xx->vmax = reg_val & 0x1F;
	for (i = 0; i < AW869XX_SEQUENCER_SIZE; i++) {
		ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_WAVCFG1 + i, &reg_val);
		aw869xx->seq[i] = reg_val;
	}
	aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_STANDBY_MODE);
	aw869xx_haptic_set_pwm(aw869xx, AW869XX_PWM_12K);
	/* misc value init */
	aw869xx_haptic_misc_para_init(aw869xx);
	/* set BST_ADJ */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_BSTCFG5,
		AW869XX_BIT_BSTCFG5_BST_ADJ_MASK, AW869XX_BIT_BSTCFG5_BST_ADJ_LOW);
	aw869xx_haptic_set_bst_peak_cur(aw869xx);
	aw869xx_haptic_swicth_motor_protect_config(aw869xx, 0x00, 0x00);
	aw869xx_haptic_auto_bst_enable(aw869xx,
		aw869xx->dts_info.is_enabled_auto_bst);
	aw869xx_haptic_trig_param_init(aw869xx);
	aw869xx_haptic_trig_param_config(aw869xx);
	aw869xx_haptic_offset_calibration(aw869xx);
	/* vbat compensation */
	aw869xx_haptic_vbat_mode_config(aw869xx,
		AW869XX_HAPTIC_CONT_VBAT_HW_ADJUST_MODE);
	aw869xx->ram_vbat_compensate = AW869XX_HAPTIC_RAM_VBAT_COMP_ENABLE;
	/* i2s config */
	if (aw869xx->dts_info.is_enabled_i2s) {
		pr_info("%s i2s is enabled!\n", __func__);
		aw869xx_haptic_i2s_init(aw869xx);
	}
	mutex_unlock(&aw869xx->lock);
#ifdef AW_F0_COARSE_CALI /* Only Test for F0 calibration offset */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_TRIMCFG3,
		AW869XX_BIT_TRIMCFG3_OSC_TRIM_SRC_MASK,
		AW869XX_BIT_TRIMCFG3_OSC_TRIM_SRC_REG);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_TRIMCFG4, 0xEC);
#endif
	/* f0 calibration */
	if (aw869xx->dts_info.is_enabled_powerup_f0_cali) {
		mutex_lock(&aw869xx->lock);
		aw869xx_haptic_upload_lra(aw869xx, WRITE_ZERO);
		aw869xx_haptic_f0_calibration(aw869xx);
		mutex_unlock(&aw869xx->lock);
	} else {
		pr_info("%s powerup f0 calibration is disabled\n", __func__);
	}
	return ret;
}

#ifdef TIMED_OUTPUT
static int aw869xx_vibrator_get_time(struct timed_output_dev *dev)
{
	struct aw869xx *aw869xx = NULL;

	if (!dev)
		return 0;
	aw869xx = container_of(dev, struct aw869xx, vib_dev);
	if (!aw869xx)
		return 0;

	if (hrtimer_active(&aw869xx->timer)) {
		ktime_t r = hrtimer_get_remaining(&aw869xx->timer);
		return ktime_to_ms(r);
	}
	return 0;
}

static void aw869xx_vibrator_enable(struct timed_output_dev *dev, int value)
{
	struct aw869xx *aw869xx = NULL;

	if (!aw869xx)
		return;

	aw869xx = container_of(dev, struct aw869xx, vib_dev);
	if (!aw869xx)
		return;
	pr_info("%s enter\n", __func__);
	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_stop(aw869xx);
	if (value > 0) {
		aw869xx_haptic_ram_vbat_compensate(aw869xx, false);
		aw869xx_haptic_bst_mode_config(aw869xx,
			AW869XX_HAPTIC_BST_MODE_BOOST);
		aw869xx_haptic_play_wav_seq(aw869xx, value);
	}
	mutex_unlock(&aw869xx->lock);
	pr_info("%s exit\n", __func__);
}
#else
static enum led_brightness aw869xx_haptic_brightness_get(struct led_classdev *cdev)
{
	struct aw869xx *aw869xx = NULL;

	if (!cdev)
		return 0;

	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	if (!aw869xx)
		return 0;
	return aw869xx->amplitude;
}

static void aw869xx_haptic_brightness_set(struct led_classdev *cdev,
	enum led_brightness level)
{
	struct aw869xx *aw869xx = NULL;

	if (cdev)
		return;

	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	if (!aw869xx)
		return;

	pr_info("%s enter\n", __func__);
	if (!aw869xx->ram_init)
		return;
	aw869xx->amplitude = level;
	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_stop(aw869xx);
	if (aw869xx->amplitude > 0) {
		aw869xx_haptic_ram_vbat_compensate(aw869xx, false);
		aw869xx_haptic_bst_mode_config(aw869xx,
			AW869XX_HAPTIC_BST_MODE_BOOST);
		aw869xx_haptic_play_wav_seq(aw869xx, aw869xx->amplitude);
	}
	mutex_unlock(&aw869xx->lock);
}
#endif

static ssize_t aw869xx_vib_protocol_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int type = 0;
	int len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);;
	null_pointer_err_check(aw869xx);

	kstrtouint(buf, 0, &type);
	pr_info("%s: effect_id = %d\n",__func__, type);

	mutex_lock(&aw869xx->lock);

	if (type > LONG_HAPTIC_RTP_MAX_ID) { // long time
		aw869xx->effect_mode = LONG_VIB_RAM_MODE;
		aw869xx->index = AW869XX_LONG_VIB_EFFECT_ID;
		aw869xx->duration = type / LONG_TIME_AMP_DIV_COFF;
		aw869xx->amplitude = type % LONG_TIME_AMP_DIV_COFF;
		aw869xx_adapt_amp_again(aw869xx, LONG_VIB_RAM_MODE);
		aw869xx->state = 1;
		pr_info("%s long index = %d, amp = %d\n", __func__, aw869xx->index, aw869xx->amplitude);
		schedule_work(&aw869xx->long_vibrate_work);
	} else if ((type > 0) && (type <= SHORT_HAPTIC_RAM_MAX_ID)) { // short ram haptic
		aw869xx->effect_mode = SHORT_VIB_RAM_MODE;
		aw869xx->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		aw869xx->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		pr_info("%s short index = %d, amp = %d\n", __func__, aw869xx->index, aw869xx->amplitude);
		aw869xx->state = 1;
		aw869xx_adapt_amp_again(aw869xx, SHORT_VIB_RAM_MODE);
		schedule_work(&aw869xx->long_vibrate_work);
	} else { // long and short rtp haptic
		aw869xx->effect_mode = RTP_VIB_MODE;
		aw869xx->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		aw869xx->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		aw869xx_haptic_stop(aw869xx);
		aw869xx_haptic_set_rtp_aei(aw869xx, false);
		aw869xx_interrupt_clear(aw869xx);
		aw869xx_adapt_amp_again(aw869xx, RTP_VIB_MODE);
		aw869xx->state = 1;
		aw869xx->rtp_idx = aw869xx->index - BASE_INDEX;
		len += snprintf(aw869xx_rtp_idx_name, AW869XX_RTP_NAME_MAX - 1,
			"aw869xx_%d.bin", aw869xx->rtp_idx);
		pr_info("%s get rtp name = %s, index = %d, len = %d\n", __func__,
			aw869xx_rtp_idx_name, aw869xx->rtp_idx, len);
		schedule_work(&aw869xx->rtp_work);
	}
	mutex_unlock(&aw869xx->lock);
	return count;
}

static ssize_t aw869xx_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	return snprintf(buf, PAGE_SIZE, "state = %d\n", aw869xx->state);
}

static ssize_t aw869xx_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t aw869xx_duration_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ktime_t time_rem;
	s64 time_ms = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (hrtimer_active(&aw869xx->timer)) {
		time_rem = hrtimer_get_remaining(&aw869xx->timer);
		time_ms = ktime_to_ms(time_rem);
	}
	return snprintf(buf, PAGE_SIZE, "duration = %lldms\n", time_ms);
}

static ssize_t aw869xx_duration_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	/* setting 0 on duration is NOP for now */
	if (val <= 0)
		return count;
	mutex_lock(&aw869xx->lock);
	aw869xx->duration = val;
	aw869xx->index = AW869XX_LONG_VIB_BOOST_OFF_ID;
	aw869xx->effect_mode = LONG_VIB_RAM_MODE;
	aw869xx->cust_boost_on = 0;
	__pm_wakeup_event(&lra_lock, aw869xx->duration + HAPTIC_WAKE_LOCK_GAP);
	mutex_unlock(&aw869xx->lock);

	return count;
}

static ssize_t aw869xx_activate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	/* For now nothing to show */
	return snprintf(buf, PAGE_SIZE, "activate = %d\n", aw869xx->state);
}

static ssize_t aw869xx_activate_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	pr_info("%s: value=%d\n", __func__, val);
	mutex_lock(&aw869xx->lock);
	hrtimer_cancel(&aw869xx->timer);
	aw869xx->amplitude = MAX_LONG_HAPTIC_AMP;
	aw869xx->gain = long_amp_again_val[MAX_LONG_HAPTIC_AMP];
	aw869xx->state = val;
	mutex_unlock(&aw869xx->lock);
	schedule_work(&aw869xx->long_vibrate_work);
	return count;
}

static ssize_t aw869xx_activate_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	return snprintf(buf, PAGE_SIZE, "activate_mode = %d\n",
		aw869xx->activate_mode);
}

static ssize_t aw869xx_activate_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	mutex_lock(&aw869xx->lock);
	aw869xx->activate_mode = val;
	mutex_unlock(&aw869xx->lock);
	return count;
}

static ssize_t aw869xx_index_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned char reg_val = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	aw869xx_i2c_read(aw869xx, AW869XX_REG_WAVCFG1, &reg_val);
	aw869xx->index = reg_val;
	return snprintf(buf, PAGE_SIZE, "index = %d\n", aw869xx->index);
}

static ssize_t aw869xx_index_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val;
	int rc = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	pr_info("%s: value=%d\n", __func__, val);
	mutex_lock(&aw869xx->lock);
	aw869xx->index = val;
	aw869xx_haptic_set_repeat_wav_seq(aw869xx, aw869xx->index);
	mutex_unlock(&aw869xx->lock);
	return count;
}

static ssize_t aw869xx_vmax_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	return snprintf(buf, PAGE_SIZE, "vmax = 0x%02X\n", aw869xx->vmax);
}

static ssize_t aw869xx_vmax_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	pr_info("%s: value=%d\n", __func__, val);

	mutex_lock(&aw869xx->lock);
	aw869xx->vmax = val;
	aw869xx_haptic_set_bst_vol(aw869xx, aw869xx->vmax);
	mutex_unlock(&aw869xx->lock);
	return count;
}

static ssize_t aw869xx_gain_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	return snprintf(buf, PAGE_SIZE, "gain = 0x%02X\n", aw869xx->gain);
}

static ssize_t aw869xx_gain_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	pr_info("%s: value=%d\n", __func__, val);

	mutex_lock(&aw869xx->lock);
	aw869xx->gain = val;
	aw869xx_haptic_set_gain(aw869xx, aw869xx->gain);
	mutex_unlock(&aw869xx->lock);
	return count;
}

static ssize_t aw869xx_seq_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	size_t count = 0;
	unsigned char i;
	unsigned char reg_val = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	for (i = 0; i < AW869XX_SEQUENCER_SIZE; i++) {
		aw869xx_i2c_read(aw869xx, AW869XX_REG_WAVCFG1 + i, &reg_val);
		count += snprintf(buf + count, PAGE_SIZE - count,
			"seq%d = %d\n", i + 1, reg_val);
		aw869xx->seq[i] |= reg_val;
	}
	return count;
}

static ssize_t aw869xx_seq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[2] = { 0, 0 };

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		pr_info("%s: seq%d=0x%02X\n", __func__, databuf[0], databuf[1]);
		mutex_lock(&aw869xx->lock);
		aw869xx->seq[databuf[0]] = (unsigned char)databuf[1];
		aw869xx_haptic_set_wav_seq(aw869xx, (unsigned char)databuf[0],
					   aw869xx->seq[databuf[0]]);
		mutex_unlock(&aw869xx->lock);
	}
	return count;
}

static ssize_t aw869xx_loop_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	size_t count = 0;
	unsigned char i;
	unsigned char reg_val = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	for (i = 0; i < AW869XX_SEQUENCER_LOOP_SIZE; i++) {
		aw869xx_i2c_read(aw869xx, AW869XX_REG_WAVCFG9 + i, &reg_val);
		aw869xx->loop[i * 2 + 0] = (reg_val >> 4) & 0x0F;
		aw869xx->loop[i * 2 + 1] = (reg_val >> 0) & 0x0F;

		count += snprintf(buf + count, PAGE_SIZE - count,
			"seq%d_loop = %d\n", i * 2 + 1,
			aw869xx->loop[i * 2 + 0]);
		count += snprintf(buf + count, PAGE_SIZE - count,
			"seq%d_loop = %d\n", i * 2 + 2,
			aw869xx->loop[i * 2 + 1]);
	}
	return count;
}

static ssize_t aw869xx_loop_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[2] = { 0, 0 };

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		pr_info("%s: seq%d loop=0x%02X\n", __func__,
			 databuf[0], databuf[1]);
		mutex_lock(&aw869xx->lock);
		aw869xx->loop[databuf[0]] = (unsigned char)databuf[1];
		aw869xx_haptic_set_wav_loop(aw869xx, (unsigned char)databuf[0],
			aw869xx->loop[databuf[0]]);
		mutex_unlock(&aw869xx->lock);
	}

	return count;
}

static ssize_t aw869xx_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;
	unsigned char i;
	unsigned char reg_val = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	for (i = 0; i < AW869XX_REG_MAX; i++) {
		if (!(aw869xx_reg_access[i] & REG_RD_ACCESS))
			continue;
		aw869xx_i2c_read(aw869xx, i, &reg_val);
		len += snprintf(buf + len, PAGE_SIZE - len,
			"reg:0x%02X=0x%02X\n", i, reg_val);
	}
	return len;
}

static ssize_t aw869xx_reg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[2] = { 0, 0 };

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2)
		aw869xx_i2c_write(aw869xx, (unsigned char)databuf[0],
			(unsigned char)databuf[1]);

	return count;
}

static ssize_t aw869xx_rtp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len, "rtp_cnt = %d\n",
		aw869xx->rtp_cnt);
	return len;
}

static ssize_t aw869xx_rtp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;
	int len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0) {
		pr_info("%s: kstrtouint fail\n", __func__);
		return rc;
	}
	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_stop(aw869xx);
	aw869xx_haptic_set_rtp_aei(aw869xx, false);
	aw869xx_interrupt_clear(aw869xx);

	len += snprintf(aw869xx_rtp_idx_name, AW869XX_RTP_NAME_MAX - 1, "aw869xx_%u.bin", val);
	pr_info("%s get rtp name = %s, len = %d\n", __func__, aw869xx_rtp_idx_name, len);
	schedule_work(&aw869xx->rtp_work);
	mutex_unlock(&aw869xx->lock);
	return count;
}

static ssize_t aw869xx_ram_update_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;
	unsigned int i;
	unsigned char reg_val = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	/* RAMINIT Enable */
	aw869xx_haptic_raminit(aw869xx, true);
	aw869xx_haptic_stop(aw869xx);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_RAMADDRH,
		(unsigned char)(aw869xx->ram.base_addr >> 8));
	aw869xx_i2c_write(aw869xx, AW869XX_REG_RAMADDRL,
		(unsigned char)(aw869xx->ram.base_addr & 0x00ff));
	len += snprintf(buf + len, PAGE_SIZE - len, "aw869xx_haptic_ram:\n");
	for (i = 0; i < aw869xx->ram.len; i++) {
		aw869xx_i2c_read(aw869xx, AW869XX_REG_RAMDATA, &reg_val);
		len += snprintf(buf + len, PAGE_SIZE - len, "0x%02X,", reg_val);
	}
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	/* RAMINIT Disable */
	aw869xx_haptic_raminit(aw869xx, false);
	return len;
}

static ssize_t aw869xx_ram_update_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	if (val)
		aw869xx_ram_update(aw869xx);
	return count;
}

static ssize_t aw869xx_f0_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_upload_lra(aw869xx, WRITE_ZERO);
	aw869xx_haptic_cont_get_f0(aw869xx);
	mutex_unlock(&aw869xx->lock);
	len += snprintf(buf + len, PAGE_SIZE - len,
		"lra_f0 = %d cont_f0 = %d\n", aw869xx->f0, aw869xx->cont_f0);
	return len;
}

static ssize_t aw869xx_f0_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int val = 0;
	int rc;

	if (!buf)
		return 0;
	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	return count;
}

static ssize_t aw869xx_rtp_src_sel_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len,
		"rtp_src_sel = %d\n", aw869xx->rtp_src_sel);
	return len;
}

static ssize_t aw869xx_rtp_src_sel_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	pr_info("%s: rtp_src_sel value=%d\n", __func__, val);
	mutex_lock(&aw869xx->lock);
	aw869xx->rtp_src_sel = val;
	mutex_unlock(&aw869xx->lock);

	return count;
}


static ssize_t aw869xx_ram_f0_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_upload_lra(aw869xx, F0_CALI);
	aw869xx_haptic_ram_get_f0(aw869xx);
	mutex_unlock(&aw869xx->lock);
	len += snprintf(buf + len, PAGE_SIZE - len, "ram_lra_f0 = %d\n",
			aw869xx->f0);
	return len;
}

static ssize_t aw869xx_ram_f0_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int val = 0;
	int rc;

	if (!buf)
		return 0;
	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	return count;
}

static ssize_t aw869xx_osc_save_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;

	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len, "osc_cali_data = 0x%02X\n",
		aw869xx->osc_cali_data);

	return len;
}

static ssize_t aw869xx_osc_save_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;

	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	aw869xx->osc_cali_data = val;
	return count;
}

static ssize_t aw869xx_f0_save_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;

	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len, "f0_cali_data = 0x%02X\n",
		aw869xx->f0_cali_data);

	return len;
}

static ssize_t aw869xx_f0_save_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	aw869xx->f0_cali_data = val;
	return count;
}

static ssize_t aw869xx_cali_val_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (aw869xx->f0_val_err < 0)
		aw869xx->f0 = 0;
	pr_info("%s cal f0 val = %u\n", __func__, aw869xx->f0);

#ifdef F0_CONT_F0
	len += snprintf(buf + len, PAGE_SIZE - len,
		"lra_f0 = %d cont_f0 = %d\n", aw869xx->f0,
		aw869xx->cont_f0);
#endif
	len += snprintf(buf + len, PAGE_SIZE - len, "%d", aw869xx->f0);

	return len;
}

static ssize_t aw869xx_cali_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;
	int cal_rst;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	pr_info("%s cal val = %u\n", __func__, aw869xx->f0);
#ifdef F0_CONT_F0
	len += snprintf(buf + len, PAGE_SIZE - len,
		"lra_f0 = %d cont_f0 = %d\n", aw869xx->f0,
		aw869xx->cont_f0);
#endif
	if (aw869xx->f0 > AW869XX_F0_CALI_THER_H ||
		aw869xx->f0 < AW869XX_F0_CALI_THER_L)
		cal_rst = 0;
	else
		cal_rst = 1;

	if (aw869xx->f0_val_err < 0) {
		cal_rst = 0;
		pr_info("%s cal err\n", __func__);
	}
	len += snprintf(buf + len, PAGE_SIZE - len, "%d", cal_rst);

	return len;
}

static ssize_t aw869xx_cali_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int i;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	if (val) {
		aw869xx->f0_val_err = 0;
		mutex_lock(&aw869xx->lock);
		aw869xx_haptic_upload_lra(aw869xx, WRITE_ZERO);
		for (i = 0; i < 2; i++) {
			aw869xx_haptic_f0_calibration(aw869xx);
			mdelay(20);
		}
		mutex_unlock(&aw869xx->lock);
	}
	return count;
}

static ssize_t aw869xx_cont_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	aw869xx_haptic_read_cont_f0(aw869xx);
	len += snprintf(buf + len, PAGE_SIZE - len,
		"cont_f0 = %d\n", aw869xx->cont_f0);
	return len;
}

static ssize_t aw869xx_cont_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	aw869xx_haptic_stop(aw869xx);
	if (val)
		aw869xx_haptic_cont_config(aw869xx);
	return count;
}

static ssize_t aw869xx_cont_wait_num_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len,
		"cont_wait_num = 0x%02X\n", aw869xx->cont_wait_num);
	return len;
}

static ssize_t aw869xx_cont_wait_num_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[1] = { 0 };

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%x", &databuf[0]) == 1) {
		aw869xx->cont_wait_num = databuf[0];
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG4, databuf[0]);
	}
	return count;
}

static ssize_t aw869xx_cont_drv_lvl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len,
			"cont_drv1_lvl = 0x%02X, cont_drv2_lvl = 0x%02X\n",
			aw869xx->cont_drv1_lvl, aw869xx->cont_drv2_lvl);
	return len;
}

static ssize_t aw869xx_cont_drv_lvl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[2] = { 0, 0 };

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		aw869xx->cont_drv1_lvl = databuf[0];
		aw869xx->cont_drv2_lvl = databuf[1];
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_CONTCFG6,
		AW869XX_BIT_CONTCFG6_DRV1_LVL_MASK, aw869xx->cont_drv1_lvl);
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG7,
			aw869xx->cont_drv2_lvl);
	}
	return count;
}

static ssize_t aw869xx_cont_drv_time_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len,
			"cont_drv1_time = 0x%02X, cont_drv2_time = 0x%02X\n",
			aw869xx->cont_drv1_time, aw869xx->cont_drv2_time);
	return len;
}

static ssize_t aw869xx_cont_drv_time_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[2] = { 0, 0 };

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		aw869xx->cont_drv1_time = databuf[0];
		aw869xx->cont_drv2_time = databuf[1];
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG8,
			aw869xx->cont_drv1_time);
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG9,
			aw869xx->cont_drv2_time);
	}
	return count;
}

static ssize_t aw869xx_cont_brk_time_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len, "cont_brk_time = 0x%02X\n",
			aw869xx->cont_brk_time);
	return len;
}

static ssize_t aw869xx_cont_brk_time_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[1] = { 0 };

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%x", &databuf[0]) == 1) {
		aw869xx->cont_brk_time = databuf[0];
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG10,
			aw869xx->cont_brk_time);
	}
	return count;
}

static ssize_t aw869xx_vbat_monitor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_get_vbat(aw869xx);
	len += snprintf(buf + len, PAGE_SIZE - len, "vbat_monitor = %d\n",
		aw869xx->vbat);
	mutex_unlock(&aw869xx->lock);

	return len;
}

static ssize_t aw869xx_vbat_monitor_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t aw869xx_lra_resistance_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	aw869xx_haptic_get_lra_resistance(aw869xx);
	len += snprintf(buf + len, PAGE_SIZE - len, "lra_resistance = %d\n",
		aw869xx->lra);
	return len;
}

static ssize_t aw869xx_lra_resistance_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	return count;
}

static ssize_t aw869xx_auto_boost_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len, "auto_boost = %d\n",
			aw869xx->auto_boost);

	return len;
}

static ssize_t aw869xx_auto_boost_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_stop(aw869xx);
	aw869xx_haptic_auto_bst_enable(aw869xx, val);
	mutex_unlock(&aw869xx->lock);

	return count;
}

static ssize_t aw869xx_prctmode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;
	unsigned char reg_val = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	aw869xx_i2c_read(aw869xx, AW869XX_REG_DETCFG1, &reg_val);

	len += snprintf(buf + len, PAGE_SIZE - len, "prctmode = %d\n",
		reg_val & 0x08);
	return len;
}

static ssize_t aw869xx_prctmode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[2] = { 0, 0 };
	unsigned int addr;
	unsigned int val;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		addr = databuf[0];
		val = databuf[1];
		mutex_lock(&aw869xx->lock);
		aw869xx_haptic_swicth_motor_protect_config(aw869xx, addr, val);
		mutex_unlock(&aw869xx->lock);
	}
	return count;
}

static ssize_t aw869xx_trig_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;
	unsigned char i = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	for (i = 0; i < AW869XX_TRIG_NUM; i++) {
		len += snprintf(buf + len, PAGE_SIZE - len,
			"trig%d: trig_level=%d, trig_polar=%d, pos_enable=%d, pos_sequence=%d, neg_enable=%d, neg_sequence=%d trig_brk=%d, trig_bst=%d\n",
			i + 1,
			aw869xx->trig[i].trig_level,
			aw869xx->trig[i].trig_polar,
			aw869xx->trig[i].pos_enable,
			aw869xx->trig[i].pos_sequence,
			aw869xx->trig[i].neg_enable,
			aw869xx->trig[i].neg_sequence,
			aw869xx->trig[i].trig_brk,
			aw869xx->trig[i].trig_bst);
	}

	return len;
}

static ssize_t aw869xx_trig_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int databuf[9] = { 0 };

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	if (sscanf(buf, "%d %d %d %d %d %d %d %d %d", &databuf[0], &databuf[1],
		&databuf[2], &databuf[3], &databuf[4], &databuf[5],
		&databuf[6], &databuf[7], &databuf[8])) {
		pr_info("%s: %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			__func__, databuf[0], databuf[1], databuf[2],
			databuf[3], databuf[4], databuf[5], databuf[6],
			databuf[7], databuf[8]);
		if (databuf[0] > 3)
			databuf[0] = 3;
		aw869xx->trig[databuf[0]].trig_level = databuf[1];
		aw869xx->trig[databuf[0]].trig_polar = databuf[2];
		aw869xx->trig[databuf[0]].pos_enable = databuf[3];
		aw869xx->trig[databuf[0]].pos_sequence = databuf[4];
		aw869xx->trig[databuf[0]].neg_enable = databuf[5];
		aw869xx->trig[databuf[0]].neg_sequence = databuf[6];
		aw869xx->trig[databuf[0]].trig_brk = databuf[7];
		aw869xx->trig[databuf[0]].trig_bst = databuf[8];
		mutex_lock(&aw869xx->lock);
		aw869xx_haptic_trig_param_config(aw869xx);
		mutex_unlock(&aw869xx->lock);
	}
	return count;
}

static ssize_t aw869xx_ram_vbat_compensate_show(struct device *dev,
						struct device_attribute *attr,
						char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len, "ram_vbat_compensate = %d\n",
			aw869xx->ram_vbat_compensate);

	return len;
}

static ssize_t aw869xx_ram_vbat_compensate_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	mutex_lock(&aw869xx->lock);
	if (val)
		aw869xx->ram_vbat_compensate =
		    AW869XX_HAPTIC_RAM_VBAT_COMP_ENABLE;
	else
		aw869xx->ram_vbat_compensate =
		    AW869XX_HAPTIC_RAM_VBAT_COMP_DISABLE;
	mutex_unlock(&aw869xx->lock);

	return count;
}

static ssize_t aw869xx_osc_cali_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	ssize_t len = 0;

	if (!dev || !buf)
		return 0;
	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	len += snprintf(buf + len, PAGE_SIZE - len, "osc_cali_data = 0x%02X\n",
		aw869xx->osc_cali_data);

	return len;
}

static ssize_t aw869xx_osc_cali_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct aw869xx *aw869xx = NULL;
	unsigned int val = 0;
	int rc;

	if (!dev || !buf)
		return 0;

	cdev = dev_get_drvdata(dev);
	null_pointer_err_check(cdev);
	aw869xx = container_of(cdev, struct aw869xx, vib_dev);
	null_pointer_err_check(aw869xx);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	mutex_lock(&aw869xx->lock);
	if (val == 1) {
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_D2SCFG1,
			AW869XX_BIT_D2SCFG1_CLK_TRIM_MODE_MASK,
			AW869XX_BIT_D2SCFG1_CLK_TRIM_MODE_24K);
		aw869xx_haptic_upload_lra(aw869xx, WRITE_ZERO);
		aw869xx_rtp_osc_calibration(aw869xx);
		aw869xx_rtp_trim_lra_calibration(aw869xx);
	}
	/* osc calibration flag end,Other behaviors are permitted */
	mutex_unlock(&aw869xx->lock);

	return count;
}

static DEVICE_ATTR(state, S_IWUSR | S_IRUGO, aw869xx_state_show,
	aw869xx_state_store);
static DEVICE_ATTR(duration, S_IWUSR | S_IRUGO, aw869xx_duration_show,
	aw869xx_duration_store);
static DEVICE_ATTR(vib_protocol, S_IWUSR | S_IRUGO, NULL,
	aw869xx_vib_protocol_store);
static DEVICE_ATTR(activate, S_IWUSR | S_IRUGO, aw869xx_activate_show,
	aw869xx_activate_store);
static DEVICE_ATTR(activate_mode, S_IWUSR | S_IRUGO, aw869xx_activate_mode_show,
	aw869xx_activate_mode_store);
static DEVICE_ATTR(index, S_IWUSR | S_IRUGO, aw869xx_index_show,
	aw869xx_index_store);
static DEVICE_ATTR(vmax, S_IWUSR | S_IRUGO, aw869xx_vmax_show,
	aw869xx_vmax_store);
static DEVICE_ATTR(gain, S_IWUSR | S_IRUGO, aw869xx_gain_show,
	aw869xx_gain_store);
static DEVICE_ATTR(seq, S_IWUSR | S_IRUGO, aw869xx_seq_show, aw869xx_seq_store);
static DEVICE_ATTR(loop, S_IWUSR | S_IRUGO, aw869xx_loop_show,
	aw869xx_loop_store);
static DEVICE_ATTR(reg, S_IWUSR | S_IRUGO, aw869xx_reg_show, aw869xx_reg_store);
static DEVICE_ATTR(rtp, S_IWUSR | S_IRUGO, aw869xx_rtp_show, aw869xx_rtp_store);
static DEVICE_ATTR(ram_update, S_IWUSR | S_IRUGO, aw869xx_ram_update_show,
	aw869xx_ram_update_store);
static DEVICE_ATTR(f0, S_IWUSR | S_IRUGO, aw869xx_f0_show, aw869xx_f0_store);
static DEVICE_ATTR(f0_save, S_IWUSR | S_IRUGO, aw869xx_f0_save_show,
	aw869xx_f0_save_store);
static DEVICE_ATTR(rtp_src_sel, S_IWUSR | S_IRUGO, aw869xx_rtp_src_sel_show, aw869xx_rtp_src_sel_store);
static DEVICE_ATTR(osc_save, S_IWUSR | S_IRUGO, aw869xx_osc_save_show,
	aw869xx_osc_save_store);
static DEVICE_ATTR(ram_f0, S_IWUSR | S_IRUGO, aw869xx_ram_f0_show,
	aw869xx_ram_f0_store);
static DEVICE_ATTR(vibrator_calib, S_IWUSR | S_IRUGO, aw869xx_cali_show,
	aw869xx_cali_store);
static DEVICE_ATTR(cali_val, S_IWUSR | S_IRUGO, aw869xx_cali_val_show,
	NULL);
static DEVICE_ATTR(cont, S_IWUSR | S_IRUGO, aw869xx_cont_show,
	aw869xx_cont_store);
static DEVICE_ATTR(cont_wait_num, S_IWUSR | S_IRUGO, aw869xx_cont_wait_num_show,
	aw869xx_cont_wait_num_store);
static DEVICE_ATTR(cont_drv_lvl, S_IWUSR | S_IRUGO, aw869xx_cont_drv_lvl_show,
	aw869xx_cont_drv_lvl_store);
static DEVICE_ATTR(cont_drv_time, S_IWUSR | S_IRUGO, aw869xx_cont_drv_time_show,
	aw869xx_cont_drv_time_store);
static DEVICE_ATTR(cont_brk_time, S_IWUSR | S_IRUGO, aw869xx_cont_brk_time_show,
	aw869xx_cont_brk_time_store);
static DEVICE_ATTR(vbat_monitor, S_IWUSR | S_IRUGO, aw869xx_vbat_monitor_show,
	aw869xx_vbat_monitor_store);
static DEVICE_ATTR(lra_resistance, S_IWUSR | S_IRUGO,
	aw869xx_lra_resistance_show, aw869xx_lra_resistance_store);
static DEVICE_ATTR(auto_boost, S_IWUSR | S_IRUGO, aw869xx_auto_boost_show,
	aw869xx_auto_boost_store);
static DEVICE_ATTR(prctmode, S_IWUSR | S_IRUGO, aw869xx_prctmode_show,
	aw869xx_prctmode_store);
static DEVICE_ATTR(trig, S_IWUSR | S_IRUGO, aw869xx_trig_show,
	aw869xx_trig_store);
static DEVICE_ATTR(ram_vbat_comp, S_IWUSR | S_IRUGO,
	aw869xx_ram_vbat_compensate_show, aw869xx_ram_vbat_compensate_store);
static DEVICE_ATTR(osc_cali, S_IWUSR | S_IRUGO, aw869xx_osc_cali_show,
	aw869xx_osc_cali_store);
static struct attribute *aw869xx_vibrator_attributes[] = {
	&dev_attr_state.attr,
	&dev_attr_duration.attr,
	&dev_attr_vib_protocol.attr,
	&dev_attr_activate.attr,
	&dev_attr_activate_mode.attr,
	&dev_attr_index.attr,
	&dev_attr_vmax.attr,
	&dev_attr_gain.attr,
	&dev_attr_seq.attr,
	&dev_attr_loop.attr,
	&dev_attr_reg.attr,
	&dev_attr_rtp.attr,
	&dev_attr_ram_update.attr,
	&dev_attr_f0.attr,
	&dev_attr_f0_save.attr,
	&dev_attr_rtp_src_sel.attr,
	&dev_attr_osc_save.attr,
	&dev_attr_ram_f0.attr,
	&dev_attr_vibrator_calib.attr,
	&dev_attr_cali_val.attr,
	&dev_attr_cont.attr,
	&dev_attr_cont_wait_num.attr,
	&dev_attr_cont_drv_lvl.attr,
	&dev_attr_cont_drv_time.attr,
	&dev_attr_cont_brk_time.attr,
	&dev_attr_vbat_monitor.attr,
	&dev_attr_lra_resistance.attr,
	&dev_attr_auto_boost.attr,
	&dev_attr_prctmode.attr,
	&dev_attr_trig.attr,
	&dev_attr_ram_vbat_comp.attr,
	&dev_attr_osc_cali.attr,
	NULL
};

static struct attribute_group aw869xx_vibrator_attribute_group = {
	.attrs = aw869xx_vibrator_attributes
};

static enum hrtimer_restart aw869xx_vibrator_timer_func(struct hrtimer *timer)
{
	struct aw869xx *aw869xx = container_of(timer, struct aw869xx, timer);

	if (!aw869xx)
		return HRTIMER_NORESTART;
	pr_info("%s enter\n", __func__);
	aw869xx->state = 0;
	schedule_work(&aw869xx->long_vibrate_work);

	return HRTIMER_NORESTART;
}

static void aw869xx_long_ram_mode_boost_config(struct aw869xx *aw869xx,
	unsigned char wavseq)
{
	if (aw869xx->cust_boost_on == AW869XX_SEL_BOOST_CFG_ON) {
		aw869xx_haptic_set_wav_seq(aw869xx, 0, LONG_HAPTIC_DRIVER_WAV_CNT);
		aw869xx_haptic_set_wav_loop(aw869xx, 0, 0); // loop mode
		aw869xx_haptic_set_wav_seq(aw869xx, 1, wavseq);
		aw869xx_haptic_set_wav_loop(aw869xx, 1, LONG_HAPTIC_LOOP_MODE_CNT);
		aw869xx_haptic_set_wav_seq(aw869xx, LONG_HAPTIC_WAV_SEQ_CNT, 0);
		aw869xx_haptic_set_wav_loop(aw869xx, LONG_HAPTIC_WAV_SEQ_CNT, 0);
	} else {
		aw869xx_haptic_set_wav_seq(aw869xx, 0, wavseq);
		aw869xx_haptic_set_wav_loop(aw869xx, 0, LONG_HAPTIC_LOOP_MODE_CNT);
		aw869xx_haptic_set_wav_seq(aw869xx, 1, 0);
		aw869xx_haptic_set_wav_loop(aw869xx, 1, 0);
	}
	aw869xx_i2c_write(aw869xx, AW869XX_REG_SYSCTRL7, aw869xx->ram_long_d2s_gain);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG5, aw869xx->ram_long_brk_gain);
	aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG10, aw869xx->ram_long_brk_time);
}

static void aw869xx_haptic_play_repeat_seq(struct aw869xx *aw869xx,
	unsigned char flag)
{
	if (flag) {
		aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_RAM_LOOP_MODE);
		aw869xx_haptic_play_go(aw869xx);
	}
}

static int aw869xx_haptic_ram_config(struct aw869xx *aw869xx)
{
	unsigned char wavseq = 0;
	unsigned char wavloop = 0;
	unsigned char seq_idx = 0;

	for (seq_idx = 0; seq_idx < 8; seq_idx++) { // 8 define max loop cnt
		aw869xx_haptic_set_wav_seq(aw869xx, seq_idx, 0);
		aw869xx_haptic_set_wav_loop(aw869xx, seq_idx, 0);
	}
	aw869xx_haptic_set_pwm(aw869xx, AW869XX_PWM_12K);
	if (aw869xx->effect_mode == SHORT_VIB_RAM_MODE) {
		wavseq = aw869xx->index;
		wavloop = 0;
		aw869xx_haptic_set_wav_seq(aw869xx, 0, wavseq);
		aw869xx_haptic_set_wav_loop(aw869xx, 0, wavloop);
		aw869xx_haptic_set_wav_seq(aw869xx, 1, 0);
		aw869xx_haptic_set_wav_loop(aw869xx, 1, 0);
		aw869xx_i2c_write(aw869xx, AW869XX_REG_SYSCTRL7,
			aw869xx->ram_short_d2s_gain);
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG5,
			aw869xx->ram_short_brk_gain);
		aw869xx_i2c_write(aw869xx, AW869XX_REG_CONTCFG10,
			aw869xx->ram_short_brk_time);
	} else {
		wavseq = aw869xx->index;
		aw869xx_long_ram_mode_boost_config(aw869xx, wavseq);
	}
	pr_info("%s wavseq:%d,\n", __func__, wavseq);

	return 0;
}

static void aw869xx_ram_mode_sel_boost_play_cfg(struct aw869xx *aw869xx)
{
	if (aw869xx->cust_boost_on == AW869XX_SEL_BOOST_CFG_ON) {
		aw869xx_haptic_ram_vbat_compensate(aw869xx, false);
		aw869xx_haptic_bst_mode_config(aw869xx, AW869XX_HAPTIC_BST_MODE_BOOST);
		aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_RAM_MODE);
		aw869xx_haptic_play_go(aw869xx);
		return;
	}
	if (aw869xx->effect_mode == LONG_VIB_RAM_MODE) {
		aw869xx_haptic_ram_vbat_compensate(aw869xx, true);
		aw869xx_haptic_bst_mode_config(aw869xx,
			AW869XX_HAPTIC_BST_MODE_BYPASS);
		aw869xx_haptic_play_repeat_seq(aw869xx, true);
	} else {
		aw869xx_haptic_ram_vbat_compensate(aw869xx, false);
		aw869xx_haptic_bst_mode_config(aw869xx,
			AW869XX_HAPTIC_BST_MODE_BOOST);
		aw869xx_haptic_play_mode(aw869xx, AW869XX_HAPTIC_RAM_MODE);
		aw869xx_haptic_play_go(aw869xx);
	}
}

static void aw869xx_long_vibrate_work_routine(struct work_struct *work)
{
	struct aw869xx *aw869xx =
		container_of(work, struct aw869xx, long_vibrate_work);

	pr_info("%s enter\n", __func__);
	if (aw869xx->ram_init == 0) {
		pr_info("%s ram init not ready\n", __func__);
		return;
	}
	mutex_lock(&aw869xx->lock);
	if (aw869xx->f0_is_cali == HAPTIC_CHARGING_IS_CALIB) {
		aw869xx_haptic_upload_lra(aw869xx, F0_CALI);
		aw869xx_haptic_f0_calibration(aw869xx);
		aw869xx->f0_is_cali = 0;
		mutex_unlock(&aw869xx->lock);
		return;
	}
	/* Enter standby mode */
	aw869xx_haptic_stop(aw869xx);
	aw869xx_haptic_upload_lra(aw869xx, F0_CALI);
	if (aw869xx->state) {
		if (aw869xx->activate_mode == AW869XX_HAPTIC_ACTIVATE_RAM_MODE) {
			aw869xx_haptic_ram_config(aw869xx);
			aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_PLAYCFG3,
				AW869XX_BIT_PLAYCFG3_BRK_EN_MASK,
				AW869XX_BIT_PLAYCFG3_BRK_ENABLE);
			aw869xx_ram_mode_sel_boost_play_cfg(aw869xx);
		} else if (aw869xx->activate_mode == AW869XX_HAPTIC_ACTIVATE_CONT_MODE) {
			aw869xx_haptic_cont_config(aw869xx);
		} else {
			pr_err("%s: activate_mode error\n", __func__);
		}
		/* run ms timer */
		if ((aw869xx->effect_mode == LONG_VIB_RAM_MODE) ||
		(aw869xx->activate_mode == AW869XX_HAPTIC_ACTIVATE_CONT_MODE))
			hrtimer_start(&aw869xx->timer,
				ktime_set(aw869xx->duration / 1000, (aw869xx->duration % 1000) * 1000000),
				HRTIMER_MODE_REL);
	}
	mutex_unlock(&aw869xx->lock);
}

static int aw869xx_vibrator_init(struct aw869xx *aw869xx)
{
	int ret;

	pr_info("%s enter\n", __func__);
#ifdef TIMED_OUTPUT
	pr_info("%s: TIMED_OUT FRAMEWORK!\n", __func__);
	aw869xx->vib_dev.name = "vibrator";
	aw869xx->vib_dev.get_time = aw869xx_vibrator_get_time;
	aw869xx->vib_dev.enable = aw869xx_vibrator_enable;

	ret = timed_output_dev_register(&(aw869xx->vib_dev));
	if (ret < 0) {
		pr_err("%s: fail to create timed output dev\n", __func__);
		return ret;
	}
	ret = sysfs_create_group(&aw869xx->vib_dev.dev->kobj,
		&aw869xx_vibrator_attribute_group);
	if (ret < 0) {
		pr_err( "%s error creating sysfs attr files\n", __func__);
		return ret;
	}
#else
	pr_info("%s: loaded in leds_cdev framework!\n", __func__);
	aw869xx->vib_dev.name = "vibrator";
	aw869xx->vib_dev.brightness_get = aw869xx_haptic_brightness_get;
	aw869xx->vib_dev.brightness_set = aw869xx_haptic_brightness_set;

	ret = devm_led_classdev_register(&aw869xx->i2c->dev, &aw869xx->vib_dev);
	if (ret < 0) {
		pr_err("%s: fail to create led dev\n", __func__);
		return ret;
	}
	ret = sysfs_create_group(&aw869xx->vib_dev.dev->kobj,
		&aw869xx_vibrator_attribute_group);
	if (ret < 0) {
		pr_err("%s error creating sysfs attr files\n", __func__);
		return ret;
	}
#endif
	hrtimer_init(&aw869xx->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	aw869xx->timer.function = aw869xx_vibrator_timer_func;
	INIT_WORK(&aw869xx->long_vibrate_work, aw869xx_long_vibrate_work_routine);
	INIT_WORK(&aw869xx->rtp_work, aw869xx_rtp_work_routine);
	mutex_init(&aw869xx->lock);
	mutex_init(&aw869xx->rtp_lock);
	return 0;
}

/******************************************************
 *
 * Digital Audio Interface
 *
 ******************************************************/
static int aw869xx_i2s_enable(struct aw869xx *aw869xx, bool flag)
{
	if (flag)
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_I2SCFG2,
			AW869XX_BIT_I2SCFG2_I2S_EN_MASK,
			AW869XX_BIT_I2SCFG2_I2S_ENABLE);
	else
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_I2SCFG2,
			AW869XX_BIT_I2SCFG2_I2S_EN_MASK,
			AW869XX_BIT_I2SCFG2_I2S_DISABLE);
	return 0;
}

static int aw869xx_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	aw_snd_soc_codec_t *codec = aw_get_codec(dai);
	struct aw869xx *aw869xx =
		aw_componet_codec_ops.aw_snd_soc_codec_get_drvdata(codec);

	pr_info("%s: enter\n", __func__);
	if (!aw869xx)
		return 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		mutex_lock(&aw869xx->lock);
		aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL2,
			AW869XX_BIT_SYSCTRL2_STANDBY_MASK,
			AW869XX_BIT_SYSCTRL2_STANDBY_OFF);
		mutex_unlock(&aw869xx->lock);
	}

	return 0;
}

static int aw869xx_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	aw_snd_soc_codec_t *codec = aw_get_codec(dai);

	pr_info("%s: fmt=0x%02X\n", __func__, fmt);
	if (!codec)
		return 0;

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		if ((fmt & SND_SOC_DAIFMT_MASTER_MASK) !=
			SND_SOC_DAIFMT_CBS_CFS) {
			pr_err("%s: invalid codec master mode\n", __func__);
			return -EINVAL;
		}
		break;
	default:
		pr_err("%s: unsupported DAI format %d\n",
			__func__, fmt & SND_SOC_DAIFMT_FORMAT_MASK);
		return -EINVAL;
	}
	return 0;
}

static int aw869xx_set_dai_sysclk(struct snd_soc_dai *dai,
	int clk_id, unsigned int freq, int dir)
{
	aw_snd_soc_codec_t *codec = aw_get_codec(dai);
	struct aw869xx *aw869xx =
		aw_componet_codec_ops.aw_snd_soc_codec_get_drvdata(codec);

	pr_info("%s: freq=%d\n", __func__, freq);

	if (!aw869xx)
		return 0;

	aw869xx->sysclk = freq;
	return 0;
}

static int aw869xx_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	aw_snd_soc_codec_t *codec = aw_get_codec(dai);
	struct aw869xx *aw869xx =
	    aw_componet_codec_ops.aw_snd_soc_codec_get_drvdata(codec);
	uint8_t i2sfs_val = 0;
	uint8_t i2sbck_val = 0;
	uint8_t tmp_val = 0;

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		pr_info("%s: steam is capture\n", __func__);
		return 0;
	}
	if (!aw869xx)
		return 0;
	pr_info("%s: requested rate: %d, sample size: %d\n",
		__func__, params_rate(params),
		snd_pcm_format_width(params_format(params)));

	/* get rate param */
	aw869xx->rate = params_rate(params);
	mutex_lock(&aw869xx->lock);

	/* get bit width */
	aw869xx->width = params_width(params);
	pr_info("%s: width = %d\n", __func__, aw869xx->width);
	i2sbck_val = params_width(params) * params_channels(params);
	/* match bit width */
	switch (aw869xx->width) {
	case 16:
		i2sfs_val = AW869XX_BIT_I2SCFG1_I2SFS_16BIT;
		break;
	case 24:
		i2sfs_val = AW869XX_BIT_I2SCFG1_I2SFS_24BIT;
		i2sbck_val = 32 * params_channels(params);
		break;
	case 32:
		i2sfs_val = AW869XX_BIT_I2SCFG1_I2SFS_32BIT;
		break;
	default:
		i2sfs_val = AW869XX_BIT_I2SCFG1_I2SFS_16BIT;
		i2sbck_val = 16 * params_channels(params);
		pr_err( "%s: width [%d]can not support\n", __func__, aw869xx->width);
		break;
	}
	pr_info("%s: i2sfs_val=0x%02X\n", __func__, i2sfs_val);
	/* set width */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_I2SCFG1,
		AW869XX_BIT_I2SCFG1_I2SFS_MASK, i2sfs_val);

	/* match bck mode */
	switch (i2sbck_val) {
	case 32:
		i2sbck_val = AW869XX_BIT_I2SCFG1_I2SBCK_32FS;
		break;
	case 48:
		i2sbck_val = AW869XX_BIT_I2SCFG1_I2SBCK_48FS;
		break;
	case 64:
		i2sbck_val = AW869XX_BIT_I2SCFG1_I2SBCK_64FS;
		break;
	default:
		pr_err( "%s: i2sbck [%d] can not support\n", __func__, i2sbck_val);
		i2sbck_val = AW869XX_BIT_I2SCFG1_I2SBCK_32FS;
		break;
	}
	/* set bck mode */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_I2SCFG1,
		AW869XX_BIT_I2SCFG1_I2SBCK_MASK, i2sbck_val);

	/* check i2s cfg */
	aw869xx_i2c_read(aw869xx, AW869XX_REG_I2SCFG1, &tmp_val);
	pr_info("%s: i2scfg1=0x%02X\n", __func__, tmp_val);

	mutex_unlock(&aw869xx->lock);

	return 0;
}

static int aw869xx_mute(struct snd_soc_dai *dai, int mute, int stream)
{
	aw_snd_soc_codec_t *codec = aw_get_codec(dai);
	struct aw869xx *aw869xx =
		aw_componet_codec_ops.aw_snd_soc_codec_get_drvdata(codec);
	uint8_t reg_val = 0;

	if (stream == SNDRV_PCM_STREAM_CAPTURE) {
		pr_info("%s: steam is capture\n", __func__);
		return 0;
	}
	if (!aw869xx)
		return 0;

	pr_info("%s: mute state=%d\n", __func__, mute);

	if (mute) {
		mutex_lock(&aw869xx->lock);
		aw869xx_i2s_enable(aw869xx, false);
		mutex_unlock(&aw869xx->lock);
	} else {
		mutex_lock(&aw869xx->lock);
		aw869xx_i2s_enable(aw869xx, true);
		usleep_range(2000, 3000);
		aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSST, &reg_val);
		pr_info("%s: sysst=0x%02X\n", __func__, reg_val);
		aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSST2, &reg_val);
		pr_info("%s: sysst2=0x%02X\n", __func__, reg_val);
		aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSER, &reg_val);
		pr_info("%s: syser=0x%02X\n", __func__, reg_val);
		aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &reg_val);
		if (reg_val != AW869XX_GLB_RD5_STATE) { // glb state err
			pr_err("%s: i2s config error, glb_state=0x%02X\n",
				__func__, reg_val);
			aw869xx_i2s_enable(aw869xx, false);
		} else {
			pr_info("%s: i2s config pass, glb_state=0x%02X\n",
				__func__, reg_val);
		}
		mutex_unlock(&aw869xx->lock);
	}

	return 0;
}

static void aw869xx_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	aw_snd_soc_codec_t *codec = aw_get_codec(dai);
	struct aw869xx *aw869xx =
		aw_componet_codec_ops.aw_snd_soc_codec_get_drvdata(codec);

	pr_info("%s: enter\n", __func__);
	if (!aw869xx)
		return;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		aw869xx->rate = 0;
		mutex_lock(&aw869xx->lock);
		/* aw869xx_i2s_enable to  false */
		mutex_unlock(&aw869xx->lock);
	}
}

static const struct snd_soc_dai_ops aw869xx_dai_ops = {
	.startup = aw869xx_startup,
	.set_fmt = aw869xx_set_fmt,
	.set_sysclk = aw869xx_set_dai_sysclk,
	.hw_params = aw869xx_hw_params,
	.mute_stream = aw869xx_mute,
	.shutdown = aw869xx_shutdown,
};

static struct snd_soc_dai_driver aw869xx_dai[] = {
	{
	.name = "aw869xx-aif",
	.id = 1,
	.playback = {
		.stream_name = "Speaker_Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = AW869XX_RATES,
		.formats = AW869XX_FORMATS,
	},
	.capture = {
		.stream_name = "Speaker_Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = AW869XX_RATES,
		.formats = AW869XX_FORMATS,
	},
	.ops = &aw869xx_dai_ops,
	.symmetric_rates = 1,
	},
};

static void aw869xx_add_codec_controls(struct aw869xx *aw869xx)
{
	pr_info("%s: enter\n", __func__);
}

static int aw869xx_probe(aw_snd_soc_codec_t *codec)
{
	struct aw869xx *aw869xx = NULL;

	pr_info("%s: enter\n", __func__);
	aw869xx = aw_componet_codec_ops.aw_snd_soc_codec_get_drvdata(codec);
	if (!aw869xx)
		return 0;

	aw869xx->codec = codec;
	aw869xx_add_codec_controls(aw869xx);
	pr_info("%s: exit\n", __func__);
	return 0;
}

#ifdef AW_KERNEL_VER_OVER_4_19
static void aw869xx_remove(struct snd_soc_component *component)
{
	pr_info("%s: enter\n", __func__);
}
#else
static int aw869xx_remove(aw_snd_soc_codec_t *codec)
{
	pr_info("%s: enter\n", __func__);
	return 0;
}
#endif

static unsigned int aw869xx_codec_read(aw_snd_soc_codec_t *codec,
	unsigned int reg)
{
	struct aw869xx *aw869xx = NULL;
	uint8_t value = 0;
	int ret;

	pr_info("%s: enter\n", __func__);
	aw869xx = aw_componet_codec_ops.aw_snd_soc_codec_get_drvdata(codec);
	if (!aw869xx)
		return 0;

	ret = aw869xx_i2c_read(aw869xx, reg, &value);
	if (ret < 0)
		pr_err( "%s: read register failed\n", __func__);

	return ret;
}

static int aw869xx_codec_write(aw_snd_soc_codec_t *codec,
	unsigned int reg, unsigned int value)
{
	int ret;

	struct aw869xx *aw869xx =
		aw_componet_codec_ops.aw_snd_soc_codec_get_drvdata(codec);
	pr_info("%s: enter ,reg:0x%02X = 0x%02X\n", __func__, reg, value);

	ret = aw869xx_i2c_write(aw869xx, (uint8_t) reg, (uint8_t) value);

	return ret;
}

#ifdef AW_KERNEL_VER_OVER_4_19
static struct snd_soc_component_driver soc_codec_dev_aw869xx = {
	.probe = aw869xx_probe,
	.remove = aw869xx_remove,
	.read = aw869xx_codec_read,
	.write = aw869xx_codec_write,
};
#else
static struct snd_soc_codec_driver soc_codec_dev_aw869xx = {
	.probe = aw869xx_probe,
	.remove = aw869xx_remove,
	.read = aw869xx_codec_read,
	.write = aw869xx_codec_write,
	.reg_cache_size = AW869XX_REG_MAX,
	.reg_word_size = 2,
};
#endif

static void aw869xx_interrupt_clear(struct aw869xx *aw869xx)
{
	unsigned char reg_val = 0;

	pr_info("%s enter\n", __func__);
	aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSINT, &reg_val);
	pr_debug("%s: reg SYSINT=0x%02X\n", __func__, reg_val);
}

static void aw869xx_interrupt_setup(struct aw869xx *aw869xx)
{
	unsigned char reg_val = 0;

	aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSINT, &reg_val);
	pr_info("%s: reg SYSINT=0x%02X\n", __func__, reg_val);

	/* edge int mode */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL7,
		AW869XX_BIT_SYSCTRL7_INT_MODE_MASK,
		AW869XX_BIT_SYSCTRL7_INT_MODE_EDGE);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSCTRL7,
		AW869XX_BIT_SYSCTRL7_INT_EDGE_MODE_MASK,
		AW869XX_BIT_SYSCTRL7_INT_EDGE_MODE_POS);
	/* int enable */
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSINTM,
		AW869XX_BIT_SYSINTM_BST_SCPM_MASK,
		AW869XX_BIT_SYSINTM_BST_SCPM_OFF);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSINTM,
		AW869XX_BIT_SYSINTM_BST_OVPM_MASK,
		AW869XX_BIT_SYSINTM_BST_OVPM_ON);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSINTM,
		AW869XX_BIT_SYSINTM_UVLM_MASK,
		AW869XX_BIT_SYSINTM_UVLM_ON);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSINTM,
		AW869XX_BIT_SYSINTM_OCDM_MASK,
		AW869XX_BIT_SYSINTM_OCDM_ON);
	aw869xx_i2c_write_bits(aw869xx, AW869XX_REG_SYSINTM,
		AW869XX_BIT_SYSINTM_OTM_MASK,
		AW869XX_BIT_SYSINTM_OTM_ON);
	aw869xx_haptic_set_bst_vol(aw869xx, HAPTIC_BST_VOL_9V);
}

static void aw869xx_sysint_ff_aei_fault(struct aw869xx *aw869xx)
{
	unsigned char glb_state_val = 0;
	unsigned int buf_len = 0;

	if (!(aw869xx->rtp_init)) {
		pr_info("%s: aw869xx rtp init = %d, init error\n",
			__func__, aw869xx->rtp_init);
		return;
	}
	while ((!aw869xx_haptic_rtp_get_fifo_afs(aw869xx)) &&
		(aw869xx->play_mode == AW869XX_HAPTIC_RTP_MODE)) {
		mutex_lock(&aw869xx->rtp_lock);
		pr_info("%s: aw869xx rtp mode fifo update, cnt=%d\n",
			__func__, aw869xx->rtp_cnt);
		if (!aw869xx_rtp) {
			pr_info("%s:aw869xx_rtp is null, break\n", __func__);
			mutex_unlock(&aw869xx->rtp_lock);
			break;
		}
		if ((aw869xx_rtp->len - aw869xx->rtp_cnt) < (aw869xx->ram.base_addr >> 2))
			buf_len = aw869xx_rtp->len - aw869xx->rtp_cnt;
		else
			buf_len = (aw869xx->ram.base_addr >> 2);
		aw869xx->rtp_update_flag =
			aw869xx_i2c_writes(aw869xx, AW869XX_REG_RTPDATA,
				&aw869xx_rtp->data[aw869xx->rtp_cnt], buf_len);
		aw869xx->rtp_cnt += buf_len;
		aw869xx_i2c_read(aw869xx, AW869XX_REG_GLBRD5, &glb_state_val);
		if ((aw869xx->rtp_cnt == aw869xx_rtp->len) ||
			((glb_state_val & 0x0f) == 0)) {
			pr_info("%s: rtp update complete\n", __func__);
 			aw869xx_haptic_set_rtp_aei(aw869xx, false);
			aw869xx->rtp_cnt = 0;
			aw869xx->rtp_init = 0;
			mutex_unlock(&aw869xx->rtp_lock);
			break;
		}
		mutex_unlock(&aw869xx->rtp_lock);
	}

}

static irqreturn_t aw869xx_irq(int irq, void *data)
{
	struct aw869xx *aw869xx = data;
	unsigned char reg_val = 0;
	unsigned char sys_state_val = 0;

	if (!aw869xx)
		return IRQ_HANDLED;
	aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSINT, &reg_val);
	aw869xx_i2c_read(aw869xx, AW869XX_REG_SYSST, &sys_state_val);
	pr_info("%s: reg SYSINT=0x%02X\n", __func__, reg_val);
	if ((reg_val & AW869XX_BIT_SYSINT_BST_OVPI) &&
		((sys_state_val & AW869XX_BIT_SYSST_BST_OVPS)))
		pr_err( "%s chip ov int error\n", __func__);
	if (reg_val & AW869XX_BIT_SYSINT_UVLI)
		pr_err( "%s chip uvlo int error\n", __func__);
	if (reg_val & AW869XX_BIT_SYSINT_OCDI)
		pr_err( "%s chip over current int error\n",
			   __func__);
	if (reg_val & AW869XX_BIT_SYSINT_OTI)
		pr_err( "%s chip over temperature int error\n",
			   __func__);
	if (reg_val & AW869XX_BIT_SYSINT_DONEI)
		pr_info("%s chip playback done\n", __func__);

	if (reg_val & AW869XX_BIT_SYSINT_FF_AEI) {
		pr_info("%s: aw869xx rtp fifo almost empty\n", __func__);
		aw869xx_sysint_ff_aei_fault(aw869xx);
	}

	if (reg_val & AW869XX_BIT_SYSINT_FF_AFI)
		pr_info("%s: aw869xx rtp mode fifo almost full\n", __func__);

	if (aw869xx->play_mode != AW869XX_HAPTIC_RTP_MODE)
		aw869xx_haptic_set_rtp_aei(aw869xx, false);

	pr_info("%s exit\n", __func__);

	return IRQ_HANDLED;
}

static int aw869xx_parse_dt(struct device *dev, struct aw869xx *aw869xx,
	struct device_node *np)
{
	unsigned int val;
	unsigned int bstcfg_temp[5] = { 0x2a, 0x24, 0x9a, 0x40, 0x91 }; // default bstcfg val
	unsigned int prctmode_temp[3] = {0};
	unsigned int sine_array_temp[4] = { 0x05, 0xB2, 0xFF, 0xEF }; // default sine val
	unsigned int trig_config_temp[24] = { 1, 0, 1, 1, 1, 2, 0, 0,
		1, 0, 0, 1, 0, 2, 0, 0,
		1, 0, 0, 1, 0, 2, 0, 0
	};

	aw869xx->reset_gpio = of_get_named_gpio(np, "reset-gpio", 0);
	if (aw869xx->reset_gpio < 0) {
		pr_err("%s: no reset gpio provide\n", __func__);
		return -EINVAL;
	} else {
		pr_info("%s: reset gpio provide ok\n", __func__);
	}
	aw869xx->irq_gpio = of_get_named_gpio(np, "irq-gpio", 0);
	if (aw869xx->irq_gpio < 0)
		pr_err("%s: no irq gpio provide\n", __func__);
	else
		pr_info("%s: irq gpio provide ok\n", __func__);

	val = of_property_read_u32(np, "vib_mode", &aw869xx->dts_info.mode);
	if (val != 0)
		pr_info("%s vib_mode not found\n", __func__);

	val = of_property_read_u32(np, "vib_f0_ref", &aw869xx->dts_info.f0_ref);
	if (val != 0)
		pr_info("%s vib_f0_ref not found\n", __func__);

 	val = of_property_read_u32(np, "vib_f0_cali_percent",
		&aw869xx->dts_info.f0_cali_percent);
	if (val != 0)
		pr_info("%s vib_f0_cali_percent not found\n", __func__);

	val = of_property_read_u32(np, "vib_cont_drv1_lvl",
		&aw869xx->dts_info.cont_drv1_lvl_dt);
	if (val != 0)
		pr_info("%s vib_cont_drv1_lvl not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_drv2_lvl",
		&aw869xx->dts_info.cont_drv2_lvl_dt);
	if (val != 0)
		pr_info("%s vib_cont_drv2_lvl not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_drv1_time",
		&aw869xx->dts_info.cont_drv1_time_dt);
	if (val != 0)
		pr_info("%s vib_cont_drv1_time not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_drv2_time",
		&aw869xx->dts_info.cont_drv2_time_dt);
	if (val != 0)
		pr_info("%s vib_cont_drv2_time not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_drv_width",
		&aw869xx->dts_info.cont_drv_width);
	if (val != 0)
		pr_info("%s vib_cont_drv_width not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_wait_num",
		&aw869xx->dts_info.cont_wait_num_dt);
	if (val != 0)
		pr_info("%s vib_cont_wait_num not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_bst_brk_gain",
		&aw869xx->dts_info.cont_bst_brk_gain);
	if (val != 0)
		pr_info("%s vib_cont_bst_brk_gain not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_brk_gain",
		&aw869xx->dts_info.cont_brk_gain);
	if (val != 0)
		pr_info("%s vib_cont_brk_gain not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_tset",
		&aw869xx->dts_info.cont_tset);
	if (val != 0)
		pr_info("%s vib_cont_tset not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_bemf_set",
		&aw869xx->dts_info.cont_bemf_set);
	if (val != 0)
		pr_info("%s vib_cont_bemf_set not found\n", __func__);
	val = of_property_read_u32(np, "vib_d2s_gain",
		&aw869xx->dts_info.d2s_gain);
	if (val != 0)
		pr_info("%s vib_d2s_gain not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_brk_time",
		&aw869xx->dts_info.cont_brk_time_dt);
	if (val != 0)
		pr_info("%s vib_cont_brk_time not found\n", __func__);
	val = of_property_read_u32(np, "vib_cont_track_margin",
		&aw869xx->dts_info.cont_track_margin);
	if (val != 0)
		pr_info("%s vib_cont_track_margin not found\n", __func__);
	aw869xx->dts_info.is_enabled_auto_bst = of_property_read_bool(np,
		"vib_is_enabled_auto_bst");
	pr_info("%s aw869xx->dts_info.is_enabled_auto_bst = %d\n", __func__,
		aw869xx->dts_info.is_enabled_auto_bst);
	aw869xx->dts_info.is_enabled_auto_bst = true;
	aw869xx->dts_info.is_enabled_i2s = of_property_read_bool(np,
		"vib_is_enabled_i2s");
	pr_info("%s aw869xx->dts_info.is_enabled_i2s = %d\n",
		__func__, aw869xx->dts_info.is_enabled_i2s);
	aw869xx->dts_info.is_enabled_powerup_f0_cali = of_property_read_bool(np,
		"vib_is_enabled_powerup_f0_cali");
	pr_info("%s aw869xx->dts_info.is_enabled_powerup_f0_cali = %d\n",
		__func__, aw869xx->dts_info.is_enabled_powerup_f0_cali);
	val = of_property_read_u32_array(np, "vib_bstcfg", bstcfg_temp,
		ARRAY_SIZE(bstcfg_temp));
	if (val != 0)
		pr_info("%s vib_bstcfg not found\n", __func__);
	memcpy(aw869xx->dts_info.bstcfg, bstcfg_temp, sizeof(bstcfg_temp));
	val = of_property_read_u32_array(np, "vib_prctmode",
		prctmode_temp, ARRAY_SIZE(prctmode_temp));
	if (val != 0)
		pr_info("%s vib_prctmode not found\n", __func__);
	memcpy(aw869xx->dts_info.prctmode, prctmode_temp, sizeof(prctmode_temp));
	val = of_property_read_u32_array(np, "vib_sine_array", sine_array_temp,
		ARRAY_SIZE(sine_array_temp));
	if (val != 0)
		pr_info("%s vib_sine_array not found\n", __func__);
	memcpy(aw869xx->dts_info.sine_array, sine_array_temp,
		sizeof(sine_array_temp));
	val = of_property_read_u32_array(np, "vib_trig_config", trig_config_temp,
		ARRAY_SIZE(trig_config_temp));
	if (val != 0)
		pr_info("%s vib_trig_config not found\n", __func__);
	memcpy(aw869xx->dts_info.trig_config, trig_config_temp, sizeof(trig_config_temp));

	return 0;
}

static int aw869xx_hw_reset(struct aw869xx *aw869xx)
{
	pr_info("%s enter\n", __func__);

	if (aw869xx && gpio_is_valid(aw869xx->reset_gpio)) {
		gpio_set_value_cansleep(aw869xx->reset_gpio, 0);
		usleep_range(1000, 2000);
		gpio_set_value_cansleep(aw869xx->reset_gpio, 1);
		usleep_range(3500, 4000);
	} else {
		pr_err( "%s:  failed\n", __func__);
	}

	return 0;
}

static int aw869xx_read_chipid(struct aw869xx *aw869xx)
{
	int ret;
	unsigned char cnt = 0;
	unsigned char reg = 0;

	while (cnt < AW_READ_CHIPID_RETRIES) {
		/* hardware reset */
		aw869xx_hw_reset(aw869xx);
		/* aw869xx_haptic_softreset */
		ret = aw869xx_i2c_read(aw869xx, AW869XX_REG_ID, &reg);
		if (ret < 0)
			pr_err( "%s: failed to read id: %d\n", __func__, ret);

		switch (reg) {
		case AW86905_CHIPID:
			pr_info("%s aw86905, id: 0x%02X\n", __func__, reg);
			aw869xx->chipid = AW86905_CHIPID;
			aw869xx->bst_pc = AW869XX_HAPTIC_BST_PC_L1;
			aw869xx->dts_info.is_enabled_i2s = false;
			return 0;
		case AW86907_CHIPID:
			pr_info("%s aw86907 id: 0x%02X\n", __func__, reg);
			aw869xx->chipid = AW86907_CHIPID;
			aw869xx->bst_pc = AW869XX_HAPTIC_BST_PC_L2;
			aw869xx->dts_info.is_enabled_i2s = false;
			return 0;
		case AW86915_CHIPID:
			pr_info("%s aw86915, id: 0x%02X\n", __func__, reg);
			aw869xx->chipid = AW86915_CHIPID;
			aw869xx->bst_pc = AW869XX_HAPTIC_BST_PC_L1;
			return 0;
		case AW86917_CHIPID:
			pr_info("%s aw86917, id: 0x%02X\n", __func__, reg);
			aw869xx->chipid = AW86917_CHIPID;
			aw869xx->bst_pc = AW869XX_HAPTIC_BST_PC_L2;
			return 0;
		default:
			pr_info("%s unsupported device revision (0x%02X)\n",
				__func__, reg);
			break;
		}
		cnt++;
		usleep_range(AW_READ_CHIPID_RETRY_DELAY * 1000,
			AW_READ_CHIPID_RETRY_DELAY * 1000 + 500);
	}
	return -EINVAL;
}

static int aw869xx_brk_params_init(struct aw869xx *aw869xx)
{
	pr_info("%s enter\n", __func__);
	aw869xx->rtp_d2s_gain = DEFAULT_RTP_D2S_GAIN;
	aw869xx->rtp_brk_gain = DEFAULT_RTP_BRK_GAIN;
	aw869xx->rtp_brk_time = DEFAULT_RTP_BRK_TIME;
	aw869xx->ram_short_d2s_gain = DEFAULT_RAM_SHORT_D2S_GAIN;
	aw869xx->ram_short_brk_gain = DEFAULT_RAM_SHORT_BRK_GAIN;
	aw869xx->ram_short_brk_time = DEFAULT_RAM_SHORT_BRK_TIME;
	aw869xx->ram_long_d2s_gain = DEFAULT_RAM_LONG_D2S_GAIN;
	aw869xx->ram_long_brk_gain = DEFAULT_RAM_LONG_BRK_GAIN;
	aw869xx->ram_long_brk_time = DEFAULT_RAM_LONG_BRK_TIME;
	return 0;
}

static int aw869xx_gpio_source_request(struct aw869xx *aw869xx)
{
	int ret;

	if (!gpio_is_valid(aw869xx->reset_gpio)) {
		pr_info("%s reset gpio number error\n", __func__);
		return -EINVAL;
	}
	if (!gpio_is_valid(aw869xx->irq_gpio)) {
		pr_info("%s reset irq gpio error\n", __func__);
		return -EINVAL;
	}

	ret = devm_gpio_request_one(aw869xx->dev, aw869xx->reset_gpio,
		GPIOF_OUT_INIT_LOW, "aw869xx_rst");
	if (ret) {
		pr_err("%s: rst request failed\n", __func__);
		return -EINVAL;
	}
	ret = devm_gpio_request_one(aw869xx->dev, aw869xx->irq_gpio,
		GPIOF_DIR_IN, "aw869xx_int");
	if (ret) {
		pr_err("%s: int request failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int aw869xx_check_chip_quality(struct aw869xx *aw869xx)
{
	int ret;
	unsigned char reg = 0;

	// chip qualify
	ret = aw869xx_i2c_read(aw869xx, 0x64, &reg); // quality reg 0x64
	if (ret < 0)
		pr_err("%s: failed to read register 0x64: %d\n", __func__, ret);

	if (!(reg & 0x80)) {
		pr_err("%s:unqualified chip!\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int aw869xx_irq_request(struct aw869xx *aw869xx)
{
	int ret;
	int irq_flags;

	if (gpio_is_valid(aw869xx->irq_gpio) &&
		!(aw869xx->flags & AW869XX_FLAG_SKIP_INTERRUPTS)) {
		/* register irq handler */
		aw869xx_interrupt_setup(aw869xx);
		irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT;
		ret = devm_request_threaded_irq(aw869xx->dev,
			gpio_to_irq(aw869xx->irq_gpio),
			NULL, aw869xx_irq, irq_flags, "aw869xx", aw869xx);
		if (ret != 0) {
			pr_err("%s: failed to request IRQ %d: %d\n",
				__func__, gpio_to_irq(aw869xx->irq_gpio), ret);
			return -EINVAL;
		}
	} else {
		pr_info("%s skipping IRQ registration\n", __func__);
		/* disable feature support if gpio was invalid */
		aw869xx->flags |= AW869XX_FLAG_SKIP_INTERRUPTS;
	}
	return 0;
}

static int aw869xx_i2c_probe(struct i2c_client *i2c,
	const struct i2c_device_id *id)
{
	struct aw869xx *aw869xx = NULL;
	struct device_node *np = i2c->dev.of_node;
	struct snd_soc_dai_driver *dai = NULL;
	int ret = -1;

	pr_info("%s enter\n", __func__);
	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
		pr_err("%s check_functionality failed\n", __func__);
		return -EIO;
	}
	aw869xx = devm_kzalloc(&i2c->dev, sizeof(struct aw869xx), GFP_KERNEL);
	if (!aw869xx)
		return -ENOMEM;
	aw869xx->dev = &i2c->dev;
	aw869xx->i2c = i2c;
	i2c_set_clientdata(i2c, aw869xx);

	if (np) {
		ret = aw869xx_parse_dt(&i2c->dev, aw869xx, np);
		if (ret) {
			pr_err("%s: fail to parse device tree node\n", __func__);
			goto err_parse_dt;
		}
	} else {
		aw869xx->reset_gpio = -1;
		aw869xx->irq_gpio = -1;
	}
	if (aw869xx_gpio_source_request(aw869xx) < 0)
		goto err_id;
	if (aw869xx_read_chipid(aw869xx) < 0)
		goto err_id;
	if (aw869xx_check_chip_quality(aw869xx) < 0)
		goto err_id;
	dai = devm_kzalloc(&i2c->dev, sizeof(aw869xx_dai), GFP_KERNEL);
	if (!dai)
		goto err_dai_kzalloc;
	wakeup_source_init(&lra_lock, "lra869xx");
	memcpy(dai, aw869xx_dai, sizeof(aw869xx_dai));
	pr_info("%s: dai->name(%s)\n", __func__, dai->name);

	ret = aw_componet_codec_ops.aw_snd_soc_register_codec(&i2c->dev,
		&soc_codec_dev_aw869xx, dai, ARRAY_SIZE(aw869xx_dai));
	if (ret < 0) {
		pr_err("%s failed to register aw869xx: %d\n", __func__, ret);
		goto err_register_codec;
	}

	if (aw869xx_irq_request(aw869xx) < 0)
		goto err_irq;
	dev_set_drvdata(&i2c->dev, aw869xx);
	aw869xx_vibrator_init(aw869xx);
	aw869xx_haptic_init(aw869xx);
	aw869xx_ram_work_init(aw869xx);
	aw869xx_brk_params_init(aw869xx);
	aw869xx->index = AW869XX_LONG_VIB_EFFECT_ID;
	g_aw869xx = aw869xx;
	pr_info("%s probe completed successfully\n", __func__);
	return 0;

err_irq:
	aw_componet_codec_ops.aw_snd_soc_unregister_codec(&i2c->dev);
err_register_codec:
	devm_kfree(&i2c->dev, dai);
	dai = NULL;
err_dai_kzalloc:
err_id:
	if (gpio_is_valid(aw869xx->irq_gpio))
		devm_gpio_free(&i2c->dev, aw869xx->irq_gpio);
	if (gpio_is_valid(aw869xx->reset_gpio))
		devm_gpio_free(&i2c->dev, aw869xx->reset_gpio);
err_parse_dt:
	devm_kfree(&i2c->dev, aw869xx);
	aw869xx = NULL;
	return ret;
}

static int aw869xx_i2c_remove(struct i2c_client *i2c)
{
	struct aw869xx *aw869xx = i2c_get_clientdata(i2c);

	pr_info("%s enter\n", __func__);
	if (!aw869xx)
		return 0;

	devm_free_irq(&i2c->dev, gpio_to_irq(aw869xx->irq_gpio), aw869xx);
	if (gpio_is_valid(aw869xx->irq_gpio))
		devm_gpio_free(&i2c->dev, aw869xx->irq_gpio);

	aw_componet_codec_ops.aw_snd_soc_unregister_codec(&i2c->dev);

	if (gpio_is_valid(aw869xx->reset_gpio))
		devm_gpio_free(&i2c->dev, aw869xx->reset_gpio);
	devm_kfree(&i2c->dev, aw869xx);
	aw869xx = NULL;

	return 0;
}

static int __maybe_unused aw869xx_suspend(struct device *dev)
{
	int ret = 0;
	struct aw869xx *aw869xx = dev_get_drvdata(dev);

	if (!aw869xx)
		return 0;

	mutex_lock(&aw869xx->lock);
	aw869xx_haptic_stop(aw869xx);
	mutex_unlock(&aw869xx->lock);

	return ret;
}

static int __maybe_unused aw869xx_resume(struct device *dev)
{
	return 0;
}

static SIMPLE_DEV_PM_OPS(aw869xx_pm_ops, aw869xx_suspend, aw869xx_resume);

static const struct i2c_device_id aw869xx_i2c_id[] = {
	{AW869XX_I2C_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, aw869xx_i2c_id);

static const struct of_device_id aw869xx_dt_match[] = {
	{.compatible = "awinic,awinic_haptic"},
	{},
};

static struct i2c_driver aw869xx_i2c_driver = {
	.driver = {
		   .name = AW869XX_I2C_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(aw869xx_dt_match),
		   },
	.probe = aw869xx_i2c_probe,
	.remove = aw869xx_i2c_remove,
	.id_table = aw869xx_i2c_id,
};

static int __init aw869xx_i2c_init(void)
{
	int ret = 0;

	pr_info("aw869xx driver version %s\n", AW869XX_VERSION);
	ret = i2c_add_driver(&aw869xx_i2c_driver);
	if (ret) {
		pr_err("fail to add aw869xx device into i2c\n");
		return ret;
	}
	return 0;
}

module_init(aw869xx_i2c_init);

static void __exit aw869xx_i2c_exit(void)
{
	i2c_del_driver(&aw869xx_i2c_driver);
}

module_exit(aw869xx_i2c_exit);

MODULE_DESCRIPTION("AW869XX Haptic Driver");
MODULE_LICENSE("GPL v2");
