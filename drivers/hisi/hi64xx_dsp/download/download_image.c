/*
 * download_image.c
 *
 * hi64xx codec dsp img download
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

#include "download_image.h"

#include <linux/kernel.h>
#include <linux/firmware.h>
#include <linux/errno.h>
#include <linux/dma-mapping.h>

#include <linux/hisi/hi64xx/hi64xx_dsp_regs.h>
#include <linux/hisi/hi64xx/hi64xx_utils.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/audio_log.h>
#include <hi64xx_dsp_interface.h>
#include <rdr_hisi_audio_codec.h>
#include <rdr_hisi_audio_adapter.h>
#include <dsm/dsm_pub.h>

#include "slimbus.h"
#include "dsp_utils.h"
#include "om_hook.h"

#define LOG_TAG "download_image"

#define HI64XX_RELOAD_RETRY_MAX 3
#define WAIT_DSP_POWERON_TIME 3
#define PARA_4BYTE_ALINED 0x3
#define TIME_STAMP_LEN 24
#define IRQ_REG_NUM 6

enum drv_hifi_image_sec_type_enum {
	DRV_HIFI_IMAGE_SEC_TYPE_CODE = 0, /* code section */
	DRV_HIFI_IMAGE_SEC_TYPE_DATA, /* data section */
	DRV_HIFI_IMAGE_SEC_TYPE_BSS, /* bss section */
	DRV_HIFI_IMAGE_SEC_TYPE_BUTT,
};

enum drv_hifi_image_sec_load_enum {
	DRV_HIFI_IMAGE_SEC_LOAD_STATIC = 0, /* before dsp reset  download one time */
	DRV_HIFI_IMAGE_SEC_LOAD_DYNAMIC, /* maybe need download dynamic */
	DRV_HIFI_IMAGE_SEC_UNLOAD, /* the section do not need download */
	DRV_HIFI_IMAGE_SEC_LOAD_BUTT,
};

struct drv_hifi_image_sec {
	unsigned short  sn; /* section serial number */
	unsigned char   type; /* section type :code, data, bss */
	unsigned char   load_attib; /* download attribute:static,dynmic,unload */
	unsigned int    src_offset; /* offset of down file, bytes */
	unsigned int    des_addr; /* des addr , bytes */
	unsigned int    size; /* section length, bytes */
};

struct drv_hifi_image_head {
	char time_stamp[TIME_STAMP_LEN]; /* image time stamp */
	unsigned int image_size; /* image size, bytes */
	unsigned int sections_num; /* section number */
	/* section head include section infomation */
	struct drv_hifi_image_sec sections[DSP_IMG_SEC_MAX_NUM];
};

struct hi64xx_dsp_img_dl_priv {
	struct hi64xx_dsp_config dsp_config;
	struct hi64xx_irq *p_irq;
	struct hi64xx_dsp_img_dl_config dl_config;
	dma_addr_t src_dma_addr;
	wait_queue_head_t dsp_pwron_wq;
	unsigned int dsp_pwron_done;
	unsigned int *src_addr_v;
};

static struct hi64xx_dsp_img_dl_priv *dl_data;

static int check_load_addr(const struct drv_hifi_image_sec *img_sec)
{
	if ((img_sec->des_addr >= dl_data->dsp_config.ocram_start_addr &&
		img_sec->des_addr < (dl_data->dsp_config.ocram_start_addr +
			dl_data->dsp_config.ocram_size)) ||
		(img_sec->des_addr >= dl_data->dsp_config.itcm_start_addr &&
		img_sec->des_addr < (dl_data->dsp_config.itcm_start_addr +
			dl_data->dsp_config.itcm_size)) ||
		(img_sec->des_addr >= dl_data->dsp_config.dtcm_start_addr &&
		img_sec->des_addr < (dl_data->dsp_config.dtcm_start_addr +
			dl_data->dsp_config.dtcm_size))) {
		AUDIO_LOGD("des_addr check ok");
		return 0;
	}

	AUDIO_LOGE("des addr error, addr:%pK",
		(void *)(uintptr_t)(img_sec->des_addr));

	return -EINVAL;
}

