/*
 * au_pa.c
 *
 * au pa driver
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#include <sound/au_pa.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio_keys.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/version.h>

#include <sound/soc.h>

#include "asoc_adapter.h"
#include "linux/hisi/audio_log.h"

#define LOG_TAG "au pa"

/* ALSA dapm control reg bit[31] negative  = no direct dapm */
#define ALSA_REG_MASK 0x7FFFFFFF
#define INVALID_REG_VALUE 0xFFFFFFFF
#define PGA_GAIN_UP_THRESHOLD 60
#define PGA_GAIN_LOW_THRESHOLD (-120)
#define REG_MAX_OFFSET 31
#define REG_MAX_LEN 32
#define MAX_VAL_ON_BIT(bit) ((0x1 << (bit)) - 1)
#define MASK_ON_BIT(bit, offset) (MAX_VAL_ON_BIT(bit) << (offset))

enum pga_reg_type {
	LEFT_PGA,
	RIGHT_PGA,
	PGA_BUTT,
};

struct au_pa_platform_para {
	struct snd_soc_component *codec;
	unsigned int gain_reg[PGA_BUTT];
	unsigned int gain_reg_value[PGA_BUTT];
	unsigned int gain_reg_len;
	unsigned int gain_reg_offset;
	unsigned int gain_mask;
	signed char gain_change_db;
};

struct au_pa_pinctrl {
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct pinctrl_state *pin_idle;
};

struct au_pa_priv {
	struct au_pa_pinctrl pin_ctrl;
	au_pa_callback callback[CALLBACK_COUNT];
	struct au_pa_platform_para platform_para;
};

enum au_pa_pinctrl_state {
	STATE_DEFAULT = 0,
	STATE_IDLE,
};

enum audio_gain_operation {
	MODIFY_AUDIO_GAIN = 0,
	RESTORE_AUDIO_GAIN,
};

static struct au_pa_priv *g_privdata = NULL;

static unsigned int audio_calc_db_value(
	struct au_pa_platform_para *platform_para,
	unsigned int original_val)
{
	signed char change_val;
	unsigned int ret;
	int sum;

	change_val = (original_val & platform_para->gain_mask) >> platform_para->gain_reg_offset;
	sum = change_val + platform_para->gain_change_db;

	if (sum < PGA_GAIN_LOW_THRESHOLD)
		change_val = PGA_GAIN_LOW_THRESHOLD;
	else if (sum > PGA_GAIN_UP_THRESHOLD)
		change_val = PGA_GAIN_UP_THRESHOLD;
	else
		change_val = sum;

	ret = (original_val & (~(platform_para->gain_mask))) |
		(change_val << platform_para->gain_reg_offset);
	AUDIO_LOGI("the value of audio register will be changed to 0x%x\n", ret);

	return ret;
}

static void audio_exec_gain_modify(struct au_pa_platform_para *platform_para)
{
	unsigned int i;
	unsigned int old_val;
	unsigned int new_val;

	for (i = 0; i < PGA_BUTT; i++) {
		if (platform_para->gain_reg[i] == 0)
			continue;

		old_val = snd_soc_component_read32(platform_para->codec,
			platform_para->gain_reg[i]);
		AUDIO_LOGI("the value of audio down register 0x%x is 0x%x\n",
			platform_para->gain_reg[i], old_val);

		platform_para->gain_reg_value[i] = old_val;
		new_val = audio_calc_db_value(platform_para, old_val);
		snd_soc_component_update_bits(platform_para->codec,
			platform_para->gain_reg[i],
			platform_para->gain_mask,
			new_val);
	}
}

static void audio_exec_gain_restore(struct au_pa_platform_para *platform_para)
{
	unsigned int i;

	for (i = 0; i < PGA_BUTT; i++) {
		if (platform_para->gain_reg[i] == 0)
			continue;

		AUDIO_LOGI("the saved value of audio down register 0x%x is 0x%x\n",
		platform_para->gain_reg[i], platform_para->gain_reg_value[i]);

		if (platform_para->gain_reg_value[i] != INVALID_REG_VALUE) {
			snd_soc_component_update_bits(platform_para->codec,
				platform_para->gain_reg[i],
				platform_para->gain_mask,
				platform_para->gain_reg_value[i]);
			platform_para->gain_reg_value[i] = INVALID_REG_VALUE;
		}
	}
}

static int audio_process_gain(enum audio_gain_operation operation)
{
	struct au_pa_priv *priv = g_privdata;
	struct au_pa_platform_para *platform_para = NULL;

	IN_FUNCTION;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL, process error\n");
		return -ENODEV;
	}

	platform_para = &priv->platform_para;

	AUDIO_LOGI("gain change db is %d\n", platform_para->gain_change_db);
	if (platform_para->gain_change_db == 0)
		return 0;

	if (platform_para->gain_mask == 0) {
		AUDIO_LOGE("gain mask is %x\n", platform_para->gain_mask);
		return -EFAULT;
	}

	switch (operation) {
	case MODIFY_AUDIO_GAIN:
		audio_exec_gain_modify(platform_para);
		break;
	case RESTORE_AUDIO_GAIN:
		audio_exec_gain_restore(platform_para);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int audio_modify_gain(void)
{
	return audio_process_gain(MODIFY_AUDIO_GAIN);
}

static int audio_restore_gain(void)
{
	return audio_process_gain(RESTORE_AUDIO_GAIN);
}

static void au_pa_read_config(struct platform_device *pdev)
{
	struct au_pa_priv *priv = g_privdata;
	struct device_node *np = pdev->dev.of_node;
	unsigned int val = 0;

	IN_FUNCTION;

	if (of_property_read_u32(np, "gain_change_db", &val))
		goto end;

	if (val == 0)
		goto end;
	priv->platform_para.gain_change_db = val;

	val = 0;
	if (of_property_read_u32(np, "gain_reg_offset", &val))
		goto end;
	if (val > REG_MAX_OFFSET)
		goto end;
	priv->platform_para.gain_reg_offset = val;

	val = 0;
	if (of_property_read_u32(np, "gain_reg_len", &val))
		goto end;
	if (val > REG_MAX_LEN)
		goto end;
	priv->platform_para.gain_reg_len = val;

	of_property_read_variable_u32_array(np, "gain_reg",
		priv->platform_para.gain_reg, LEFT_PGA, PGA_BUTT);
end:
	AUDIO_LOGI("gain change db is %d, gain reg offset is %d, gain reg len is %d\n",
		priv->platform_para.gain_change_db,
		priv->platform_para.gain_reg_offset,
		priv->platform_para.gain_reg_len);
	AUDIO_LOGI("left pga reg is 0x%x, right pga reg is 0x%x",
		priv->platform_para.gain_reg[LEFT_PGA],
		priv->platform_para.gain_reg[RIGHT_PGA]);

	OUT_FUNCTION;
	return;
}

int au_pa_set_default_func(struct snd_soc_component *codec)
{
	struct au_pa_priv *priv = g_privdata;
	unsigned int i;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is NULL\n");
		return -EFAULT;
	}

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL\n");
		return -ENODEV;
	}

	for (i = 0; i < CALLBACK_COUNT; i++) {
		if (priv->callback[i] != NULL) {
			AUDIO_LOGE("function already exists, type = %d\n", i);
			return -EINVAL;
		}
	}

	for (i = 0; i < PGA_BUTT; i++)
		priv->platform_para.gain_reg[i] &= ALSA_REG_MASK;

	priv->platform_para.codec = codec;
	priv->platform_para.gain_mask =
		MASK_ON_BIT(priv->platform_para.gain_reg_len,
		priv->platform_para.gain_reg_offset);
	priv->callback[MODIFY_GAIN] = audio_modify_gain;
	priv->callback[RESTORE_GAIN] = audio_restore_gain;

	return 0;
}

static int exec_codec_callback(enum au_pa_callback_type func_type)
{
	struct au_pa_priv *priv = g_privdata;
	int ret;

	IN_FUNCTION;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL, process error\n");
		return -ENODEV;
	}

	if (priv->callback[func_type] == NULL) {
		AUDIO_LOGE("function is not registered, type = %d\n", func_type);
		return -EINVAL;
	}

	ret = priv->callback[func_type]();

	OUT_FUNCTION;

	return ret;
}

int au_pa_modify_gain(void)
{
	int ret;

	ret = exec_codec_callback(MODIFY_GAIN);

	return ret;
}

int au_pa_restore_gain(void)
{
	int ret;

	ret = exec_codec_callback(RESTORE_GAIN);

	return ret;
}

static int set_pinctrl_state(enum au_pa_pinctrl_state state)
{
	int ret = 0;
	struct au_pa_priv *priv = g_privdata;

	IN_FUNCTION;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL, set pins state error\n");
		return -ENODEV;
	}

	if (priv->pin_ctrl.pctrl == NULL) {
		AUDIO_LOGE("pinctrl is NULL, set pins state error\n");
		return -ENODEV;
	}

	switch (state) {
	case STATE_DEFAULT:
		ret = pinctrl_select_state(priv->pin_ctrl.pctrl,
			priv->pin_ctrl.pin_default);
		if (ret)
			AUDIO_LOGE("set pins to default error, ret = %d\n", ret);
		break;

	case STATE_IDLE:
		ret = pinctrl_select_state(priv->pin_ctrl.pctrl,
			priv->pin_ctrl.pin_idle);
		if (ret)
			AUDIO_LOGE("set pins to idle error, ret = %d\n", ret);
		break;
	}

	OUT_FUNCTION;

	return ret;
}