static int check_section_size(const struct drv_hifi_image_sec *img_sec)
{
	if (img_sec->size <= dl_data->dsp_config.ocram_size ||
		img_sec->size <= dl_data->dsp_config.itcm_size ||
		img_sec->size <= dl_data->dsp_config.dtcm_size) {
		AUDIO_LOGD("section size check ok");
		return 0;
	}

	AUDIO_LOGE("section size check error, size:%u", img_sec->size);

	return -EINVAL;
}

static int check_section_head(const struct drv_hifi_image_head *img_head,
	const struct drv_hifi_image_sec *img_sec)
{
	/*
	 * BSS section do not need check offset and size,
	 * beacuse BSS section only record address and length, the content is 0
	 */
	if (img_sec->type == DRV_HIFI_IMAGE_SEC_TYPE_BSS &&
		(img_sec->sn < img_head->sections_num) &&
		(img_sec->load_attib < (unsigned char)DRV_HIFI_IMAGE_SEC_LOAD_BUTT))
		return 0;

	/* check section number and section size  */
	if ((img_sec->sn >= img_head->sections_num) ||
		(img_sec->src_offset + img_sec->size > img_head->image_size) ||
		(img_sec->type >= (unsigned char)DRV_HIFI_IMAGE_SEC_TYPE_BUTT) ||
		(img_sec->load_attib >= (unsigned char)DRV_HIFI_IMAGE_SEC_LOAD_BUTT)) {
		AUDIO_LOGE("hifi: drv_hifi_check_sections error");
		return -EINVAL;
	}

	if (check_load_addr(img_sec) ||
		check_section_size(img_sec)) {
		AUDIO_LOGE("check load addr or section size error");
		return -EINVAL;
	}

	return 0;
}

static int check_img_head(const struct drv_hifi_image_head *head)
{
	unsigned int i;
	int ret = 0;

	if (head->sections_num > DSP_IMG_SEC_MAX_NUM) {
		AUDIO_LOGE("section num error, %d", head->sections_num);
		return -EINVAL;
	}

	for (i = 0; i < head->sections_num; i++) {
		/* check the sections */
		ret = check_section_head(head, &(head->sections[i]));
		if (ret != 0) {
			AUDIO_LOGI("hifi: invalid hifi section");
			return ret;
		}
	}

	return ret;
}

static void download_section(uint32_t *src, uint32_t des,
	uint32_t size, int type)
{
	switch (type) {
	case DRV_HIFI_IMAGE_SEC_TYPE_BSS:
		if (size & PARA_4BYTE_ALINED) {
			AUDIO_LOGE("img download is not 4 byte aligned");
			AUDIO_LOGE("img size:0x%x, type:%d", size, type);
			return;
		}
		hi64xx_memset(des, size);
		break;
	case DRV_HIFI_IMAGE_SEC_TYPE_CODE:
	case DRV_HIFI_IMAGE_SEC_TYPE_DATA:
		if (size & PARA_4BYTE_ALINED) {
			AUDIO_LOGE("img download is not 4 byte aligned");
			AUDIO_LOGE("img size:0x%x, type:%d", size, type);
			return;
		}
		hi64xx_memcpy(des, src, size);
		break;
	default:
		break;
	}
}

static void download_dsp_img(const struct firmware *fw,
	enum bustype_select bus_sel)
{
	const struct drv_hifi_image_head *head = NULL;
	unsigned int dest_addr;
	unsigned int *src_addr = NULL;
	unsigned int i = 0;
	int ret;

	AUDIO_LOGI("img dl++, size:[%zu] bus sel:0x%x", fw->size, bus_sel);

	head = (struct drv_hifi_image_head *)fw->data;
	if (head == NULL) {
		AUDIO_LOGE("img head is null");
		return;
	}

	ret = check_img_head(head);
	if (ret != 0) {
		AUDIO_LOGE("img head is invalid");
		return;
	}

	if (bus_sel == BUSTYPE_SELECT_SLIMBUS)
		slimbus_bus_configure(SLIMBUS_BUS_CONFIG_REGIMGDOWN);

	for (i = 0; i < head->sections_num; i++) {
		src_addr = (unsigned int *)((char *)head +
			head->sections[i].src_offset);
		dest_addr = head->sections[i].des_addr;
		AUDIO_LOGD("hifi: sections_num = %d,dest_addr = 0x%pK",
			head->sections_num,
			(void *)(uintptr_t)(head->sections[i].des_addr));
		AUDIO_LOGD("load_attib = %d, size = 0x%x",
			head->sections[i].load_attib,
			head->sections[i].size);
		AUDIO_LOGD("sn = %d, src_offset = 0x%x, type = %d",
			head->sections[i].sn,
			head->sections[i].src_offset,
			head->sections[i].type);
		AUDIO_LOGI("[0x%pK]->[0x%pK]", src_addr,
			(void *)(uintptr_t)dest_addr);
		download_section(src_addr, dest_addr, head->sections[i].size,
			head->sections[i].type);
	}

	if (bus_sel == BUSTYPE_SELECT_SLIMBUS)
		slimbus_bus_configure(SLIMBUS_BUS_CONFIG_NORMAL);

	AUDIO_LOGI("img dl--");
}

static int check_fw_param(const struct hi64xx_param_io_buf *param)
{
	const struct fw_download_stru *dsp_fw_download = NULL;

	if (param == NULL) {
		AUDIO_LOGE("input param is null");
		return -EINVAL;
	}

	if (!param->buf_in) {
		AUDIO_LOGE("input buf_in is null");
		return -EINVAL;
	}

	if (param->buf_size_in != sizeof(struct fw_download_stru)) {
		AUDIO_LOGE("input size:%u invalid", param->buf_size_in);
		return -EINVAL;
	}

	dsp_fw_download = (struct fw_download_stru *)(param->buf_in);
	if (dsp_fw_download->chw_name[CODECDSP_FW_NAME_MAX_LENGTH - 1] != '\0') {
		AUDIO_LOGE("firmware name error");
		return -EINVAL;
	}

	return 0;
}

static void clear_log_region(void)
{
	unsigned int codec_log_addr;
	unsigned int codec_log_size;

	codec_log_addr = dl_data->dsp_config.dump_log_addr;
	if (codec_log_addr == 0) {
		AUDIO_LOGE("get codec log dump addr failed");
		return;
	}

	codec_log_size = dl_data->dsp_config.dump_log_size;
	if (codec_log_size == 0) {
		AUDIO_LOGE("get codec log dump size fialed");
		return;
	}

	hi64xx_memset(codec_log_addr, (size_t)codec_log_size);
}

static bool retry_max_detect(int reload_retry_count)
{
	if (reload_retry_count >= HI64XX_RELOAD_RETRY_MAX) {
		AUDIO_LOGE("codec hifi reset, reload retry count reaches the upper limit");
		return true;
	}

	return false;
}

static int get_dsp_fw(const struct hi64xx_param_io_buf *param,
	const struct firmware **fw)
{
	int ret;
	char *fw_name = NULL;
	struct fw_download_stru *dsp_fw_download = NULL;

	/* request dsp firmware */
	dsp_fw_download = (struct fw_download_stru *)(param->buf_in);
	fw_name = dsp_fw_download->chw_name;

	ret = request_firmware(fw, fw_name, dl_data->p_irq->dev);
	if (ret != 0) {
		AUDIO_LOGE("Failed to request dsp image(%s): %d", fw_name, ret);
		return ret;
	}

	if (*fw == NULL) {
		AUDIO_LOGE("fw is null");
		return -ENOENT;
	}

	return 0;
}