/*
 * Special treatment of AU PA ,The pinctrl process was used to handle
 * the corresponding AU PA business.
 */
int au_pa_cfg_iomux(enum au_pa_mode mode)
{
	int ret = 0;

	IN_FUNCTION;

	switch (mode) {
	case NORMAL_MODE:
		ret = set_pinctrl_state(STATE_DEFAULT);
		break;

	case RESET_MODE:
		ret = set_pinctrl_state(STATE_IDLE);
		break;

	default:
		AUDIO_LOGE("mode is error, mode = %d\n", mode);
		break;
	}

	OUT_FUNCTION;

	return ret;
}

int au_pa_func_register(enum au_pa_callback_type func_type,
	au_pa_callback func)
{
	struct au_pa_priv *priv = g_privdata;

	IN_FUNCTION;

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL, can not register function\n");
		return -ENODEV;
	}

	if (func_type >= CALLBACK_COUNT) {
		AUDIO_LOGE("function is invalid, type = %d\n", func_type);
		return -EINVAL;
	}

	if (priv->callback[func_type] != NULL) {
		AUDIO_LOGE("function already exists, type = %d\n", func_type);
		return -EINVAL;
	}

	priv->callback[func_type] = func;

	OUT_FUNCTION;

	return 0;
}

static int pinctrl_init(struct platform_device *pdev)
{
	struct pinctrl *p = NULL;
	struct pinctrl_state *state = NULL;
	struct au_pa_priv *priv = NULL;

	IN_FUNCTION;

	priv = platform_get_drvdata(pdev);

	p = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(p)) {
		AUDIO_LOGE("can not get pinctrl\n");
		return PTR_ERR(p);
	}
	priv->pin_ctrl.pctrl = p;

	state = pinctrl_lookup_state(p, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(state)) {
		AUDIO_LOGE("can not get default state:%li\n", PTR_ERR(state));
		return PTR_ERR(state);
	}
	priv->pin_ctrl.pin_default = state;

	state = pinctrl_lookup_state(p, PINCTRL_STATE_IDLE);
	if (IS_ERR(state)) {
		AUDIO_LOGE("can not get idle state:%li\n", PTR_ERR(state));
		return PTR_ERR(state);
	}
	priv->pin_ctrl.pin_idle = state;

	AUDIO_LOGI("pinctrl init ok\n");

	OUT_FUNCTION;

	return 0;
}

static void pinctrl_deinit(struct platform_device *pdev)
{
	struct au_pa_priv *priv = NULL;

	IN_FUNCTION;

	priv = platform_get_drvdata(pdev);

	devm_pinctrl_put(priv->pin_ctrl.pctrl);

	AUDIO_LOGI("pinctrl deinit ok\n");

	OUT_FUNCTION;
}

static int au_pa_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;

	AUDIO_LOGI("begin\n");

	g_privdata = devm_kzalloc(dev, sizeof(struct au_pa_priv), GFP_KERNEL);
	if (g_privdata == NULL) {
		AUDIO_LOGE("memory alloc failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, g_privdata);

	ret = pinctrl_init(pdev);
	if (ret) {
		AUDIO_LOGE("pinctrl init failed, err code 0x%x\n", ret);
		return ret;
	}

	au_pa_read_config(pdev);

	AUDIO_LOGI("end\n");
	return ret;
}

static void au_pa_shutdown(struct platform_device *pdev)
{
	struct au_pa_priv *priv = g_privdata;

	AUDIO_LOGI("begin\n");

	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL\n");
		return;
	}

	pinctrl_deinit(pdev);

	g_privdata = NULL;

	AUDIO_LOGI("end\n");
}

static const struct of_device_id au_pa_match[] = {
	{ .compatible = "hisilicon,au-pa", },
	{},
};

static struct platform_driver au_pa_driver = {
	.driver = {
		.name  = "au-pa",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(au_pa_match),
	},
	.probe = au_pa_probe,
	.shutdown = au_pa_shutdown,
};

static int __init au_pa_init(void)
{
	AUDIO_LOGI("begin\n");
	return platform_driver_register(&au_pa_driver);
}
fs_initcall_sync(au_pa_init);

static void __exit au_pa_exit(void)
{
	AUDIO_LOGI("begin\n");
	platform_driver_unregister(&au_pa_driver);
	AUDIO_LOGI("bnd\n");
}
module_exit(au_pa_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ASoC au pa driver");