static void notify_dsp_poweron(void)
{
	if (dl_data->dsp_config.dsp_ops.config_usb_low_power)
		dl_data->dsp_config.dsp_ops.config_usb_low_power();

	if (dl_data->dsp_config.msg_state_addr != 0)
		hi64xx_dsp_write_reg(dl_data->dsp_config.msg_state_addr,
			HI64XX_MSG_STATE_CLEAR);

	/* notify dsp pwr on */
	hi64xx_dsp_write_reg(dl_data->dsp_config.cmd0_addr, HIFI_POWER_ON);
	dl_data->dsp_pwron_done = HIFI_STATE_UNINIT;

	/* irq clr,unmask */
	if (hi64xx_dsp_read_reg(HI64XX_REG_IRQ_2)  & 0x1)
		hi64xx_dsp_write_reg(HI64XX_REG_IRQ_2, 0x1);

	hi64xx_dsp_reg_clr_bit(HI64XX_REG_IRQM_2, 0x0);

	if (dl_data->dsp_config.dsp_ops.runstall)
		dl_data->dsp_config.dsp_ops.runstall(true);
}

static void show_dsp_status(void)
{
	unsigned int read_res[IRQ_REG_NUM];

	read_res[0] = hi64xx_dsp_read_reg(HI64XX_REG_IRQ_0);
	read_res[1] = hi64xx_dsp_read_reg(HI64XX_REG_IRQ_1);
	read_res[2] = hi64xx_dsp_read_reg(HI64XX_REG_IRQ_2);
	read_res[3] = hi64xx_dsp_read_reg(HI64XX_REG_IRQM_0);
	read_res[4] = hi64xx_dsp_read_reg(HI64XX_REG_IRQM_1);
	read_res[5] = hi64xx_dsp_read_reg(HI64XX_REG_IRQM_2);
	AUDIO_LOGI("14:%#x, 15:%#x, 16:%#x, 17:%#x, 18:%#x, 19:%#x",
		read_res[0], read_res[1], read_res[2], read_res[3], read_res[4],
		read_res[5]);

	if (dl_data->dsp_config.msg_state_addr != 0)
		AUDIO_LOGI("dsp msg process state:0x%x",
			hi64xx_dsp_read_reg(dl_data->dsp_config.msg_state_addr));
}

static void wait_for_dsp_poweron(void)
{
	long ret;
	static int retry_count;

	/* wait 3s for dsp power on */
	ret = wait_event_interruptible_timeout(dl_data->dsp_pwron_wq,
		(dl_data->dsp_pwron_done == HIFI_STATE_INIT),
		(WAIT_DSP_POWERON_TIME * HZ));
	if (ret <= 0) {
		AUDIO_LOGE("wait for dsp pwron error, ret:%ld", ret);

		show_dsp_status();
		if (hi64xx_error_detect() || retry_max_detect(retry_count))
			rdr_system_error(RDR_AUDIO_CODEC_ERR_MODID, 0, 0);

		if (!(hi64xx_get_sync_write_state()) &&
			(retry_count <= HI64XX_RELOAD_RETRY_MAX)) {
			AUDIO_LOGE("do reset codecdsp,retry %d", retry_count);
			if (ret == 0) {
				AUDIO_LOGE("wait for dsp pwron timeout, dump log and reset dsp");
				hi64xx_set_sync_write_state(true);
				retry_count++;
				hi64xx_wtdog_process();
			} else {
				AUDIO_LOGE("wait event is interrupted, just reset dsp");
				hi64xx_wtdog_send_event();
			}
		}
	} else {
		retry_count = 0;
	}
}

static void set_dsp_pause(void)
{
	if (dl_data->dsp_config.dsp_ops.ram2axi)
		dl_data->dsp_config.dsp_ops.ram2axi(false);
	if (dl_data->dsp_config.dsp_ops.clk_enable)
		dl_data->dsp_config.dsp_ops.clk_enable(false);
	if (dl_data->dsp_config.dsp_ops.dsp_power_ctrl)
		dl_data->dsp_config.dsp_ops.dsp_power_ctrl(false);
	hi64xx_dsp_set_pll(false);
}

int hi64xx_fw_download(const struct hi64xx_param_io_buf *param)
{
	int ret;
	int i;
	const struct firmware *fw = NULL;

	IN_FUNCTION;

	if (dl_data == NULL)
		return -EINVAL;

	if (check_fw_param(param))
		return -EINVAL;

	ret = get_dsp_fw(param, &fw);
	if (ret)
		return ret;

	/*
	 * release all requeseted PLL first beacuse codec dsp maybe request PLL
	 * but didn't release when exception
	 */
	hi64xx_release_requested_pll();
	hi64xx_stop_hook_route();
	hi64xx_dsp_set_pll(true);

	for (i = 0; i < HI64XX_DSP_IF_PORT_BUTT; i++) {
		if (dl_data->dsp_config.dsp_ops.set_if_bypass)
			dl_data->dsp_config.dsp_ops.set_if_bypass(i, true);
	}

	hi64xx_reset_dsp();
	hi64xx_set_wtdog_state(false);
	hi64xx_set_sync_write_state(false);

	if (dl_data->dsp_config.dsp_ops.clk_enable)
		dl_data->dsp_config.dsp_ops.clk_enable(true);
	if (dl_data->dsp_config.dsp_ops.ram2axi)
		dl_data->dsp_config.dsp_ops.ram2axi(true);

	clear_log_region();
	download_dsp_img(fw, dl_data->dsp_config.bus_sel);
	release_firmware(fw);
	notify_dsp_poweron();
	wait_for_dsp_poweron();
	usleep_range(1000, 1100);
	set_dsp_pause();

	OUT_FUNCTION;

	return ret;
}

void hi64xx_dsp_poweron_irq_handler(void)
{
	if (dl_data == NULL)
		return;

	AUDIO_LOGI("codec hifi pwron done cnf");

	if (dl_data->dsp_config.msg_state_addr != 0)
		hi64xx_dsp_write_reg(dl_data->dsp_config.msg_state_addr,
			HI64XX_AP_RECEIVE_PWRON_CNF);

	hi64xx_clr_cmd_status_bit(HI64XX_DSP_POWERON_BIT);
	dl_data->dsp_pwron_done = HIFI_STATE_INIT;

	wake_up_interruptible_all(&dl_data->dsp_pwron_wq);
#ifdef CONFIG_HISI_HIFI_BB
	rdr_audio_clear_reboot_times();
#endif
}

unsigned int hi64xx_get_dsp_poweron_state(void)
{
	if (dl_data == NULL)
		return -EINVAL;

	return dl_data->dsp_pwron_done;
}

int hi64xx_dsp_img_dl_init(struct hi64xx_irq *irqmgr,
	const struct hi64xx_dsp_img_dl_config *dl_config,
	const struct hi64xx_dsp_config *config)
{
	int image_down_size = IMAGE_DOWN_MEM_SIZE;

	dl_data = kzalloc(sizeof(*dl_data), GFP_KERNEL);
	if (dl_data == NULL) {
		AUDIO_LOGE("dl data kzalloc error");
		return -ENOMEM;
	}

	memcpy(&dl_data->dl_config, dl_config, sizeof(*dl_config));
	memcpy(&dl_data->dsp_config, config, sizeof(*config));

	dl_data->p_irq = irqmgr;
	dl_data->src_addr_v = dma_alloc_coherent(dl_data->p_irq->dev,
		image_down_size, &(dl_data->src_dma_addr), GFP_KERNEL);
	if (dl_data->src_addr_v == NULL) {
		kfree(dl_data);
		dl_data = NULL;
		AUDIO_LOGW("dma alloc failed");
		return -ENOMEM;
	}

	dl_data->dsp_pwron_done = HIFI_STATE_UNINIT;

	init_waitqueue_head(&dl_data->dsp_pwron_wq);

	return 0;
}

void hi64xx_dsp_img_dl_deinit(void)
{
	int image_down_size = IMAGE_DOWN_MEM_SIZE;

	if (dl_data == NULL)
		return;

	dma_free_coherent(dl_data->p_irq->dev, image_down_size,
		dl_data->src_addr_v, dl_data->src_dma_addr);

	kfree(dl_data);

	dl_data = NULL;
}

