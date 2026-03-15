 /* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_display_effect.h"
#include "hisi_fb.h"
#include <linux/fb.h>
#include "global_ddr_map.h"
#include <linux/hisi/hw_cmdline_parse.h>

//lint -e747, -e838, -e774

#define COUNT_LIMIT_TO_PRINT_DELAY			(200)

typedef struct time_interval {
	long start; 				// microsecond
	long stop;
} time_interval_t;

typedef struct delay_record {
	const char *name;
	long max;
	long min;
	long sum;
	int count;
} delay_record_t;

static int hisifb_ce_do_contrast(struct hisi_fb_data_type *hisifd);
static void hisifb_ce_service_init(void);
static void hisifb_ce_service_deinit(void);

#define DEBUG_EFFECT_LOG					HISI_FB_ERR

#define _EFFECT_DEBUG_LOG(message, ...) \
	do { printk("%s: "message, __func__, ## __VA_ARGS__); } while (0)
#define EFFECT_DEBUG_LOG(feature, message, ...) \
	do { if (g_debug_effect & feature) \
		printk("%s: "message, __func__, ## __VA_ARGS__); } while (0)

#define DEFINE_DELAY_RECORD(var, name)		static delay_record_t var = {name, 0, 0xFFFFFFFF, 0, 0}
																		\
#define EFFECT_GRADUAL_REFRESH_FRAMES		(30)

static bool g_is_effect_init = false;
static bool g_is_ce_service_init = false;
static struct mutex g_ce_service_lock;
static ce_service_t g_hiace_service;

static bool g_is_effect_lock_init = false;
extern struct mutex g_rgbw_lock;
static spinlock_t g_gmp_effect_lock;
static spinlock_t g_igm_effect_lock;
static spinlock_t g_xcc_effect_lock;
static spinlock_t g_gamma_effect_lock;

#define THRESHOLD_HBM_BACKLIGHT			(741)

static time_interval_t interval_wait_hist = {0};
static time_interval_t interval_wait_lut = {0};
static time_interval_t interval_algorithm = {0};
DEFINE_DELAY_RECORD(delay_wait_hist, "event hist waiting");
DEFINE_DELAY_RECORD(delay_wait_lut, "event lut waiting");
DEFINE_DELAY_RECORD(delay_algorithm, "algorithm processing");

uint32_t g_enable_effect = ENABLE_EFFECT_HIACE | ENABLE_EFFECT_BL;
uint32_t g_debug_effect = 0;



#define ce_service_wait_event(wq, condition)		/*lint -save -e* */						\
({																								\
	long _wait_ret = 0; 																		\
	do {																						\
		_wait_ret = wait_event_interruptible_timeout(wq, condition, msecs_to_jiffies(100000));	\
	} while(!_wait_ret);																		\
	_wait_ret;																					\
})		/*lint -restore */


static inline long get_timestamp_in_us(void)
{
	struct timespec ts;
	long timestamp;
	ktime_get_ts(&ts);
	timestamp = ts.tv_sec * USEC_PER_SEC + ts.tv_nsec / NSEC_PER_USEC;
	return timestamp;
}

static inline void count_delay(delay_record_t *record, long delay)
{
	if (NULL == record) {
		return;
	}
	record->count++;
	record->sum += delay;
	if (delay > record->max) record->max = delay;
	if (delay < record->min) record->min = delay;
	if (!(record->count % COUNT_LIMIT_TO_PRINT_DELAY)) {
		DEBUG_EFFECT_LOG("[effect] Delay(us/%4d) | average:%5ld | min:%5ld | max:%8ld | %s\n", record->count, record->sum / record->count, record->min, record->max, record->name);
		record->count = 0;
		record->sum = 0;
		record->max = 0;
		record->min = 0xFFFFFFFF;
	}
}

static inline uint32_t get_fixed_point_offset(uint32_t half_block_size)
{
	uint32_t num = 2;
	uint32_t len = 2;
	while (len < half_block_size) {
		num++;
		len <<= 1;
	}
	return num;
}


static inline void enable_blc(struct hisi_fb_data_type *hisifd, bool enable)
{
	dss_display_effect_bl_enable_t *bl_enable_ctrl = &(hisifd->bl_enable_ctrl);
	int bl_enable = enable ? 1 : 0;

	mutex_lock(&(bl_enable_ctrl->ctrl_lock));
	if (bl_enable != bl_enable_ctrl->ctrl_bl_enable) {
		bl_enable_ctrl->ctrl_bl_enable = bl_enable;
	}
	mutex_unlock(&(bl_enable_ctrl->ctrl_lock));
	EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] bl_enable change to %d\n", bl_enable_ctrl->ctrl_bl_enable);
}


static int hisifb_ce_do_contrast(struct hisi_fb_data_type *hisifd)
{
	ce_service_t *service = &g_hiace_service;
	int ret = 0;
	long wait_ret = 0;
	long timeout = msecs_to_jiffies(100);

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	if (g_is_ce_service_init) {
		service->new_hist = true;
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_wait_hist.start = get_timestamp_in_us();
		}

		if (g_is_effect_init) {
			wake_up_interruptible(&service->wq_hist);
			wait_ret = wait_event_interruptible_timeout(service->wq_lut, service->new_lut || service->stop, timeout);
			service->stop = false;
		} else {
			HISI_FB_ERR("[effect] wq_lut uninit\n");
			return -EINVAL;
		}

		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_wait_lut.stop = get_timestamp_in_us();
			count_delay(&delay_wait_lut, interval_wait_lut.stop - interval_wait_lut.start);
		}
	}

	if (service->new_lut) {
		service->new_lut = false;
		ret = hisifd->hiace_info.algorithm_result;
	} else {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] wait_event_interruptible_timeout() return %ld, -ERESTARTSYS:%d\n", wait_ret, -ERESTARTSYS);
		ret = -3;
	}

	return ret;
}

bool check_underflow_clear_ack(struct hisi_fb_data_type *hisifd)
{
	char __iomem *mctl_base = NULL;
	uint32_t mctl_status = 0;

	if(NULL == hisifd){
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return false;
	}

	mctl_base = hisifd->dss_module.mctl_base[DSS_MCTL0];
	if (mctl_base) {
		mctl_status = inp32(mctl_base + MCTL_CTL_STATUS);
		if ((mctl_status & 0x10) == 0x10) {
			HISI_FB_ERR("UNDERFLOW ACK TIMEOUT!\n");
			return false;
		}
	}
	return true;
}

void hisi_effect_init(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	dss_ce_info_t *ce_info = NULL;
	hiace_alg_parameter_t *param = NULL;

	if (!g_is_effect_lock_init) {
		spin_lock_init(&g_gmp_effect_lock);
		spin_lock_init(&g_igm_effect_lock);
		spin_lock_init(&g_xcc_effect_lock);
		spin_lock_init(&g_gamma_effect_lock);
		mutex_init(&g_rgbw_lock);
		g_is_effect_lock_init = true;
	}

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] HIACE is not supported!\n");
		}
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ce_info = &hisifd->hiace_info;
		param = &pinfo->hiace_param;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return;
	}

	if (!g_is_effect_init) {
		mutex_init(&g_ce_service_lock);
		mutex_init(&(hisifd->al_ctrl.ctrl_lock));
		mutex_init(&(hisifd->ce_ctrl.ctrl_lock));
		mutex_init(&(hisifd->bl_ctrl.ctrl_lock));
		mutex_init(&(hisifd->bl_enable_ctrl.ctrl_lock));
		mutex_init(&(hisifd->metadata_ctrl.ctrl_lock));
		hisifd->bl_enable_ctrl.ctrl_bl_enable = 1;

		memset(ce_info, 0, sizeof(dss_ce_info_t));
		ce_info->algorithm_result = 1;
		mutex_init(&(ce_info->hist_lock));
		mutex_init(&(ce_info->lut_lock));

		param->iWidth = (int)pinfo->xres;
		param->iHeight = (int)pinfo->yres;
		param->iMode = 0;
		param->bitWidth = 10;
		param->iMinBackLight = (int)hisifd->panel_info.bl_min;
		param->iMaxBackLight = (int)hisifd->panel_info.bl_max;
		param->iAmbientLight = -1;

		memset(&g_hiace_service, 0, sizeof(g_hiace_service));
		init_waitqueue_head(&g_hiace_service.wq_hist);
		init_waitqueue_head(&g_hiace_service.wq_lut);

		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] width:%d, height:%d, minbl:%d, maxbl:%d\n", param->iWidth, param->iHeight, param->iMinBackLight, param->iMaxBackLight);
		}

		g_is_effect_init = true;
		HISI_FB_INFO("[effect] fb%d, hisi effect init here!", hisifd->index);
	} else {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] bypass\n");
		}
		HISI_FB_INFO("[effect] fb%d, hisi effect has been inited!", hisifd->index);
	}
}

void hisi_effect_deinit(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	dss_ce_info_t *ce_info = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] HIACE is not supported!\n");
		}
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ce_info = &hisifd->hiace_info;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return;
	}

	if (g_is_effect_lock_init) {
		g_is_effect_lock_init = false;
		mutex_destroy(&g_rgbw_lock);
	}

	down(&hisifd->hiace_hist_lock_sem);/*avoid  using  mutex_lock() but hist_lock was destoried by mutex_destory in  hisi_effect_deinit*/
	if (g_is_effect_init) {
		g_is_effect_init = false;

		mutex_destroy(&(ce_info->hist_lock));
		mutex_destroy(&(ce_info->lut_lock));

		mutex_destroy(&(hisifd->al_ctrl.ctrl_lock));
		mutex_destroy(&(hisifd->ce_ctrl.ctrl_lock));
		mutex_destroy(&(hisifd->bl_ctrl.ctrl_lock));
		mutex_destroy(&(hisifd->bl_enable_ctrl.ctrl_lock));
		mutex_destroy(&(hisifd->metadata_ctrl.ctrl_lock));

		mutex_destroy(&g_ce_service_lock);
	} else {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] bypass\n");
		}
	}
	up(&hisifd->hiace_hist_lock_sem);
}

static void hisifb_ce_service_init(void)
{
	mutex_lock(&g_ce_service_lock);
	if (!g_is_ce_service_init) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] step in\n");

		g_hiace_service.is_ready = true;
		g_hiace_service.stop = false;

		g_is_ce_service_init = true;
	}
	mutex_unlock(&g_ce_service_lock);
}

static void hisifb_ce_service_deinit(void)
{
	mutex_lock(&g_ce_service_lock);
	if (g_is_ce_service_init) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] step in\n");

		g_is_ce_service_init = false;

		g_hiace_service.is_ready = false;
		g_hiace_service.stop = true;
		wake_up_interruptible(&g_hiace_service.wq_hist);
		wake_up_interruptible(&g_hiace_service.wq_lut);
	}
	mutex_unlock(&g_ce_service_lock);
}

static inline void enable_hiace(struct hisi_fb_data_type *hisifd, bool enable)
{
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (hisifd->hiace_info.hiace_enable != enable) { //lint !e731
			char __iomem *hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;

			hisifb_activate_vsync(hisifd);
			if (enable) {
				// enable hiace
				set_reg(hiace_base + HIACE_BYPASS_ACE, 0x0, 1, 0);
			} else {
				// disable hiace
				set_reg(hiace_base + HIACE_BYPASS_ACE, 0x1, 1, 0);
			}
			hisifb_deactivate_vsync(hisifd);

			hisifd->hiace_info.hiace_enable = enable;
			EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] hiace:%d\n", (int)enable);
		}
	} else {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] fb%d, panel power off!\n", hisifd->index);
	}
	up(&hisifd->blank_sem);
}

int hisifb_ce_service_blank(int blank_mode, struct fb_info *info)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (pinfo->hiace_support) {
			EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] blank_mode is %d\n", blank_mode);
			if (blank_mode == FB_BLANK_UNBLANK) {
				hisifb_ce_service_init();
				enable_blc(hisifd, true);
				if (hisifb_display_effect_is_need_ace(hisifd)) {
					enable_hiace(hisifd, true);
				}
			} else {
				if (hisifb_display_effect_is_need_ace(hisifd)) {
					g_hiace_service.use_last_value = true;
				}
				hisifd->hiace_info.gradual_frames = 0;
				hisifd->hiace_info.hiace_enable = false;
				hisifd->hiace_info.to_stop_hdr = false;
				hisifd->hiace_info.to_stop_sre = false;
				g_hiace_service.blc_used = false;
				// Since java has no destruct function and Gallery will refresh metadata when power on, always close HDR for Gallery when power off.
				if (hisifd->ce_ctrl.ctrl_ce_mode == CE_MODE_IMAGE) {
					hisifd->ce_ctrl.ctrl_ce_mode = CE_MODE_DISABLE;
				}
				enable_blc(hisifd, false);
				hisifb_ce_service_deinit();
			}
		}
	}
	return 0;
}

int hisifb_ce_service_get_support(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	unsigned int support = 0;
	int ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support) {
		support = 1;
	}
	EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] support:%d\n", support);

	ret = (int)copy_to_user(argp, &support, sizeof(support));
	if (ret) {
		HISI_FB_ERR("[effect] copy_to_user failed! ret=%d.\n", ret);
		return ret;
	}

	return ret;
}

int hisifb_ce_service_get_limit(struct fb_info *info, void __user *argp)
{
	int limit = 0;
	int ret = 0;

	if (NULL == argp) {
		HISI_FB_ERR("argp is NULL\n");
		return -EINVAL;
	}

	ret = (int)copy_to_user(argp, &limit, sizeof(limit));
	if (ret) {
		HISI_FB_ERR("copy_to_user failed! ret=%d.\n", ret);
		return ret;
	}

	return ret;
}

int hisifb_ce_service_get_hiace_param(struct fb_info *info, void __user *argp){
	(void)info, (void)argp;
	return 0;
}

int hisifb_ce_service_get_param(struct fb_info *info, void __user *argp)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;
	dss_display_effect_al_t *al_ctrl = NULL;
	dss_display_effect_sre_t *sre_ctrl = NULL;
	dss_display_effect_metadata_t *metadata_ctrl = NULL;
	dss_ce_info_t *ce_info = NULL;
	int mode = 0;
	struct timespec ts;

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (!pinfo->hiace_support) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] Don't support HIACE\n");
		return -EINVAL;
	}

	ce_ctrl = &(hisifd->ce_ctrl);
	al_ctrl = &(hisifd->al_ctrl);
	sre_ctrl = &(hisifd->sre_ctrl);
	metadata_ctrl = &(hisifd->metadata_ctrl);
	ce_info = &(hisifd->hiace_info);
	mode = ce_ctrl->ctrl_ce_mode;
	if (mode != CE_MODE_DISABLE) {
		pinfo->hiace_param.iDoLCE = 1;
		pinfo->hiace_param.iDoAPLC = (mode == CE_MODE_VIDEO && hisifd->bl_level > 0 && al_ctrl->ctrl_al_value >= 0) ? 1 : 0;
	} else {
		if (ce_info->gradual_frames > 0) {
			pinfo->hiace_param.iDoLCE = -1;
			if (pinfo->hiace_param.iDoAPLC == 1) {
				pinfo->hiace_param.iDoAPLC = -1;
			}
		} else {
			pinfo->hiace_param.iDoLCE = 0;
			pinfo->hiace_param.iDoAPLC = 0;
			ce_info->to_stop_hdr = false;
		}
	}
	if (hisifd->bl_level == 0) {
		pinfo->hiace_param.iDoAPLC = 0;
	}
	pinfo->hiace_param.iDoSRE = sre_ctrl->ctrl_sre_enable;
	if (pinfo->hiace_param.iDoSRE == 0) {
		ce_info->to_stop_sre = false;
	}
	pinfo->hiace_param.iLevel = (mode == CE_MODE_VIDEO) ? 0 : 1;
	pinfo->hiace_param.iAmbientLight = (sre_ctrl->ctrl_sre_enable == 1) ? sre_ctrl->ctrl_sre_al : al_ctrl->ctrl_al_value;
	pinfo->hiace_param.iBackLight = (int)hisifd->bl_level;
	pinfo->hiace_param.iLaSensorSREOnTH = sre_ctrl->ctrl_sre_al_threshold;
	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] BLC:%d, bl:%d, panel_on:%d\n", pinfo->hiace_param.iDoAPLC, pinfo->hiace_param.iBackLight, hisifd->panel_power_on);
	ktime_get_ts(&ts);
	pinfo->hiace_param.lTimestamp = ts.tv_sec * MSEC_PER_SEC + ts.tv_nsec / NSEC_PER_MSEC;
	if (metadata_ctrl->count <= META_DATA_SIZE) {
		memcpy(pinfo->hiace_param.Classifieresult, metadata_ctrl->metadata, (size_t)metadata_ctrl->count);//lint !e571
		pinfo->hiace_param.iResultLen = metadata_ctrl->count;
	}

	ret = (int)copy_to_user(argp, &pinfo->hiace_param, sizeof(pinfo->hiace_param));
	if (ret) {
		HISI_FB_ERR("[effect] copy_to_user(hiace_param) failed! ret=%d.\n", ret);
		return ret;
	}

	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] iLevel:%d, iAmbientLight:%d, iBackLight:%d, lTimestamp:%ld(ms)\n",
					 pinfo->hiace_param.iLevel, pinfo->hiace_param.iAmbientLight, pinfo->hiace_param.iBackLight, pinfo->hiace_param.lTimestamp);

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_algorithm.start = get_timestamp_in_us();
	}

	return ret;
}
int hisifb_ce_service_get_hist(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	ce_service_t *service = &g_hiace_service;
	int ret = 0;
	long wait_ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return -EINVAL;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		HISI_FB_ERR("fb%d is not supported!\n", hisifd->index);
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (!pinfo->hiace_support) {
		HISI_FB_ERR("[effect] Don't support HIACE\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	HISI_FB_DEBUG("[effect] wait hist\n");

	if (g_is_effect_init) {
		unlock_fb_info(info);
		wait_ret = ce_service_wait_event(service->wq_hist, service->new_hist || service->stop);
		lock_fb_info(info);
		service->stop = false;
	} else {
		HISI_FB_ERR("[effect]wq_hist uninit\n");
		return -EINVAL;
	}

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_wait_hist.stop = get_timestamp_in_us();
		count_delay(&delay_wait_hist, interval_wait_hist.stop - interval_wait_hist.start);
	}

	down(&hisifd->hiace_hist_lock_sem);/*avoid  using  mutex_lock() but hist_lock was destoried by mutex_destory in  hisi_effect_deinit*/
	if (service->new_hist) {
		time_interval_t interval_copy_hist = {0};
		DEFINE_DELAY_RECORD(delay_copy_hist, "hist copy");

		service->new_hist = false;

		if(!g_is_effect_init){
			HISI_FB_ERR("[effect] wq_hist uninit here\n");
			up(&hisifd->hiace_hist_lock_sem);
			return -EINVAL;
		}
		mutex_lock(&hisifd->hiace_info.hist_lock);
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_copy_hist.start = get_timestamp_in_us();
		}
		ret = (int)copy_to_user(argp, hisifd->hiace_info.histogram, sizeof(hisifd->hiace_info.histogram));
		if (ret) {
			HISI_FB_ERR("[effect] copy_to_user failed(param)! ret=%d.\n", ret);
			ret = -1;
		}
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_copy_hist.stop = get_timestamp_in_us();
			count_delay(&delay_copy_hist, interval_copy_hist.stop - interval_copy_hist.start);
		}
		mutex_unlock(&hisifd->hiace_info.hist_lock);
	} else {
		if (hisifd->panel_power_on) {
			EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] wait_event_interruptible_timeout() return %ld, -ERESTARTSYS:%d\n", wait_ret, -ERESTARTSYS);
			ret = 3;
		} else {
			EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] Panel off! wait_event_interruptible_timeout() return %ld, -ERESTARTSYS:%d\n", wait_ret, -ERESTARTSYS);
			ret = 1;
		}
	}
	up(&hisifd->hiace_hist_lock_sem);

	return ret;
}

int hisifb_ce_service_set_lut(struct fb_info *info, const void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	dss_display_effect_bl_t *bl_ctrl = NULL;
	hiace_interface_set_t hiace_set_interface;
	ce_service_t *service = &g_hiace_service;
	int ret = 0;
	time_interval_t interval_copy_lut = {0};
	DEFINE_DELAY_RECORD(delay_copy_lut, "lut copy");

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_algorithm.stop = get_timestamp_in_us();
		count_delay(&delay_algorithm, interval_algorithm.stop - interval_algorithm.start);
	}

	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (!pinfo->hiace_support) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] Don't support HIACE\n");
		return -EINVAL;
	}

	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] step in\n");

	bl_ctrl = &(hisifd->bl_ctrl);

	ret = (int)copy_from_user(&hiace_set_interface, argp, sizeof(hiace_interface_set_t));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		return -2;
	}

	hisifd->hiace_info.thminv = hiace_set_interface.thminv;

	mutex_lock(&hisifd->hiace_info.lut_lock);
	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_copy_lut.start = get_timestamp_in_us();
	}
	ret = (int)copy_from_user(hisifd->hiace_info.lut_table, hiace_set_interface.lut, sizeof(hisifd->hiace_info.lut_table));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(lut_table) failed! ret=%d.\n", ret);
		ret = -2;
	}
	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_copy_lut.stop = get_timestamp_in_us();
		count_delay(&delay_copy_lut, interval_copy_lut.stop - interval_copy_lut.start);
	}
	mutex_unlock(&hisifd->hiace_info.lut_lock);
	hisifd->hiace_info.algorithm_result = 0;

	service->new_lut = true;
	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_wait_lut.start = get_timestamp_in_us();
	}
	wake_up_interruptible(&service->wq_lut);

	return ret;
}

ssize_t hisifb_display_effect_al_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_al_t *al_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	al_ctrl = &(hisifd->al_ctrl);

	return snprintf(buf, PAGE_SIZE, "%d\n", al_ctrl->ctrl_al_value);
}

ssize_t hisifb_display_effect_al_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	(void)info, (void)buf;

	return (ssize_t)count;
}

ssize_t hisifb_display_effect_ce_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	ce_ctrl = &(hisifd->ce_ctrl);

	return snprintf(buf, PAGE_SIZE, "%d\n", ce_ctrl->ctrl_ce_mode);
}

ssize_t hisifb_display_effect_ce_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	(void)info, (void)buf;
	return (ssize_t)count;
}

ssize_t hisifb_display_effect_bl_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_bl_t *bl_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	bl_ctrl = &(hisifd->bl_ctrl);

	return snprintf(buf, PAGE_SIZE, "%d\n", bl_ctrl->ctrl_bl_delta);
}

ssize_t hisifb_display_effect_bl_enable_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_bl_enable_t *bl_enable_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	bl_enable_ctrl = &(hisifd->bl_enable_ctrl);

	return snprintf(buf, PAGE_SIZE, "%d\n", bl_enable_ctrl->ctrl_bl_enable);
}

ssize_t hisifb_display_effect_bl_enable_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	(void)info, (void)buf;

	return (ssize_t)count;
}

ssize_t hisifb_display_effect_sre_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_sre_t *sre_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	sre_ctrl = &(hisifd->sre_ctrl);

	return snprintf(buf, PAGE_SIZE, "sre_enable:%d, sre_al:%d\n", sre_ctrl->ctrl_sre_enable, sre_ctrl->ctrl_sre_al);
}
/*lint -e438, -e550, -e715*/
ssize_t hisifb_display_effect_sre_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_sre_t *sre_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	sre_ctrl = &(hisifd->sre_ctrl);

	return (ssize_t)count;
}

ssize_t hisifb_display_effect_metadata_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == info) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	return 0;
}

ssize_t hisifb_display_effect_metadata_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	(void)info, (void)buf;

	return (ssize_t)count;
}

void hisifb_display_effect_func_switch(struct hisi_fb_data_type *hisifd, const char *command)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}
	if (NULL == command) {
		HISI_FB_ERR("[effect] command is NULL\n");
		return;
	}

	if (!strncmp("hiace:", command, strlen("hiace:"))) {
		if('0' == command[strlen("hiace:")]) {
			hisifd->panel_info.hiace_support = 0;
			HISI_FB_INFO("[effect] hiace disable\n");
		} else {
			hisifd->panel_info.hiace_support = 1;
			HISI_FB_INFO("[effect] hiace enable\n");
		}
	}
	if (!strncmp("effect_enable:", command, strlen("effect_enable:"))) {
		g_enable_effect = (int)simple_strtoul(&command[strlen("effect_enable:")], NULL, 0);
		HISI_FB_INFO("[effect] effect_enable changed to %d\n", g_enable_effect);
	}
	if (!strncmp("effect_debug:", command, strlen("effect_debug:"))) {
		g_debug_effect = (int)simple_strtoul(&command[strlen("effect_debug:")], NULL, 0);
		HISI_FB_INFO("[effect] effect_debug changed to %d\n", g_debug_effect);
	}
}

bool hisifb_display_effect_is_need_ace(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return false;
	}

	return (g_enable_effect & ENABLE_EFFECT_HIACE) && (hisifd->ce_ctrl.ctrl_ce_mode > 0 || hisifd->sre_ctrl.ctrl_sre_enable == 1 || hisifd->hiace_info.to_stop_hdr || hisifd->hiace_info.to_stop_sre);
}

bool hisifb_display_effect_is_need_blc(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
	return false;
	}
	return hisifd->de_info.blc_enable;
}

static void handle_first_deltabl(struct hisi_fb_data_type *hisifd, int backlight_in)
{
	struct hisi_panel_info* pinfo = NULL;
	int bl_min = (int)hisifd->panel_info.bl_min;
	int bl_max = (int)hisifd->panel_info.bl_max;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if(NULL == pinfo) {
		HISI_FB_ERR("pinfo is NULL!\n");
		return;
	}

	if (pinfo->hbm_support != 1) {
		return;
	}

	if (hisifd->bl_level == 0) {
		hisifd->de_info.blc_delta = -1;
		return;
	}

	if (hisifd->de_info.blc_delta == -1) {
		if ((backlight_in >= bl_min) && (backlight_in < THRESHOLD_HBM_BACKLIGHT)) {
			hisifd->de_info.blc_delta = bl_min + (backlight_in - bl_min)*(bl_max - bl_min)/(THRESHOLD_HBM_BACKLIGHT - 1 - bl_min ) - backlight_in;
		} else if ((backlight_in >= THRESHOLD_HBM_BACKLIGHT) && (backlight_in <= bl_max)) {
			hisifd->de_info.blc_delta = bl_max - backlight_in;
		}
		HISI_FB_DEBUG("[effect] first screen on ! delta: -1 -> %d bl: %d (%d)\n",hisifd->de_info.blc_delta,backlight_in,hisifd->bl_level);
	}
}

bool hisifb_display_effect_check_bl_value(int curr, int last) {
	return false;
}

bool hisifb_display_effect_check_bl_delta(int curr, int last) {
	return false;
}

bool hisifb_display_effect_fine_tune_backlight(struct hisi_fb_data_type *hisifd, int backlight_in, int *backlight_out)
{
	bool changed = false;
	struct hisi_panel_info* pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return false;
	}

	pinfo = &(hisifd->panel_info);
	if(NULL == pinfo) {
		HISI_FB_ERR("pinfo is NULL!\n");
		return false;
	}

	if (NULL == backlight_out) {
		HISI_FB_ERR("[effect] backlight_out is NULL\n");
		return false;
	}

	handle_first_deltabl(hisifd,backlight_in);


	if ((pinfo->hbm_support == 1) && (backlight_in <= (int)hisifd->panel_info.bl_min)) {
		HISI_FB_DEBUG("[effect] don't need add delta_bl for bl_min\n");
		return false;
	}

	if (hisifd->bl_level > 0) {
		if (hisifb_display_effect_is_need_blc(hisifd)) {
			int bl = MIN((int)hisifd->panel_info.bl_max, MAX((int)hisifd->panel_info.bl_min, backlight_in + hisifd->de_info.blc_delta));
			if (*backlight_out != bl) {
				HISI_FB_DEBUG("[effect] delta:%d bl:%d(%d)->%d\n", hisifd->de_info.blc_delta, backlight_in, hisifd->bl_level, bl);
				*backlight_out = bl;
				changed = true;
				hisifd->de_info.blc_used = true;
			}
		} else {
			if (hisifd->de_info.blc_used) {
			if (*backlight_out != backlight_in) {
				HISI_FB_DEBUG("[effect] bl:%d->%d\n", *backlight_out, backlight_in);
				*backlight_out = backlight_in;
				changed = true;
				}
				hisifd->de_info.blc_used = false;
			}
		}
	}

	return changed;
}


int hisifb_display_effect_blc_cabc_update(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	if ((hisifd->panel_info.blpwm_input_ena || hisifb_display_effect_is_need_blc(hisifd)) && hisifd->cabc_update) {
		hisifd->cabc_update(hisifd);
	}

	return 0;
}

void init_hiace(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *hiace_base = NULL;

	unsigned long dw_jiffies = 0;
	uint32_t tmp = 0;
	bool is_ready = false;

	uint32_t global_hist_ab_work;
	uint32_t global_hist_ab_shadow;
	uint32_t gamma_ab_work;
	uint32_t gamma_ab_shadow;
	uint32_t width;
	uint32_t height;
	uint32_t half_block_w;
	uint32_t half_block_h;
	uint32_t lhist_sft;
	uint32_t slop;
	uint32_t th_max;
	uint32_t th_min;
	uint32_t up_thres;
	uint32_t low_thres;
	uint32_t fixbit_x;
	uint32_t fixbit_y;
	uint32_t reciprocal_x;
	uint32_t reciprocal_y;

	uint32_t block_pixel_num;
	uint32_t max_lhist_block_pixel_num;
	uint32_t max_lhist_bin_reg_num;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] HIACE is not supported!\n");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return;
	}

	if(!check_underflow_clear_ack(hisifd)){
		HISI_FB_ERR("[effect] fb%d, clear_ack is not return!\n", hisifd->index);
		return;
	}

	set_reg(hiace_base + HIACE_BYPASS_ACE, 0x1, 1, 0);
	set_reg(hiace_base + HIACE_INIT_GAMMA, 0x1, 1, 0);
	set_reg(hiace_base + HIACE_UPDATE_LOCAL, 0x1, 1, 0);

	//parameters
	width = hisifd->panel_info.xres & 0x1fff;
	height = hisifd->panel_info.yres & 0x1fff;
	set_reg(hiace_base + HIACE_IMAGE_INFO, (height << 16) | width, 32, 0);

	half_block_w = (width / 12) & 0x1ff;
	half_block_h = ((height + 11) / 12) & 0x1ff;
	set_reg(hiace_base + HIACE_HALF_BLOCK_H_W,
		(half_block_h << 16) | half_block_w, 32, 0);

	block_pixel_num = (half_block_w * half_block_h) << 2;
	max_lhist_block_pixel_num = block_pixel_num << 2;
	max_lhist_bin_reg_num = (1 << 16) - 1; // each local hist bin 20bit -> 16bit
	if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 1)) {
		lhist_sft = 0;
	} else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 2)) {
		lhist_sft = 1;
	} else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 3)) {
		lhist_sft = 2;
	} else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 4)) {
		lhist_sft = 3;
	} else {
		lhist_sft = 4;
	}
	set_reg(hiace_base + HIACE_LHIST_SFT, lhist_sft, 3, 0);
	pinfo->hiace_param.ilhist_sft = (int)lhist_sft;

	slop = 68 & 0xff;
	th_min = 0 & 0x1ff;
	th_max = 30 & 0x1ff;
	set_reg(hiace_base + HIACE_HUE, (slop << 24) | (th_max << 12) | th_min, 32, 0);

	slop = 68 & 0xff;
	th_min = 80 & 0xff;
	th_max = 140 & 0xff;
	set_reg(hiace_base + HIACE_SATURATION, (slop << 24) | (th_max << 12) | th_min, 32, 0);

	slop = 68 & 0xff;
	th_min = 100 & 0xff;
	th_max = 255 & 0xff;
	set_reg(hiace_base + HIACE_VALUE, (slop << 24) | (th_max << 12) | th_min, 32, 0);

	set_reg(hiace_base + HIACE_SKIN_GAIN, 128, 8, 0);

	up_thres = 248 & 0xff;
	low_thres = 8 & 0xff;
	set_reg(hiace_base + HIACE_UP_LOW_TH, (up_thres << 8) | low_thres, 32, 0);

	fixbit_x = get_fixed_point_offset(half_block_w) & 0x1f;
	fixbit_y = get_fixed_point_offset(half_block_h) & 0x1f;
	reciprocal_x = (1U << (fixbit_x + 8)) / (2 * MAX(half_block_w, 1)) & 0x3ff;
	reciprocal_y = (1U << (fixbit_y + 8)) / (2 * MAX(half_block_h, 1)) & 0x3ff;
	set_reg(hiace_base + HIACE_XYWEIGHT, (fixbit_y << 26) | (reciprocal_y << 16)
		| (fixbit_x << 10) | reciprocal_x, 32, 0);

	EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] half_block_w:%d, half_block_h:%d, fixbit_x:%d, fixbit_y:%d, reciprocal_x:%d, reciprocal_y:%d, lhist_sft:%d\n",
					 half_block_w, half_block_h, fixbit_x, fixbit_y, reciprocal_x, reciprocal_y, lhist_sft);

	//wait for gamma init finishing
	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(hiace_base + HIACE_INIT_GAMMA);
		if ((tmp & 0x1) != 0x1) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies)); //lint !e550

	if (!is_ready) {
		HISI_FB_INFO("[effect] fb%d, HIACE_INIT_GAMMA is not ready! HIACE_INIT_GAMMA=0x%08X.\n",
					 hisifd->index, tmp);
	}

	global_hist_ab_work = inp32(hiace_base + HIACE_GLOBAL_HIST_AB_WORK);
	global_hist_ab_shadow = !global_hist_ab_work;

	gamma_ab_work = inp32(hiace_base + HIACE_GAMMA_AB_WORK);
	gamma_ab_shadow = !gamma_ab_work;

	set_reg(hiace_base + HIACE_GLOBAL_HIST_AB_SHADOW, global_hist_ab_shadow, 1, 0);
	set_reg(hiace_base + HIACE_GAMMA_AB_SHADOW, gamma_ab_shadow, 1, 0);

}

void hisi_dss_dpp_hiace_set_reg(struct hisi_fb_data_type *hisifd)
{
	char __iomem *hiace_base = NULL;
	dss_ce_info_t *ce_info = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	if (NULL == hisifd->dss_base) {
		HISI_FB_ERR("[effect] dss_base is NULL\n");
		return;
	}

	if (hisifd->panel_info.hiace_support == 0) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] HIACE is not support!\n");
		return;
	}

	if (PRIMARY_PANEL_IDX == hisifd->index) {
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
		ce_info = &(hisifd->hiace_info);
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!\n", hisifd->index);
		return;
	}

	if (!hisifb_display_effect_is_need_ace(hisifd)) {
		g_hiace_service.use_last_value = false;
		return;
	}

	if(!check_underflow_clear_ack(hisifd)){
		HISI_FB_ERR("[effect] fb%d, clear_ack is not return!\n", hisifd->index);
		return;
	}

	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] step in\n");

	//lint -e{438}
	if (g_hiace_service.use_last_value) {
		int gamma_ab_shadow = inp32(hiace_base + HIACE_GAMMA_AB_SHADOW);
		int gamma_ab_work = inp32(hiace_base + HIACE_GAMMA_AB_WORK);

		set_reg(hiace_base + HIACE_VALUE, (uint32_t)ce_info->thminv, 8, 0);

		if (gamma_ab_shadow == gamma_ab_work) {
			int i = 0;

			//write gamma lut
			set_reg(hiace_base + HIACE_GAMMA_EN, 1, 1, 31);

			mutex_lock(&ce_info->lut_lock);
			for (i = 0; i < (6 * 6 * 11); i++) {
				// cppcheck-suppress *
				outp32(hiace_base + HIACE_GAMMA_VxHy_3z2_3z1_3z_W, hisifd->hiace_info.lut_table[i]);
			}
			mutex_unlock(&ce_info->lut_lock);

			set_reg(hiace_base + HIACE_GAMMA_EN, 0, 1, 31);

			gamma_ab_shadow ^= 1;
			outp32(hiace_base + HIACE_GAMMA_AB_SHADOW, gamma_ab_shadow);
		}

		g_hiace_service.use_last_value = false;
	}
}

void hisi_dpp_hiace_end_handle_func(struct work_struct *work)
{
	struct hisi_fb_data_type *hisifd = NULL;
	char __iomem *hiace_base = NULL;
	uint32_t * global_hist_ptr = NULL;
	uint32_t * local_hist_ptr = NULL;
	dss_ce_info_t *ce_info = NULL;

	int i = 0;
	int global_hist_ab_shadow = 0;
	int global_hist_ab_work = 0;
	int local_valid = 0;

	time_interval_t interval_total = {0};
	time_interval_t interval_hist_global = {0};
	time_interval_t interval_hist_local = {0};
	DEFINE_DELAY_RECORD(delay_total, "interrupt handling");
	DEFINE_DELAY_RECORD(delay_hist_global, "global hist reading");
	DEFINE_DELAY_RECORD(delay_hist_local, "local hist reading");

	hisifd = container_of(work, struct hisi_fb_data_type, hiace_end_work);
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	if (hisifd->panel_info.hiace_support == 0) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] HIACE is not support!\n");
		return;
	}

	if (PRIMARY_PANEL_IDX == hisifd->index) {
		ce_info = &(hisifd->hiace_info);
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!\n", hisifd->index);
		return;
	}

	if (!hisifb_display_effect_is_need_ace(hisifd)) {
		enable_hiace(hisifd, false);
		return;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] panel power off!\n");
		goto ERR_OUT;
	}

	if(!check_underflow_clear_ack(hisifd)){
		HISI_FB_ERR("[effect] fb%d, clear_ack is not return!\n", hisifd->index);
		goto ERR_OUT;
	}

	if (g_hiace_service.is_ready) {
		g_hiace_service.is_ready = false;
	} else {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] service is not ready!\n");
		goto ERR_OUT;
	}

	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] step in\n");

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_total.start = get_timestamp_in_us();
	}

	hisifb_activate_vsync(hisifd);

	down(&hisifd->hiace_clear_sem);

	mutex_lock(&ce_info->hist_lock);

	local_valid = inp32(hiace_base + HIACE_LOCAL_VALID);
	if (local_valid == 1) {
		//read local hist
		local_hist_ptr = &hisifd->hiace_info.histogram[HIACE_GHIST_RANK];
		outp32(hiace_base + HIACE_LHIST_EN, (1 << 31));

		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_hist_local.start = get_timestamp_in_us();
		}
		for (i = 0; i < (6 * 6 * 16); i++) {//H  L
			local_hist_ptr[i] = inp32(hiace_base + HIACE_LOCAL_HIST_VxHy_2z_2z1);
		}
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_hist_local.stop = get_timestamp_in_us();
			count_delay(&delay_hist_local, interval_hist_local.stop - interval_hist_local.start);
		}

		outp32(hiace_base + HIACE_LHIST_EN, (0 << 31));
		outp32(hiace_base + HIACE_UPDATE_LOCAL, 1);
	}

	global_hist_ab_shadow = inp32(hiace_base + HIACE_GLOBAL_HIST_AB_SHADOW);
	global_hist_ab_work = inp32(hiace_base + HIACE_GLOBAL_HIST_AB_WORK);
	if (global_hist_ab_shadow == global_hist_ab_work) {
		//read global hist
		global_hist_ptr = &hisifd->hiace_info.histogram[0];//HIACE_GHIST_RANK

		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_hist_global.start = get_timestamp_in_us();
		}
		for (i = 0; i < 32; i++) {
			global_hist_ptr[i] = inp32(hiace_base + HIACE_GLOBAL_HIST_LUT_ADDR + i * 4);
		}
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_hist_global.stop = get_timestamp_in_us();
			count_delay(&delay_hist_global, interval_hist_global.stop - interval_hist_global.start);
		}

		outp32(hiace_base +  HIACE_GLOBAL_HIST_AB_SHADOW, global_hist_ab_shadow ^ 1);
	}

	mutex_unlock(&ce_info->hist_lock);

	up(&hisifd->hiace_clear_sem);

	hisifb_deactivate_vsync(hisifd);

	up(&hisifd->blank_sem);

	if ((local_valid == 1) || (global_hist_ab_shadow == global_hist_ab_work)) { // global or local hist is updated
		ce_info->algorithm_result = hisifb_ce_do_contrast(hisifd);
	}
	g_hiace_service.is_ready = true;

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] panel power off!\n");
		goto ERR_OUT;
	}

	hisifb_activate_vsync(hisifd);

	down(&hisifd->hiace_clear_sem);

	//lint -e{438}
	if (ce_info->algorithm_result == 0) {
		int gamma_ab_shadow = inp32(hiace_base + HIACE_GAMMA_AB_SHADOW);
		int gamma_ab_work = inp32(hiace_base + HIACE_GAMMA_AB_WORK);
		time_interval_t interval_lut = {0};
		DEFINE_DELAY_RECORD(delay_lut, "lut writing");

		set_reg(hiace_base + HIACE_VALUE, (uint32_t)ce_info->thminv, 12, 0);

		if (gamma_ab_shadow == gamma_ab_work) {
			//write gamma lut
			set_reg(hiace_base + HIACE_GAMMA_EN, 1, 1, 31);

			if (g_debug_effect & DEBUG_EFFECT_DELAY) {
				interval_lut.start = get_timestamp_in_us();
			}
			mutex_lock(&ce_info->lut_lock);
			for (i = 0; i < (6 * 6 * 11); i++) {
				// cppcheck-suppress *
				outp32(hiace_base + HIACE_GAMMA_VxHy_3z2_3z1_3z_W, hisifd->hiace_info.lut_table[i]);
			}
			mutex_unlock(&ce_info->lut_lock);
			if (g_debug_effect & DEBUG_EFFECT_DELAY) {
				interval_lut.stop = get_timestamp_in_us();
				count_delay(&delay_lut, interval_lut.stop - interval_lut.start);
			}

			set_reg(hiace_base + HIACE_GAMMA_EN, 0, 1, 31);

			gamma_ab_shadow ^= 1;
			outp32(hiace_base + HIACE_GAMMA_AB_SHADOW, gamma_ab_shadow);
		}

		ce_info->algorithm_result = 1;
	}


	up(&hisifd->hiace_clear_sem);

	hisifb_deactivate_vsync(hisifd);

	if (ce_info->gradual_frames > 0) {
		ce_info->gradual_frames--;
	}

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_total.stop = get_timestamp_in_us();
		count_delay(&delay_total, interval_total.stop - interval_total.start);
	}

ERR_OUT:
	up(&hisifd->blank_sem);
} //lint !e550
//lint +e845, +e732, +e774

////////////////////////////////////////////////////////////////////////////////
//ACE

void init_acm_ce(struct hisi_fb_data_type *hisifd)
{
	(void)hisifd;
}

void hisi_dss_dpp_ace_set_reg(struct hisi_fb_data_type *hisifd)
{
	(void)hisifd;
}

void hisi_dpp_ace_end_handle_func(struct work_struct *work)
{
	(void)work;
}

/*******************************************************************************
** GM IGM
*/
#define GM_LUT_LEN 257
#define GM_LUT_MHLEN 254
static uint16_t degm_gm_lut[GM_LUT_LEN *6];

int hisifb_use_dynamic_gamma(struct hisi_fb_data_type *hisifd, char __iomem *dpp_base)
{
	uint32_t i = 0;
	uint32_t index = 0;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *gamma_pre_lut_base = NULL;//lint !e838

	if (hisifd == NULL) {
		return -1;
	}

	if (dpp_base == NULL) {
		return -1;
	}

	pinfo = &(hisifd->panel_info);

	if(hisifd->dynamic_gamma_info.use_dynamic_gm_init == 1) {
		uint16_t* gm_lut_r =  degm_gm_lut;
		uint16_t* gm_lut_g =  gm_lut_r + GM_LUT_LEN;
		uint16_t* gm_lut_b =  gm_lut_g + GM_LUT_LEN;

		for (i = 0; i < pinfo->gamma_lut_table_len / 2; i++) {
			index = i << 1;
			if (index >= GM_LUT_MHLEN)
			{
				index = GM_LUT_MHLEN;
			}
			outp32(dpp_base + (U_GAMA_R_COEF + i * 4), gm_lut_r[index] | gm_lut_r[index+1] << 16);
			outp32(dpp_base + (U_GAMA_G_COEF + i * 4), gm_lut_g[index] | gm_lut_g[index+1] << 16);
			outp32(dpp_base + (U_GAMA_B_COEF + i * 4), gm_lut_b[index] | gm_lut_b[index+1] << 16);

			if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
				gamma_pre_lut_base = hisifd->dss_base + DSS_DPP_GAMA_PRE_LUT_OFFSET;
				//GAMA PRE LUT
				outp32(gamma_pre_lut_base + (U_GAMA_PRE_R_COEF + i * 4), gm_lut_r[index] | gm_lut_r[index+1] << 16);
				outp32(gamma_pre_lut_base + (U_GAMA_PRE_G_COEF + i * 4), gm_lut_g[index] | gm_lut_g[index+1] << 16);
				outp32(gamma_pre_lut_base + (U_GAMA_PRE_B_COEF + i * 4), gm_lut_b[index] | gm_lut_b[index+1] << 16);
			}
		}
		outp32(dpp_base + U_GAMA_R_LAST_COEF, gm_lut_r[pinfo->gamma_lut_table_len - 1]);
		outp32(dpp_base + U_GAMA_G_LAST_COEF, gm_lut_g[pinfo->gamma_lut_table_len - 1]);
		outp32(dpp_base + U_GAMA_B_LAST_COEF, gm_lut_b[pinfo->gamma_lut_table_len - 1]);

		if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
			//GAMA PRE LUT
			outp32(gamma_pre_lut_base + U_GAMA_PRE_R_LAST_COEF, gm_lut_r[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_pre_lut_base + U_GAMA_PRE_G_LAST_COEF, gm_lut_g[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_pre_lut_base + U_GAMA_PRE_B_LAST_COEF, gm_lut_b[pinfo->gamma_lut_table_len - 1]);
		}
		return 1;//lint !e438
	}

	return 0;//lint !e438

}//lint !e550

int hisifb_use_dynamic_degamma(struct hisi_fb_data_type *hisifd, char __iomem *dpp_base)
{
	uint32_t i = 0;
	uint32_t index = 0;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		return -1;
	}

	if (dpp_base == NULL) {
		return -1;
	}

	pinfo = &(hisifd->panel_info);

	if(hisifd->dynamic_gamma_info.use_dynamic_gm_init == 1) {

		uint16_t* degm_lut_r = degm_gm_lut + GM_LUT_LEN * 3;
		uint16_t* degm_lut_g = degm_lut_r + GM_LUT_LEN;
		uint16_t* degm_lut_b = degm_lut_g + GM_LUT_LEN;

		for (i = 0; i < pinfo->igm_lut_table_len / 2; i++) {
			index = i << 1;
			if(index >= GM_LUT_MHLEN)
			{
				index = GM_LUT_MHLEN;
			}
			outp32(dpp_base + (U_DEGAMA_R_COEF +  i * 4), degm_lut_r[index] | degm_lut_r[index+1] << 16);
			outp32(dpp_base + (U_DEGAMA_G_COEF +  i * 4), degm_lut_g[index] | degm_lut_g[index+1] << 16);
			outp32(dpp_base + (U_DEGAMA_B_COEF +  i * 4), degm_lut_b[index] | degm_lut_b[index+1] << 16);
		}
		outp32(dpp_base + U_DEGAMA_R_LAST_COEF, degm_lut_r[pinfo->igm_lut_table_len - 1]);
		outp32(dpp_base + U_DEGAMA_G_LAST_COEF, degm_lut_g[pinfo->igm_lut_table_len - 1]);
		outp32(dpp_base + U_DEGAMA_B_LAST_COEF, degm_lut_b[pinfo->igm_lut_table_len - 1]);

		return 1;
	}

	return 0;

}

void hisifb_update_dynamic_gamma(struct hisi_fb_data_type *hisifd, const char* buffer, size_t len)
{
	struct hisi_panel_info *pinfo = NULL;
	if (hisifd == NULL || buffer == NULL) {
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) { //lint !e774
		return;
	}

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_GAMA)){//lint !e774
		return;
	}

	hisifd->dynamic_gamma_info.use_dynamic_gm = 0;
	hisifd->dynamic_gamma_info.use_dynamic_gm_init = 0;

	if (pinfo->gamma_support == 1) {
		if ((len > 0) && (len <= (int)sizeof(degm_gm_lut))) {
			memcpy((char*)degm_gm_lut, buffer, len);
			hisifd->dynamic_gamma_info.use_dynamic_gm = 1;
			hisifd->dynamic_gamma_info.use_dynamic_gm_init = 1;
		}
	}

}

void hisifb_update_gm_from_reserved_mem(uint32_t *gm_r, uint32_t *gm_g, uint32_t *gm_b,
	uint32_t *igm_r, uint32_t *igm_g, uint32_t *igm_b)
{
	int i = 0;
	int len = 0;
	uint16_t *u16_gm_r = NULL;
	uint16_t *u16_gm_g = NULL;
	uint16_t *u16_gm_b = NULL;
	uint16_t *u16_igm_r = NULL;
	uint16_t *u16_igm_g = NULL;
	uint16_t *u16_igm_b = NULL;
	void *mem = NULL;
	unsigned long gm_addr = 0;
	unsigned long gm_size = 0;

	g_factory_gamma_enable = 0;

	if (gm_r == NULL || gm_g == NULL || gm_b == NULL
		|| igm_r == NULL || igm_g == NULL || igm_b == NULL) {
		return;
	}

	gm_addr = HISI_SUB_RESERVED_LCD_GAMMA_MEM_PHYMEM_BASE;
	gm_size = HISI_SUB_RESERVED_LCD_GAMMA_MEM_PHYMEM_SIZE;

	HISI_FB_INFO("gamma kernel gm_addr = 0x%lx  gm_size = 0x%lx \n", gm_addr, gm_size);

	mem = (void *)ioremap_wc(gm_addr, gm_size);
	if (mem == NULL) {
		HISI_FB_ERR("mem ioremap error ! \n");
		return;
	}
	memcpy(&len, mem, 4UL);
	HISI_FB_INFO("gamma read len = %d \n", len);
	if (len != GM_IGM_LEN) {
		HISI_FB_ERR("gamma read len error ! \n");
		iounmap(mem);
		return;
	}

	u16_gm_r = (uint16_t *)(mem + 4);
	u16_gm_g = u16_gm_r + GM_LUT_LEN;
	u16_gm_b = u16_gm_g + GM_LUT_LEN;

	u16_igm_r = u16_gm_b + GM_LUT_LEN;
	u16_igm_g = u16_igm_r + GM_LUT_LEN;
	u16_igm_b = u16_igm_g + GM_LUT_LEN;

	for (i = 0; i < GM_LUT_LEN; i++) {
		gm_r[i] = u16_gm_r[i];
		gm_g[i] = u16_gm_g[i];
		gm_b[i] = u16_gm_b[i];

		igm_r[i]  = u16_igm_r[i];
		igm_g[i] = u16_igm_g[i];
		igm_b[i] = u16_igm_b[i];
	}
	iounmap(mem);

	g_factory_gamma_enable = 1;
	return;
}

/*lint -e571, -e573, -e737, -e732, -e850, -e730, -e713, -e529, -e574, -e679, -e732, -e845, -e570, -e774*/

#define ACM_HUE_LUT_LENGTH ((uint32_t)256)
#define ACM_SATA_LUT_LENGTH ((uint32_t)256)
#define ACM_SATR_LUT_LENGTH ((uint32_t)64)

#define LCP_GMP_LUT_LENGTH	((uint32_t)9*9*9)
#define LCP_XCC_LUT_LENGTH	((uint32_t)12)

#define IGM_LUT_LEN ((uint32_t)257)
#define GAMMA_LUT_LEN ((uint32_t)257)

#define BYTES_PER_TABLE_ELEMENT 4

static int hisi_effect_copy_to_user(uint32_t *table_dst, uint32_t *table_src, uint32_t table_length)
{
	unsigned long table_size = 0;

	if ((NULL == table_dst) || (NULL == table_src) || (table_length == 0)) {
		HISI_FB_ERR("invalid input parameters.\n");
		return -EINVAL;
	}

	table_size = (unsigned long)table_length * BYTES_PER_TABLE_ELEMENT;

	if (copy_to_user(table_dst, table_src, table_size)) {
		HISI_FB_ERR("failed to copy table to user.\n");
		return -EINVAL;
	}

	return 0;
}

static int hisi_effect_alloc_and_copy(uint32_t **table_dst, uint32_t *table_src,
	uint32_t lut_table_length, bool copy_user)
{
	uint32_t *table_new = NULL;
	unsigned long table_size = 0;

	if ((NULL == table_dst) ||(NULL == table_src) ||  (lut_table_length == 0)) {
		HISI_FB_ERR("invalid input parameter");
		return -EINVAL;
	}

	table_size = (unsigned long)lut_table_length * BYTES_PER_TABLE_ELEMENT;

	if (*table_dst == NULL) {
		table_new = (uint32_t *)kmalloc(table_size, GFP_ATOMIC);
		if (table_new) {
			memset(table_new, 0, table_size);
			*table_dst = table_new;
		} else {
			HISI_FB_ERR("failed to kmalloc lut_table!\n");
			return -EINVAL;
		}
	}

	if (copy_user) {
		if (copy_from_user(*table_dst, table_src, table_size)) {
			HISI_FB_ERR("failed to copy table from user\n");
			if (table_new)
				kfree(table_new);
			*table_dst = NULL;
			return -EINVAL;
		}
	} else {
		memcpy(*table_dst, table_src, table_size);
	}

	return 0;
}

static void hisi_effect_kfree(uint32_t **free_table)
{
	if (*free_table) {
		kfree((uint32_t *) *free_table);
		*free_table = NULL;
	}
}

static void free_acm_table(struct acm_info *acm)
{
	if(acm == NULL){
		HISI_FB_ERR("acm is NULL!\n");
		return;
	}

	hisi_effect_kfree(&acm->hue_table);
	hisi_effect_kfree(&acm->sata_table);
	hisi_effect_kfree(&acm->satr0_table);
	hisi_effect_kfree(&acm->satr1_table);
	hisi_effect_kfree(&acm->satr2_table);
	hisi_effect_kfree(&acm->satr3_table);
	hisi_effect_kfree(&acm->satr4_table);
	hisi_effect_kfree(&acm->satr5_table);
	hisi_effect_kfree(&acm->satr6_table);
	hisi_effect_kfree(&acm->satr7_table);
}

static void free_gamma_table(struct gamma_info *gamma)
{
	if(gamma == NULL){
		HISI_FB_ERR("gamma is NULL!\n");
		return;
	}

	hisi_effect_kfree(&gamma->gamma_r_table);
	hisi_effect_kfree(&gamma->gamma_g_table);
	hisi_effect_kfree(&gamma->gamma_b_table);
}

int hisi_effect_arsr2p_info_get(struct hisi_fb_data_type *hisifd, struct arsr2p_info *arsr2p)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == arsr2p) {
		HISI_FB_ERR("fb%d, arsr2p is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.arsr2p_sharp_support) {
		HISI_FB_INFO("fb%d, arsr2p is not supported!\n", hisifd->index);
		return 0;
	}

	memcpy(&arsr2p[0], &(hisifd->dss_module_default.arsr2p[DSS_RCHN_V1].arsr2p_effect), sizeof(struct arsr2p_info));
	memcpy(&arsr2p[1], &(hisifd->dss_module_default.arsr2p[DSS_RCHN_V1].arsr2p_effect_scale_up), sizeof(struct arsr2p_info));
	memcpy(&arsr2p[2], &(hisifd->dss_module_default.arsr2p[DSS_RCHN_V1].arsr2p_effect_scale_down), sizeof(struct arsr2p_info));
	arsr2p[0].sharp_enable = hisifd->panel_info.prefix_sharpness2D_support;
	arsr2p[1].sharp_enable = hisifd->panel_info.prefix_sharpness2D_support;
	arsr2p[2].sharp_enable = hisifd->panel_info.prefix_sharpness2D_support;

	return 0;
}

int hisi_effect_arsr1p_info_get(struct hisi_fb_data_type *hisifd, struct arsr1p_info *arsr1p)
{
    (void)hisifd, (void)arsr1p;
    return 0;

}

int hisi_effect_acm_info_get(struct hisi_fb_data_type *hisifd, struct acm_info *acm_dst)
{
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == acm_dst) {
		HISI_FB_ERR("fb%d, acm is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.acm_support) {
		HISI_FB_INFO("fb%d, acm is not supported!\n", hisifd->index);
		return 0;
	}

	pinfo = &hisifd->panel_info;

	acm_dst->acm_hue_rlh01 = (pinfo->r1_lh<<16) | pinfo->r0_lh;
	acm_dst->acm_hue_rlh23 = (pinfo->r3_lh<<16) | pinfo->r2_lh;
	acm_dst->acm_hue_rlh45 = (pinfo->r5_lh<<16) | pinfo->r4_lh;
	acm_dst->acm_hue_rlh67 = (pinfo->r6_hh<<16) | pinfo->r6_lh;
	acm_dst->acm_hue_param01 = pinfo->hue_param01;
	acm_dst->acm_hue_param23 = pinfo->hue_param23;
	acm_dst->acm_hue_param45 = pinfo->hue_param45;
	acm_dst->acm_hue_param67 = pinfo->hue_param67;
	acm_dst->acm_hue_smooth0 = pinfo->hue_smooth0;
	acm_dst->acm_hue_smooth1 = pinfo->hue_smooth1;
	acm_dst->acm_hue_smooth2 = pinfo->hue_smooth2;
	acm_dst->acm_hue_smooth3 = pinfo->hue_smooth3;
	acm_dst->acm_hue_smooth4 = pinfo->hue_smooth4;
	acm_dst->acm_hue_smooth5 = pinfo->hue_smooth5;
	acm_dst->acm_hue_smooth6 = pinfo->hue_smooth6;
	acm_dst->acm_hue_smooth7 = pinfo->hue_smooth7;
	acm_dst->acm_color_choose = pinfo->color_choose;
	acm_dst->acm_l_cont_en = pinfo->l_cont_en;
	acm_dst->acm_lc_param01  = pinfo->lc_param01;
	acm_dst->acm_lc_param23  = pinfo->lc_param23;
	acm_dst->acm_lc_param45  = pinfo->lc_param45;
	acm_dst->acm_lc_param67  = pinfo->lc_param67;
	acm_dst->acm_l_adj_ctrl = pinfo->l_adj_ctrl;
	acm_dst->acm_capture_ctrl = pinfo->capture_ctrl;
	acm_dst->acm_capture_in = pinfo->capture_in;
	acm_dst->acm_capture_out = pinfo->capture_out;
	acm_dst->acm_ink_ctrl = pinfo->ink_ctrl;
	acm_dst->acm_ink_out = pinfo->ink_out;
	acm_dst->acm_en = pinfo->acm_ce_support;

	if (hisi_effect_copy_to_user(acm_dst->hue_table, pinfo->acm_lut_hue_table, ACM_HUE_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm hue table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->sata_table, pinfo->acm_lut_sata_table, ACM_SATA_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm sata table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr0_table, pinfo->acm_lut_satr0_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr0 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr1_table, pinfo->acm_lut_satr1_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr1 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr2_table, pinfo->acm_lut_satr2_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr2 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr3_table, pinfo->acm_lut_satr3_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr3 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr4_table, pinfo->acm_lut_satr4_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr4 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr5_table, pinfo->acm_lut_satr5_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr5 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr6_table, pinfo->acm_lut_satr6_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr6 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr7_table, pinfo->acm_lut_satr7_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr7 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	return 0;
}

int hisi_effect_lcp_info_get(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == lcp) {
		HISI_FB_ERR("fb%d, lcp is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);

	if (hisifd->effect_ctl.lcp_gmp_support && (pinfo->gmp_lut_table_len == LCP_GMP_LUT_LENGTH)) {
		ret = hisi_effect_copy_to_user(lcp->gmp_table_low32, pinfo->gmp_lut_table_low32bit, LCP_GMP_LUT_LENGTH);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gmp_table_low32 to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(lcp->gmp_table_high4, pinfo->gmp_lut_table_high4bit, LCP_GMP_LUT_LENGTH);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gmp_table_high4 to user!\n", hisifd->index);
			goto err_ret;
		}
	}

	if (hisifd->effect_ctl.lcp_xcc_support && (pinfo->xcc_table_len == LCP_XCC_LUT_LENGTH)) {
		ret = hisi_effect_copy_to_user(lcp->xcc_table, pinfo->xcc_table, LCP_XCC_LUT_LENGTH);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy xcc_table to user!\n", hisifd->index);
			goto err_ret;
		}
	}

	if (hisifd->effect_ctl.lcp_igm_support && (pinfo->igm_lut_table_len == IGM_LUT_LEN)) {
		ret = hisi_effect_copy_to_user(lcp->igm_r_table, pinfo->igm_lut_table_R, IGM_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy igm_r_table to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(lcp->igm_g_table, pinfo->igm_lut_table_G, IGM_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy igm_g_table to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(lcp->igm_b_table, pinfo->igm_lut_table_B, IGM_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy igm_b_table to user!\n", hisifd->index);
			goto err_ret;
		}
	}

err_ret:
	return ret;
}
int hisi_effect_hiace_info_get(struct hisi_fb_data_type *hisifd, struct hiace_info *hiace) {
	(void)hisifd, (void)hiace;
	return 0;
}
int hisi_effect_gamma_info_get(struct hisi_fb_data_type *hisifd, struct gamma_info *gamma)
{
	struct hisi_panel_info *pinfo = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == gamma) {
		HISI_FB_ERR("fb%d, gamma is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.gamma_support) {
		HISI_FB_INFO("fb%d, gamma is not supported!\n", hisifd->index);
		return 0;
	}

	pinfo = &(hisifd->panel_info);

	if (hisifd->effect_ctl.lcp_gmp_support && (pinfo->gamma_lut_table_len== GAMMA_LUT_LEN)) {
		gamma->para_mode = 0;

		ret = hisi_effect_copy_to_user(gamma->gamma_r_table, pinfo->gamma_lut_table_R, GAMMA_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gamma_r_table to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(gamma->gamma_g_table, pinfo->gamma_lut_table_G, GAMMA_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gamma_g_table to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(gamma->gamma_b_table, pinfo->gamma_lut_table_B, GAMMA_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gamma_b_table to user!\n", hisifd->index);
			goto err_ret;
		}

	}

err_ret:
	return ret;
}
#define ARSR2P_MAX_NUM 3

int hisi_effect_arsr2p_info_set(struct hisi_fb_data_type *hisifd, struct arsr2p_info *arsr2p)
{
	uint32_t i;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == arsr2p) {
		HISI_FB_ERR("fb%d, arsr2p is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.arsr2p_sharp_support) {
		HISI_FB_INFO("fb%d, arsr2p sharp is not supported!\n", hisifd->index);
		return 0;
	}

	for (i = 0; i < ARSR2P_MAX_NUM; i++) {
		if (arsr2p[i].update == 1) {
			memcpy(&(hisifd->effect_info.arsr2p[i]), &(arsr2p[i]), sizeof(struct arsr2p_info));
		}
	}

	hisifd->effect_updated_flag.arsr2p_effect_updated = true;

	//debug info
	for (i = 0; i < ARSR2P_MAX_NUM; i++) {
		if (hisifd->effect_info.arsr2p[i].update) {
			HISI_FB_INFO("mode %d: enable : %u, sharp_enable:%u, shoot_enable:%u, skin_enable:%u, update: %u\n",
				i, hisifd->effect_info.arsr2p[i].enable, hisifd->effect_info.arsr2p[i].sharp_enable, hisifd->effect_info.arsr2p[i].shoot_enable, hisifd->effect_info.arsr2p[i].skin_enable,
                hisifd->effect_info.arsr2p[i].update);
		}
	}

	return 0;
}

int hisi_effect_arsr1p_info_set(struct hisi_fb_data_type *hisifd, struct arsr1p_info *arsr1p)
{
    (void)hisifd, (void)arsr1p;
    return 0;

}

static int set_acm_normal_param(struct acm_info *acm_dst, struct hisi_panel_info *pinfo)
{
	if (acm_dst == NULL) {
		HISI_FB_DEBUG("acm_dst is NULL!\n");
		return -1;
	}

	if (pinfo == NULL) {
		HISI_FB_DEBUG("pinfo is NULL!\n");
		return -1;
	}

	acm_dst->acm_hue_rlh01 = (pinfo->r1_lh<<16) | pinfo->r0_lh;
	acm_dst->acm_hue_rlh23 = (pinfo->r3_lh<<16) | pinfo->r2_lh;
	acm_dst->acm_hue_rlh45 = (pinfo->r5_lh<<16) | pinfo->r4_lh;
	acm_dst->acm_hue_rlh67 = (pinfo->r6_hh<<16) | pinfo->r6_lh;
	acm_dst->acm_hue_param01 = pinfo->hue_param01;
	acm_dst->acm_hue_param23 = pinfo->hue_param23;
	acm_dst->acm_hue_param45 = pinfo->hue_param45;
	acm_dst->acm_hue_param67 = pinfo->hue_param67;
	acm_dst->acm_hue_smooth0 = pinfo->hue_smooth0;
	acm_dst->acm_hue_smooth1 = pinfo->hue_smooth1;
	acm_dst->acm_hue_smooth2 = pinfo->hue_smooth2;
	acm_dst->acm_hue_smooth3 = pinfo->hue_smooth3;
	acm_dst->acm_hue_smooth4 = pinfo->hue_smooth4;
	acm_dst->acm_hue_smooth5 = pinfo->hue_smooth5;
	acm_dst->acm_hue_smooth6 = pinfo->hue_smooth6;
	acm_dst->acm_hue_smooth7 = pinfo->hue_smooth7;
	acm_dst->acm_color_choose = pinfo->color_choose;
	acm_dst->acm_l_cont_en = pinfo->l_cont_en;
	acm_dst->acm_lc_param01  = pinfo->lc_param01;
	acm_dst->acm_lc_param23  = pinfo->lc_param23;
	acm_dst->acm_lc_param45  = pinfo->lc_param45;
	acm_dst->acm_lc_param67  = pinfo->lc_param67;
	acm_dst->acm_l_adj_ctrl = pinfo->l_adj_ctrl;
	acm_dst->acm_capture_ctrl = pinfo->capture_ctrl;
	acm_dst->acm_ink_ctrl = pinfo->ink_ctrl;
	acm_dst->acm_ink_out = pinfo->ink_out;

	/* malloc acm_dst lut table memory*/
	if (hisi_effect_alloc_and_copy(&acm_dst->hue_table, pinfo->acm_lut_hue_table,
		ACM_HUE_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm hut table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->sata_table, pinfo->acm_lut_sata_table,
		ACM_SATA_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm sata table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr0_table, pinfo->acm_lut_satr0_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr0 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr1_table, pinfo->acm_lut_satr1_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr1 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr2_table, pinfo->acm_lut_satr2_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr2 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr3_table, pinfo->acm_lut_satr3_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr3 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr4_table, pinfo->acm_lut_satr4_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr4 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr5_table, pinfo->acm_lut_satr5_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr5 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr6_table, pinfo->acm_lut_satr6_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr6 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr7_table, pinfo->acm_lut_satr7_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr7 table from panel\n");
		return -EINVAL;
	}

	return 0;
}

static int set_acm_cinema_param(struct acm_info *acm_dst, struct hisi_panel_info *pinfo)
{
	if (acm_dst == NULL) {
		HISI_FB_DEBUG("acm_dst is NULL!\n");
		return -1;
	}

	if (pinfo == NULL) {
		HISI_FB_DEBUG("pinfo is NULL!\n");
		return -1;
	}

	acm_dst->acm_hue_rlh01 = (pinfo->cinema_r1_lh<<16) | pinfo->cinema_r0_lh;
	acm_dst->acm_hue_rlh23 = (pinfo->cinema_r3_lh<<16) | pinfo->cinema_r2_lh;
	acm_dst->acm_hue_rlh45 = (pinfo->cinema_r5_lh<<16) | pinfo->cinema_r4_lh;
	acm_dst->acm_hue_rlh67 = (pinfo->cinema_r6_hh<<16) | pinfo->cinema_r6_lh;

	acm_dst->acm_hue_param01 = pinfo->hue_param01;
	acm_dst->acm_hue_param23 = pinfo->hue_param23;
	acm_dst->acm_hue_param45 = pinfo->hue_param45;
	acm_dst->acm_hue_param67 = pinfo->hue_param67;
	acm_dst->acm_hue_smooth0 = pinfo->hue_smooth0;
	acm_dst->acm_hue_smooth1 = pinfo->hue_smooth1;
	acm_dst->acm_hue_smooth2 = pinfo->hue_smooth2;
	acm_dst->acm_hue_smooth3 = pinfo->hue_smooth3;
	acm_dst->acm_hue_smooth4 = pinfo->hue_smooth4;
	acm_dst->acm_hue_smooth5 = pinfo->hue_smooth5;
	acm_dst->acm_hue_smooth6 = pinfo->hue_smooth6;
	acm_dst->acm_hue_smooth7 = pinfo->hue_smooth7;
	acm_dst->acm_color_choose = pinfo->color_choose;
	acm_dst->acm_l_cont_en = pinfo->l_cont_en;
	acm_dst->acm_lc_param01  = pinfo->lc_param01;
	acm_dst->acm_lc_param23  = pinfo->lc_param23;
	acm_dst->acm_lc_param45  = pinfo->lc_param45;
	acm_dst->acm_lc_param67  = pinfo->lc_param67;
	acm_dst->acm_l_adj_ctrl = pinfo->l_adj_ctrl;
	acm_dst->acm_capture_ctrl = pinfo->capture_ctrl;
	acm_dst->acm_ink_ctrl = pinfo->ink_ctrl;
	acm_dst->acm_ink_out = pinfo->ink_out;
	/* malloc acm_dst lut table memory*/
	if (hisi_effect_alloc_and_copy(&acm_dst->hue_table, pinfo->cinema_acm_lut_hue_table,
		ACM_HUE_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm hut table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->sata_table, pinfo->cinema_acm_lut_sata_table,
		ACM_SATA_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm sata table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr0_table, pinfo->cinema_acm_lut_satr0_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr0 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr1_table, pinfo->cinema_acm_lut_satr1_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr1 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr2_table, pinfo->cinema_acm_lut_satr2_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr2 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr3_table, pinfo->cinema_acm_lut_satr3_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr3 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr4_table, pinfo->cinema_acm_lut_satr4_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr4 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr5_table, pinfo->cinema_acm_lut_satr5_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr5 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr6_table, pinfo->cinema_acm_lut_satr6_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr6 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr7_table, pinfo->cinema_acm_lut_satr7_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr7 table from panel cinema mode\n");
		return -EINVAL;
	}

	return 0;
}

static int set_acm_user_param(struct acm_info *acm_dst, struct acm_info *acm_src)
{
	if (acm_dst == NULL) {
		HISI_FB_DEBUG("acm_dst is NULL!\n");
		return -1;
	}

	if (acm_src == NULL) {
		HISI_FB_DEBUG("acm_src is NULL!\n");
		return -1;
	}

	acm_dst->acm_en = acm_src->acm_en;
	acm_dst->sata_offset = acm_src->sata_offset;
	acm_dst->acm_hue_rlh01 = acm_src->acm_hue_rlh01;
	acm_dst->acm_hue_rlh23 = acm_src->acm_hue_rlh23;
	acm_dst->acm_hue_rlh45 = acm_src->acm_hue_rlh45;
	acm_dst->acm_hue_rlh67 = acm_src->acm_hue_rlh67;
	acm_dst->acm_hue_param01 = acm_src->acm_hue_param01;
	acm_dst->acm_hue_param23 = acm_src->acm_hue_param23;
	acm_dst->acm_hue_param45 = acm_src->acm_hue_param45;
	acm_dst->acm_hue_param67 = acm_src->acm_hue_param67;
	acm_dst->acm_hue_smooth0 = acm_src->acm_hue_smooth0;
	acm_dst->acm_hue_smooth1 = acm_src->acm_hue_smooth1;
	acm_dst->acm_hue_smooth2 = acm_src->acm_hue_smooth2;
	acm_dst->acm_hue_smooth3 = acm_src->acm_hue_smooth3;
	acm_dst->acm_hue_smooth4 = acm_src->acm_hue_smooth4;
	acm_dst->acm_hue_smooth5 = acm_src->acm_hue_smooth5;
	acm_dst->acm_hue_smooth6 = acm_src->acm_hue_smooth6;
	acm_dst->acm_hue_smooth7 = acm_src->acm_hue_smooth7;
	acm_dst->acm_color_choose = acm_src->acm_color_choose;
	acm_dst->acm_l_cont_en = acm_src->acm_l_cont_en;
	acm_dst->acm_lc_param01 = acm_src->acm_lc_param01;
	acm_dst->acm_lc_param23 = acm_src->acm_lc_param23;
	acm_dst->acm_lc_param45 = acm_src->acm_lc_param45;
	acm_dst->acm_lc_param67 = acm_src->acm_lc_param67;
	acm_dst->acm_l_adj_ctrl = acm_src->acm_l_adj_ctrl;
	acm_dst->acm_capture_out = acm_src->acm_capture_out;
	acm_dst->acm_ink_ctrl = acm_src->acm_ink_ctrl;
	acm_dst->acm_ink_out = acm_src->acm_ink_out;

	/* malloc acm_dst lut table memory*/
	if (hisi_effect_alloc_and_copy(&acm_dst->hue_table, acm_src->hue_table,
		ACM_HUE_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm hut table from user\n");
		return -EINVAL;
	}

	if (hisi_effect_alloc_and_copy(&acm_dst->sata_table, acm_src->sata_table,
		ACM_SATA_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm sata table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr0_table, acm_src->satr0_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr0 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr1_table, acm_src->satr1_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr1 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr2_table, acm_src->satr2_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr2 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr3_table, acm_src->satr3_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr3 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr4_table, acm_src->satr4_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr4 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr5_table, acm_src->satr5_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr5 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr6_table, acm_src->satr6_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr6 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr7_table, acm_src->satr7_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr7 table from user\n");
		return -EINVAL;
	}

	return 0;
}

int hisi_effect_acm_info_set(struct hisi_fb_data_type *hisifd, struct acm_info *acm_src)
{
	struct acm_info *acm_dst = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == acm_src) {
		HISI_FB_ERR("fb%d, acm_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	acm_dst = &(hisifd->effect_info.acm);
	pinfo = &(hisifd->panel_info);

	if (!hisifd->effect_ctl.acm_support) {
		HISI_FB_INFO("fb%d, acm is not supported!\n", hisifd->index);
		return 0;
	}

	/*set acm info*/
	if (hisifd->effect_updated_flag.acm_effect_updated == false) {
		acm_dst->acm_en = acm_src->acm_en;
		if (acm_src->param_mode == 0) {
			if (set_acm_normal_param(acm_dst, pinfo)) {
				HISI_FB_ERR("fb%d, failed to set acm normal mode parameters\n", hisifd->index);
				goto err_ret;
			}
		} else if (acm_src->param_mode == 1) {
			if (set_acm_cinema_param(acm_dst, pinfo)) {
				HISI_FB_ERR("fb%d, failed to set acm cinema mode parameters\n", hisifd->index);
				goto err_ret;
			}
		} else if (acm_src->param_mode == 2) {
			if (set_acm_user_param(acm_dst, acm_src)) {
				HISI_FB_ERR("fb%d, failed to set acm user mode parameters\n", hisifd->index);
				goto err_ret;
			}
		} else {
			HISI_FB_ERR("fb%d, invalid acm para mode!\n", hisifd->index);
			return -EINVAL;
		}

		hisifd->effect_updated_flag.acm_effect_updated = true;
	}
	return 0;

err_ret:
	free_acm_table(acm_dst);
	return -EINVAL;
}

int hisi_effect_gmp_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src){
	struct lcp_info *lcp_dst = NULL;
	struct dss_effect *effect = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return -EINVAL;
	}

	if (NULL == lcp_src) {
		HISI_FB_ERR("fb%d, lcp_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	lcp_dst = &(hisifd->effect_info.lcp);
	effect = &(hisifd->effect_ctl);

	if (!effect->lcp_gmp_support) {
		HISI_FB_INFO("fb%d, lcp gmp is not supported!\n", hisifd->index);
		return 0;
	}

	spin_lock(&g_gmp_effect_lock);

	lcp_dst->gmp_enable = lcp_src->gmp_enable;
	if (hisi_effect_alloc_and_copy(&lcp_dst->gmp_table_high4, lcp_src->gmp_table_high4,
		LCP_GMP_LUT_LENGTH, true)) {
		HISI_FB_ERR("fb%d, failed to set gmp_table_high4!\n", hisifd->index);
		goto err_ret;
	}

	if (hisi_effect_alloc_and_copy(&lcp_dst->gmp_table_low32, lcp_src->gmp_table_low32,
		LCP_GMP_LUT_LENGTH, true)) {
		HISI_FB_ERR("fb%d, failed to set gmp_lut_table_low32bit!\n", hisifd->index);
		goto err_ret;
	}

	hisifd->effect_updated_flag.gmp_effect_updated = true;

	spin_unlock(&g_gmp_effect_lock);
	return 0;

err_ret:
	hisi_effect_kfree(&lcp_dst->gmp_table_high4);
	hisi_effect_kfree(&lcp_dst->gmp_table_low32);

	spin_unlock(&g_gmp_effect_lock);
	return -EINVAL;
}

int hisi_effect_igm_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src){
	struct lcp_info *lcp_dst = NULL;
	struct dss_effect *effect = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return -EINVAL;
	}

	if (NULL == lcp_src) {
		HISI_FB_ERR("fb%d, lcp_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	lcp_dst = &(hisifd->effect_info.lcp);
	effect = &(hisifd->effect_ctl);

	if (!effect->lcp_igm_support) {
		HISI_FB_INFO("fb%d, lcp degamma is not supported!\n", hisifd->index);
		return 0;
	}

	spin_lock(&g_igm_effect_lock);

	lcp_dst->igm_enable = lcp_src->igm_enable;

	if (hisi_effect_alloc_and_copy(&lcp_dst->igm_r_table, lcp_src->igm_r_table,
		IGM_LUT_LEN, true)) {
		HISI_FB_ERR("fb%d, failed to set igm_r_table!\n", hisifd->index);
		goto err_ret;
	}

	if (hisi_effect_alloc_and_copy(&lcp_dst->igm_g_table, lcp_src->igm_g_table,
		IGM_LUT_LEN, true)) {
		HISI_FB_ERR("fb%d, failed to set igm_g_table!\n", hisifd->index);
		goto err_ret;
	}

	if (hisi_effect_alloc_and_copy(&lcp_dst->igm_b_table, lcp_src->igm_b_table,
		IGM_LUT_LEN, true)) {
		HISI_FB_ERR("fb%d, failed to set igm_b_table!\n", hisifd->index);
		goto err_ret;
	}

	hisifd->effect_updated_flag.igm_effect_updated = true;

	spin_unlock(&g_igm_effect_lock);
	return 0;

err_ret:
	hisi_effect_kfree(&lcp_dst->igm_r_table);
	hisi_effect_kfree(&lcp_dst->igm_g_table);
	hisi_effect_kfree(&lcp_dst->igm_b_table);

	spin_unlock(&g_igm_effect_lock);
	return -EINVAL;
}

int hisi_effect_xcc_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src){
	struct lcp_info *lcp_dst = NULL;
	struct dss_effect *effect = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return -EINVAL;
	}

	if (NULL == lcp_src) {
		HISI_FB_ERR("fb%d, lcp_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	lcp_dst = &(hisifd->effect_info.lcp);
	effect = &(hisifd->effect_ctl);

	if (!effect->lcp_xcc_support) {
		HISI_FB_INFO("fb%d, lcp xcc are not supported!\n", hisifd->index);
		return 0;
	}

	spin_lock(&g_xcc_effect_lock);

	lcp_dst->xcc_enable = lcp_src->xcc_enable;

	if (hisi_effect_alloc_and_copy(&lcp_dst->xcc_table, lcp_src->xcc_table,
		LCP_XCC_LUT_LENGTH, true)) {
		HISI_FB_ERR("fb%d, failed to set xcc_table!\n", hisifd->index);
		goto err_ret;
	}

	hisifd->effect_updated_flag.xcc_effect_updated = true;

	spin_unlock(&g_xcc_effect_lock);
	return 0;

err_ret:
	hisi_effect_kfree(&lcp_dst->xcc_table);

	spin_unlock(&g_xcc_effect_lock);
	return -EINVAL;
}

static int hisi_efffect_gamma_param_set(struct gamma_info *gammaDst, struct gamma_info *gammaSrc,
                                  struct hisi_panel_info* pInfo) {

	if((gammaDst == NULL) || (gammaSrc == NULL) || (pInfo == NULL)){
		HISI_FB_ERR("gammaDst or gammaSrc or pInfo is null pointer\n");
		return -1;
	}

	if (gammaSrc->para_mode == 0) {
		//Normal mode
		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_r_table, pInfo->gamma_lut_table_R,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_r_table!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_g_table, pInfo->gamma_lut_table_G,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_g_table!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_b_table, pInfo->gamma_lut_table_B,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_b_table!\n");
			goto err_ret;
		}
	} else if (gammaSrc->para_mode == 1) {
		//Cinema mode
		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_r_table, pInfo->cinema_gamma_lut_table_R,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_r_table!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_g_table, pInfo->cinema_gamma_lut_table_G,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_g_table!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_b_table, pInfo->cinema_gamma_lut_table_B,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_b_table!\n");
			goto err_ret;
		}
	} else if (gammaSrc->para_mode == 2) {
		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_r_table, gammaSrc->gamma_r_table,
			GAMMA_LUT_LEN, true)) {
			HISI_FB_ERR("failed to copy gamma_r_table from user!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_g_table, gammaSrc->gamma_g_table,
			GAMMA_LUT_LEN, true)) {
			HISI_FB_ERR("failed to copy gamma_g_table from user!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_b_table, gammaSrc->gamma_b_table,
			GAMMA_LUT_LEN, true)) {
			HISI_FB_ERR("failed to copy gamma_b_table from user!\n");
			goto err_ret;
		}
	} else {
		HISI_FB_ERR("not supported gamma para_mode!\n");
		return -EINVAL;
	}

	return 0;

err_ret:
	free_gamma_table(gammaDst);

	return -EINVAL;
}

int hisi_effect_gamma_info_set(struct hisi_fb_data_type *hisifd, struct gamma_info *gamma_src)
{
	struct gamma_info *gamma_dst = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return -EINVAL;
	}

	if (NULL == gamma_src) {
		HISI_FB_ERR("fb%d, gamma_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	gamma_dst = &(hisifd->effect_info.gamma);
	pinfo = &(hisifd->panel_info);

	if (!hisifd->effect_ctl.gamma_support) {
		HISI_FB_INFO("fb%d, gamma is not supported!\n", hisifd->index);
		return 0;
	}

	spin_lock(&g_gamma_effect_lock);

	gamma_dst->enable = gamma_src->enable;
	gamma_dst->para_mode = gamma_src->para_mode;
	ret = hisi_efffect_gamma_param_set(gamma_dst, gamma_src, pinfo);
	if (ret < 0) {
		HISI_FB_ERR("fb%d, failed to set gamma table!\n", hisifd->index);
		spin_unlock(&g_gamma_effect_lock);
		return ret;
	}

	hisifd->effect_updated_flag.gamma_effect_updated = true;

	spin_unlock(&g_gamma_effect_lock);

	return 0;
}

void hisi_effect_acm_set_reg(struct hisi_fb_data_type *hisifd)
{
	struct acm_info *acm_param = NULL;
	char __iomem *acm_base = NULL;
	char __iomem *acm_lut_base = NULL;
	static uint32_t acm_config_flag = 0;
	uint32_t acm_lut_sel;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!");
		return;
	}

	if (!hisifd->effect_ctl.acm_support) {
		return;
	}

	if (!hisifd->effect_updated_flag.acm_effect_updated) {
		return;
	}

	acm_base = hisifd->dss_base + DSS_DPP_ACM_OFFSET;
	acm_lut_base = hisifd->dss_base + DSS_DPP_ACM_LUT_OFFSET;

	if (acm_config_flag == 0) {
		//Disable ACM
		set_reg(acm_base + ACM_EN, 0x0, 1, 0);
		acm_config_flag = 1;
		return;
	}

	acm_param = &(hisifd->effect_info.acm);

	if (NULL == acm_param->hue_table) {
		HISI_FB_ERR("fb%d, invalid acm hue table param!\n", hisifd->index);
		goto err_ret;
	}

	if (NULL == acm_param->sata_table) {
		HISI_FB_ERR("fb%d, invalid acm sata table param!\n", hisifd->index);
		goto err_ret;
	}

	if ((NULL == acm_param->satr0_table) || (NULL == acm_param->satr1_table)
		|| (NULL == acm_param->satr2_table) || (NULL == acm_param->satr3_table)
		|| (NULL == acm_param->satr4_table) || (NULL == acm_param->satr5_table)
		|| (NULL == acm_param->satr6_table) || (NULL == acm_param->satr7_table)) {
		HISI_FB_ERR("fb%d, invalid acm satr table param!\n", hisifd->index);
		goto err_ret;
	}

	set_reg(acm_base + ACM_SATA_OFFSET, acm_param->sata_offset, 6, 0);
	set_reg(acm_base + ACM_HUE_RLH01, acm_param->acm_hue_rlh01, 32, 0);
	set_reg(acm_base + ACM_HUE_RLH23, acm_param->acm_hue_rlh23, 32, 0);
	set_reg(acm_base + ACM_HUE_RLH45, acm_param->acm_hue_rlh45, 32, 0);
	set_reg(acm_base + ACM_HUE_RLH67, acm_param->acm_hue_rlh67, 32, 0);
	set_reg(acm_base + ACM_HUE_PARAM01, acm_param->acm_hue_param01, 32,0);
	set_reg(acm_base + ACM_HUE_PARAM23, acm_param->acm_hue_param23, 32,0);
	set_reg(acm_base + ACM_HUE_PARAM45, acm_param->acm_hue_param45, 32,0);
	set_reg(acm_base + ACM_HUE_PARAM67, acm_param->acm_hue_param67, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH0, acm_param->acm_hue_smooth0, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH1, acm_param->acm_hue_smooth1, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH2, acm_param->acm_hue_smooth2, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH3, acm_param->acm_hue_smooth3, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH4, acm_param->acm_hue_smooth4, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH5, acm_param->acm_hue_smooth5, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH6, acm_param->acm_hue_smooth6, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH7, acm_param->acm_hue_smooth7, 32,0);
	set_reg(acm_base + ACM_COLOR_CHOOSE,acm_param->acm_color_choose,1,0);
	set_reg(acm_base + ACM_L_CONT_EN, acm_param->acm_l_cont_en, 1,0);
	set_reg(acm_base + ACM_LC_PARAM01, acm_param->acm_lc_param01, 32,0);
	set_reg(acm_base + ACM_LC_PARAM23, acm_param->acm_lc_param23, 32,0);
	set_reg(acm_base + ACM_LC_PARAM45, acm_param->acm_lc_param45, 32,0);
	set_reg(acm_base + ACM_LC_PARAM67, acm_param->acm_lc_param67, 32,0);
	set_reg(acm_base + ACM_L_ADJ_CTRL, acm_param->acm_l_adj_ctrl, 32,0);
	set_reg(acm_base + ACM_CAPTURE_CTRL, acm_param->acm_capture_ctrl,32,0);
	set_reg(acm_base + ACM_INK_CTRL, acm_param->acm_ink_ctrl, 32,0);
	set_reg(acm_base + ACM_INK_OUT, acm_param->acm_ink_out, 30,0);

	acm_set_lut_hue(acm_lut_base + ACM_U_H_COEF, acm_param->hue_table, ACM_HUE_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATA_COEF, acm_param->sata_table, ACM_SATA_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR0_COEF, acm_param->satr0_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR1_COEF, acm_param->satr1_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR2_COEF, acm_param->satr2_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR3_COEF, acm_param->satr3_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR4_COEF, acm_param->satr4_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR5_COEF, acm_param->satr5_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR6_COEF, acm_param->satr6_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR7_COEF, acm_param->satr7_table, ACM_SATR_LUT_LENGTH);

	acm_lut_sel = (uint32_t)inp32(acm_base + ACM_LUT_SEL);
	set_reg(acm_base + ACM_LUT_SEL, (~(acm_lut_sel & 0x380)) & (acm_lut_sel | 0x380), 16, 0);

	set_reg(acm_base + ACM_EN, acm_param->acm_en, 1, 0);
err_ret:
	hisifd->effect_updated_flag.acm_effect_updated = false;

	//free_acm_table(acm_param);
	acm_config_flag = 0;
	return;
}

#define XCC_COEF_LEN	12
#define GMP_BLOCK_SIZE	137
#define GMP_CNT_NUM 18
#define GMP_COFE_CNT 729

static bool lcp_igm_set_reg(char __iomem *degamma_lut_base, struct lcp_info *lcp_param)
{
	int cnt;

	if (degamma_lut_base == NULL) {
		HISI_FB_ERR("lcp_lut_base is NULL!\n");
		return false;
	}

	if (lcp_param == NULL) {
		HISI_FB_ERR("lcp_param is NULL!\n");
		return false;
	}

	if (lcp_param->igm_r_table == NULL || lcp_param->igm_g_table == NULL || lcp_param->igm_b_table == NULL) {
		HISI_FB_ERR("igm_r_table or igm_g_table or igm_b_table	is NULL!\n");
		return false;
	}

	for (cnt = 0; cnt < IGM_LUT_LEN; cnt = cnt + 2) {
		set_reg(degamma_lut_base + (U_DEGAMA_R_COEF + cnt * 2), lcp_param->igm_r_table[cnt], 12,0);
		if(cnt != IGM_LUT_LEN-1)
			set_reg(degamma_lut_base + (U_DEGAMA_R_COEF + cnt * 2), lcp_param->igm_r_table[cnt+1], 12,16);

		set_reg(degamma_lut_base + (U_DEGAMA_G_COEF + cnt * 2), lcp_param->igm_g_table[cnt], 12,0);
		if(cnt != IGM_LUT_LEN-1)
			set_reg(degamma_lut_base + (U_DEGAMA_G_COEF + cnt * 2), lcp_param->igm_g_table[cnt+1], 12,16);

		set_reg(degamma_lut_base + (U_DEGAMA_B_COEF + cnt * 2), lcp_param->igm_b_table[cnt], 12,0);
		if(cnt != IGM_LUT_LEN-1)
			set_reg(degamma_lut_base + (U_DEGAMA_B_COEF + cnt * 2), lcp_param->igm_b_table[cnt+1], 12,16);
	}
	return true;
}

static bool lcp_xcc_set_reg(char __iomem *xcc_base, struct lcp_info *lcp_param)
{
	int cnt;

	if (xcc_base == NULL) {
		HISI_FB_DEBUG("lcp_base is NULL!\n");
		return false;
	}

	if (lcp_param == NULL) {
		HISI_FB_DEBUG("lcp_param is NULL!\n");
		return false;
	}

	if (lcp_param->xcc_table == NULL) {
		HISI_FB_DEBUG("xcc_table is NULL!\n");
		return false;
	}
	for (cnt = 0; cnt < XCC_COEF_LEN; cnt++) {
		set_reg(xcc_base + LCP_XCC_COEF_00 +cnt * 4,  lcp_param->xcc_table[cnt], 17, 0);
	}
	return true;
}

static bool lcp_gmp_set_reg(char __iomem *gmp_lut_base, struct lcp_info *lcp_param)
{
	int i;

	if (gmp_lut_base == NULL) {
		HISI_FB_DEBUG("lcp_lut_base is NULL!\n");
		return false;
	}

	if (lcp_param == NULL) {
		HISI_FB_DEBUG("lcp_param is NULL!\n");
		return false;
	}
	if (lcp_param->gmp_table_low32 == NULL || lcp_param->gmp_table_high4 == NULL) {
		HISI_FB_DEBUG("gmp_table_low32 or gmp_table_high4 is NULL!\n");
		return false;
	}
	for (i = 0; i < GMP_COFE_CNT; i++) {
		set_reg(gmp_lut_base + i * 2 * 4, lcp_param->gmp_table_low32[i], 32, 0);
		set_reg(gmp_lut_base + i * 2 * 4 + 4, lcp_param->gmp_table_high4[i], 4, 0);
	}
	return true;
}

void hisi_effect_lcp_set_reg(struct hisi_fb_data_type *hisifd)
{
	struct dss_effect *effect = NULL;
	struct lcp_info *lcp_param = NULL;
	char __iomem *xcc_base = NULL;
	char __iomem *gmp_base = NULL;
	char __iomem *degamma_base = NULL;
	char __iomem *gmp_lut_base = NULL;
	char __iomem *degamma_lut_base = NULL;
	char __iomem *lcp_base = NULL;
	uint32_t degama_lut_sel;
	uint32_t gmp_lut_sel;
	bool ret;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!");
		return;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return;
	}

	effect = &hisifd->effect_ctl;

	lcp_base = hisifd->dss_base + DSS_DPP_LCP_OFFSET;
	xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
	gmp_base = hisifd->dss_base + DSS_DPP_GMP_OFFSET;
	degamma_base = hisifd->dss_base + DSS_DPP_DEGAMMA_OFFSET;
	degamma_lut_base = hisifd->dss_base + DSS_DPP_DEGAMMA_LUT_OFFSET;
	gmp_lut_base = hisifd->dss_base + DSS_DPP_GMP_LUT_OFFSET;

	lcp_param = &(hisifd->effect_info.lcp);

	//Update De-Gamma LUT
	if (effect->lcp_igm_support && hisifd->effect_updated_flag.igm_effect_updated) {
		if (spin_can_lock(&g_igm_effect_lock)) {
			spin_lock(&g_igm_effect_lock);
			ret = lcp_igm_set_reg(degamma_lut_base, lcp_param);
			//Enable De-Gamma
			if (ret) {
				degama_lut_sel = inp32(degamma_base + DEGAMA_LUT_SEL);
				set_reg(degamma_base + DEGAMA_LUT_SEL, (~(degama_lut_sel & 0x1)) & 0x1, 1, 0);
				//set_reg(degamma_base + DEGAMA_EN,  lcp_param->igm_enable, 1, 0);
			}
			hisifd->effect_updated_flag.igm_effect_updated = false;
			spin_unlock(&g_igm_effect_lock);
		} else {
			HISI_FB_INFO("igm effect param is being updated, delay set reg to next frame!\n");
		}
	}

	//Update XCC Coef
	if (effect->lcp_xcc_support && hisifd->effect_updated_flag.xcc_effect_updated) {
		if (spin_can_lock(&g_xcc_effect_lock)) {
			spin_lock(&g_xcc_effect_lock);
			ret = lcp_xcc_set_reg(xcc_base, lcp_param);
			//Enable XCC
			if (ret) {
				set_reg(xcc_base + LCP_XCC_BYPASS_EN,	~(lcp_param->xcc_enable), 1, 0);
				//set_reg(xcc_base + LCP_XCC_BYPASS_EN,	lcp_param->xcc_enable, 1, 0);
				//Enable XCC pre
				//set_reg(xcc_base + XCC_EN,  lcp_param->xcc_enable, 1, 1);
			}
			hisifd->effect_updated_flag.xcc_effect_updated = false;
			spin_unlock(&g_xcc_effect_lock);
		} else {
			HISI_FB_INFO("xcc effect param is being updated, delay set reg to next frame!\n");
		}
	}

	//Update GMP LUT
	if (effect->lcp_gmp_support && hisifd->effect_updated_flag.gmp_effect_updated) {
		if (spin_can_lock(&g_gmp_effect_lock)) {
			spin_lock(&g_gmp_effect_lock);
			ret = lcp_gmp_set_reg(gmp_lut_base, lcp_param);
			//Enable GMP
			if (ret) {
				gmp_lut_sel = inp32(gmp_base + GMP_LUT_SEL);
				set_reg(gmp_base + GMP_LUT_SEL, (~(gmp_lut_sel & 0x1)) & 0x1, 1, 0);
				set_reg(gmp_base + GMP_EN, lcp_param->gmp_enable, 1, 0);
			}
			hisifd->effect_updated_flag.gmp_effect_updated = false;
			spin_unlock(&g_gmp_effect_lock);
		} else {
			HISI_FB_INFO("gmp effect param is being updated, delay set reg to next frame!\n");
		}
	}

	//free_lcp_table(lcp_param);
	return;
}

void hisi_effect_gamma_set_reg(struct hisi_fb_data_type *hisifd)
{
	struct gamma_info *gamma_param = NULL;
	char __iomem *gamma_base = NULL;
	char __iomem *gamma_lut_base = NULL;
	char __iomem *gamma_pre_lut_base = NULL;
	int cnt = 0;
	uint32_t gama_lut_sel;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!");
		return;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return;
	}

	if (!hisifd->effect_ctl.gamma_support) {
		return;
	}

	if (!hisifd->effect_updated_flag.gamma_effect_updated) {
		return;
	}

	gamma_base = hisifd->dss_base + DSS_DPP_GAMA_OFFSET;
	gamma_lut_base = hisifd->dss_base + DSS_DPP_GAMA_LUT_OFFSET;
	gamma_pre_lut_base = hisifd->dss_base + DSS_DPP_GAMA_PRE_LUT_OFFSET;

	gamma_param = &(hisifd->effect_info.gamma);

	if (!spin_can_lock(&g_gamma_effect_lock)) {
	    HISI_FB_INFO("gamma effect param is being updated, delay set reg to next frame!\n");
	    return;
	}
	spin_lock(&g_gamma_effect_lock);

	if ((NULL == gamma_param->gamma_r_table) ||
		(NULL == gamma_param->gamma_g_table) ||
		(NULL == gamma_param->gamma_b_table)) {
		HISI_FB_ERR("fb%d, gamma table is null!\n", hisifd->index);
		goto err_ret;
	}

	//Update Gamma LUT
	for (cnt = 0; cnt < GAMMA_LUT_LEN; cnt = cnt + 2) {
		set_reg(gamma_lut_base + (U_GAMA_R_COEF + cnt * 2), gamma_param->gamma_r_table[cnt], 12,0);
		if (cnt != GAMMA_LUT_LEN - 1)
			set_reg(gamma_lut_base + (U_GAMA_R_COEF + cnt * 2), gamma_param->gamma_r_table[cnt+1], 12,16);

		set_reg(gamma_lut_base + (U_GAMA_G_COEF + cnt * 2), gamma_param->gamma_g_table[cnt], 12,0);
		if (cnt != GAMMA_LUT_LEN - 1)
			set_reg(gamma_lut_base + (U_GAMA_G_COEF + cnt * 2), gamma_param->gamma_g_table[cnt+1], 12,16);

		set_reg(gamma_lut_base + (U_GAMA_B_COEF + cnt * 2), gamma_param->gamma_b_table[cnt], 12,0);
		if (cnt != GAMMA_LUT_LEN - 1)
			set_reg(gamma_lut_base + (U_GAMA_B_COEF + cnt * 2), gamma_param->gamma_b_table[cnt+1], 12,16);
	}
	gama_lut_sel = inp32(gamma_base + GAMA_LUT_SEL);
	set_reg(gamma_base + GAMA_LUT_SEL, (~(gama_lut_sel & 0x1)) & 0x1, 1, 0);

	//Enable Gamma
	set_reg(gamma_base + GAMA_EN,  gamma_param->enable, 1, 0);

err_ret:
	hisifd->effect_updated_flag.gamma_effect_updated = false;
	//free_gamma_table(gamma_param);

	spin_unlock(&g_gamma_effect_lock);

	return;
}

/*******************************************************************************
**ACM REG DIMMING
*/
/*lint -e866*/
static inline uint32_t cal_hue_value(uint32_t cur,uint32_t target,int count) {
	if (count <= 0) {
		return cur;
	}

	if (abs((int)(cur - target)) > HUE_REG_RANGE) {
		if (cur > target) {
			target += HUE_REG_OFFSET;
		} else {
			cur += HUE_REG_OFFSET;
		}
	}

	if (target > cur) {
		cur += (target - cur) / (uint32_t)count;
	} else if (cur > target) {
		cur -= (cur - target) / (uint32_t)count;
	}

	cur %= HUE_REG_OFFSET;

	return cur;
}

static inline uint32_t cal_sata_value(uint32_t cur,uint32_t target,int count) {
	if (count <= 0) {
		return cur;
	}

	if (target > cur) {
		cur += (target - cur) / (uint32_t)count;
	} else if (cur > target) {
		cur -= (cur - target) / (uint32_t)count;
	}

	return cur;
}

/*lint -e838*/
static inline int satr_unint32_to_int(uint32_t input) {
	int result = 0;

	if (input >= SATR_REG_RANGE) {
		result = (int)input - SATR_REG_OFFSET;
	} else {
		result = (int)input;
	}
	return result;
}

/*lint -e838*/
static inline uint32_t cal_satr_value(uint32_t cur,uint32_t target,int count) {
	int i_cur = 0;
	int i_target = 0;

	if (count <= 0) {
		return cur;
	}

	i_cur = satr_unint32_to_int(cur);
	i_target = satr_unint32_to_int(target);

	return (uint32_t)(i_cur + (i_target - i_cur) / count) % SATR_REG_OFFSET;
}

/*lint -e838*/
static inline void cal_cur_acm_reg(int count,acm_reg_t *cur_acm_reg,acm_reg_table_t *target_acm_reg) {
	int index = 0;

	if (NULL == cur_acm_reg || NULL == target_acm_reg) {
		return;
	}

	for (index = 0;index < ACM_HUE_SIZE;index++) {
		cur_acm_reg->acm_lut_hue_table[index] = cal_hue_value(cur_acm_reg->acm_lut_hue_table[index],target_acm_reg->acm_lut_hue_table[index],count);
	}
	for (index = 0;index < ACM_SATA_SIZE;index++) {
		cur_acm_reg->acm_lut_sata_table[index] = cal_sata_value(cur_acm_reg->acm_lut_sata_table[index],target_acm_reg->acm_lut_sata_table[index],count);
	}
	for (index = 0;index < ACM_SATR_SIZE;index++) {
		cur_acm_reg->acm_lut_satr0_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr0_table[index],target_acm_reg->acm_lut_satr0_table[index],count);
		cur_acm_reg->acm_lut_satr1_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr1_table[index],target_acm_reg->acm_lut_satr1_table[index],count);
		cur_acm_reg->acm_lut_satr2_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr2_table[index],target_acm_reg->acm_lut_satr2_table[index],count);
		cur_acm_reg->acm_lut_satr3_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr3_table[index],target_acm_reg->acm_lut_satr3_table[index],count);
		cur_acm_reg->acm_lut_satr4_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr4_table[index],target_acm_reg->acm_lut_satr4_table[index],count);
		cur_acm_reg->acm_lut_satr5_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr5_table[index],target_acm_reg->acm_lut_satr5_table[index],count);
		cur_acm_reg->acm_lut_satr6_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr6_table[index],target_acm_reg->acm_lut_satr6_table[index],count);
		cur_acm_reg->acm_lut_satr7_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr7_table[index],target_acm_reg->acm_lut_satr7_table[index],count);
	}
}

/*lint -e838*/
void hisi_effect_color_dimming_acm_reg_set(struct hisi_fb_data_type *hisifd, acm_reg_t *cur_acm_reg) {
	acm_reg_table_t target_acm_reg = {0};
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!");
		return;
	}

	if (NULL == cur_acm_reg) {
		HISI_FB_ERR("cur_acm_reg is NULL!");
		return;
	}

	pinfo = &(hisifd->panel_info);

	if (SCENE_MODE_GALLERY == hisifd->user_scene_mode || SCENE_MODE_DEFAULT == hisifd->user_scene_mode) {
		target_acm_reg.acm_lut_hue_table = pinfo->acm_lut_hue_table;
		target_acm_reg.acm_lut_sata_table = pinfo->acm_lut_sata_table;
		target_acm_reg.acm_lut_satr0_table = pinfo->acm_lut_satr0_table;
		target_acm_reg.acm_lut_satr1_table = pinfo->acm_lut_satr1_table;
		target_acm_reg.acm_lut_satr2_table = pinfo->acm_lut_satr2_table;
		target_acm_reg.acm_lut_satr3_table = pinfo->acm_lut_satr3_table;
		target_acm_reg.acm_lut_satr4_table = pinfo->acm_lut_satr4_table;
		target_acm_reg.acm_lut_satr5_table = pinfo->acm_lut_satr5_table;
		target_acm_reg.acm_lut_satr6_table = pinfo->acm_lut_satr6_table;
		target_acm_reg.acm_lut_satr7_table = pinfo->acm_lut_satr7_table;
	} else {
		target_acm_reg.acm_lut_hue_table = pinfo->video_acm_lut_hue_table;
		target_acm_reg.acm_lut_sata_table = pinfo->video_acm_lut_sata_table;
		target_acm_reg.acm_lut_satr0_table = pinfo->video_acm_lut_satr0_table;
		target_acm_reg.acm_lut_satr1_table = pinfo->video_acm_lut_satr1_table;
		target_acm_reg.acm_lut_satr2_table = pinfo->video_acm_lut_satr2_table;
		target_acm_reg.acm_lut_satr3_table = pinfo->video_acm_lut_satr3_table;
		target_acm_reg.acm_lut_satr4_table = pinfo->video_acm_lut_satr4_table;
		target_acm_reg.acm_lut_satr5_table = pinfo->video_acm_lut_satr5_table;
		target_acm_reg.acm_lut_satr6_table = pinfo->video_acm_lut_satr6_table;
		target_acm_reg.acm_lut_satr7_table = pinfo->video_acm_lut_satr7_table;
	}

	cal_cur_acm_reg(hisifd->dimming_count,cur_acm_reg,&target_acm_reg);
}

/*lint -e838*/
void hisi_effect_color_dimming_acm_reg_init(struct hisi_fb_data_type *hisifd) {//add dimming reg init
	struct hisi_panel_info *pinfo = NULL;
	acm_reg_t *cur_acm_reg = NULL;
	int index = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd, null pointer warning.\n");
		return;
	}

	pinfo = &(hisifd->panel_info);

	cur_acm_reg = &(hisifd->acm_reg);

	if (SCENE_MODE_GALLERY == hisifd->user_scene_mode || SCENE_MODE_DEFAULT == hisifd->user_scene_mode) {
		for (index = 0;index < ACM_HUE_SIZE;index++) {
			cur_acm_reg->acm_lut_hue_table[index] = pinfo->acm_lut_hue_table[index];
		}
		for (index = 0;index < ACM_SATA_SIZE;index++) {
			cur_acm_reg->acm_lut_sata_table[index] = pinfo->acm_lut_sata_table[index];
		}
		for (index = 0;index < ACM_SATR_SIZE;index++) {
			cur_acm_reg->acm_lut_satr0_table[index] = pinfo->acm_lut_satr0_table[index];
			cur_acm_reg->acm_lut_satr1_table[index] = pinfo->acm_lut_satr1_table[index];
			cur_acm_reg->acm_lut_satr2_table[index] = pinfo->acm_lut_satr2_table[index];
			cur_acm_reg->acm_lut_satr3_table[index] = pinfo->acm_lut_satr3_table[index];
			cur_acm_reg->acm_lut_satr4_table[index] = pinfo->acm_lut_satr4_table[index];
			cur_acm_reg->acm_lut_satr5_table[index] = pinfo->acm_lut_satr5_table[index];
			cur_acm_reg->acm_lut_satr6_table[index] = pinfo->acm_lut_satr6_table[index];
			cur_acm_reg->acm_lut_satr7_table[index] = pinfo->acm_lut_satr7_table[index];
		}
	} else {
		for (index = 0;index < ACM_HUE_SIZE;index++) {
			cur_acm_reg->acm_lut_hue_table[index] = pinfo->video_acm_lut_hue_table[index];
		}
		for (index = 0;index < ACM_SATA_SIZE;index++) {
			cur_acm_reg->acm_lut_sata_table[index] = pinfo->video_acm_lut_sata_table[index];
		}
		for (index = 0;index < ACM_SATR_SIZE;index++) {
			cur_acm_reg->acm_lut_satr0_table[index] = pinfo->video_acm_lut_satr0_table[index];
			cur_acm_reg->acm_lut_satr1_table[index] = pinfo->video_acm_lut_satr1_table[index];
			cur_acm_reg->acm_lut_satr2_table[index] = pinfo->video_acm_lut_satr2_table[index];
			cur_acm_reg->acm_lut_satr3_table[index] = pinfo->video_acm_lut_satr3_table[index];
			cur_acm_reg->acm_lut_satr4_table[index] = pinfo->video_acm_lut_satr4_table[index];
			cur_acm_reg->acm_lut_satr5_table[index] = pinfo->video_acm_lut_satr5_table[index];
			cur_acm_reg->acm_lut_satr6_table[index] = pinfo->video_acm_lut_satr6_table[index];
			cur_acm_reg->acm_lut_satr7_table[index] = pinfo->video_acm_lut_satr7_table[index];
		}
	}
}

static void hisi_effect_arsr2p_update(struct hisi_fb_data_type *hisifd, struct dss_arsr2p *arsr2p) {
	if (hisifd == NULL || arsr2p == NULL) {
		HISI_FB_ERR("hisifd or arsr2p is null. return\n");
		return;
	}

	if (hisifd->effect_info.arsr2p[0].update == 1) {
		memcpy(&(arsr2p->arsr2p_effect), &(hisifd->effect_info.arsr2p[0]), sizeof(struct arsr2p_info));
		hisifd->effect_info.arsr2p[0].update = 0;
	}

	if (hisifd->effect_info.arsr2p[1].update == 1) {
		memcpy(&(arsr2p->arsr2p_effect_scale_up), &(hisifd->effect_info.arsr2p[1]), sizeof(struct arsr2p_info));
		hisifd->effect_info.arsr2p[1].update = 0;
	}

	if (hisifd->effect_info.arsr2p[2].update == 1) {
		memcpy(&(arsr2p->arsr2p_effect_scale_down), &(hisifd->effect_info.arsr2p[2]), sizeof(struct arsr2p_info));
		hisifd->effect_info.arsr2p[2].update = 0;
	}
}


int hisi_effect_arsr2p_config(struct arsr2p_info *arsr2p_effect_dst, int ih_inc, int iv_inc)
{
	struct hisi_fb_data_type *hisifd_primary = NULL;
	struct dss_arsr2p *arsr2p = NULL;

	hisifd_primary = hisifd_list[PRIMARY_PANEL_IDX];
	if (NULL == hisifd_primary) {
		HISI_FB_ERR("hisifd_primary is NULL pointer, return!\n");
		return -EINVAL;
	}

	if (NULL == arsr2p_effect_dst) {
		HISI_FB_ERR("arsr2p_effect_dst is NULL pointer, return!\n");
		return -EINVAL;
	}

	arsr2p = &(hisifd_primary->dss_module_default.arsr2p[DSS_RCHN_V1]);
	hisi_effect_arsr2p_update(hisifd_primary, arsr2p);

	if ((ih_inc == ARSR2P_INC_FACTOR) && (iv_inc == ARSR2P_INC_FACTOR)) {
		memcpy(arsr2p_effect_dst, &(arsr2p->arsr2p_effect), sizeof(struct arsr2p_info));
	} else if ((ih_inc < ARSR2P_INC_FACTOR) || (iv_inc < ARSR2P_INC_FACTOR)) {
		memcpy(arsr2p_effect_dst, &(arsr2p->arsr2p_effect_scale_up), sizeof(struct arsr2p_info));
	} else {
		memcpy(arsr2p_effect_dst, &(arsr2p->arsr2p_effect_scale_down), sizeof(struct arsr2p_info));
	}

	return 0;
}
int hisifb_ce_service_enable_hiace(struct fb_info *info, const void __user *argp)
{
	struct hisi_panel_info *pinfo = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;
	dss_ce_info_t *ce_info = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_metadata_t *metadata_ctrl = NULL;
	int enable;
	int ret;
	int mode;
	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}
	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}
	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] HIACE is not supported!\n");
		return -1;
	}
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ce_ctrl = &(hisifd->ce_ctrl);
		ce_info = &(hisifd->hiace_info);
		metadata_ctrl = &(hisifd->metadata_ctrl);
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return -1;
	}
	ret = (int)copy_from_user(&enable, argp, sizeof(enable));
	if (ret) {
		HISI_FB_ERR("[effect] arg is invalid");
		return -EINVAL;
	}
	mode = enable;
	if (mode < 0) {
		mode = 0;
	} else if (mode >= CE_MODE_COUNT) {
		mode = CE_MODE_COUNT - 1;
	}
	if (mode != ce_ctrl->ctrl_ce_mode) {
		mutex_lock(&(ce_ctrl->ctrl_lock));
		ce_ctrl->ctrl_ce_mode = mode;
		mutex_unlock(&(ce_ctrl->ctrl_lock));
		if (mode == CE_MODE_DISABLE) {
			ce_info->gradual_frames = EFFECT_GRADUAL_REFRESH_FRAMES;
			ce_info->to_stop_hdr = true;
		}
		enable_hiace(hisifd, mode);
	}
	return 0;
}
int hisifb_get_reg_val(struct fb_info *info, void __user *argp) {
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dss_reg reg;
	uint32_t addr;
	int ret = 0;
	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}
	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}
	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (!pinfo->hiace_support) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] Don't support HIACE\n");
		return -EINVAL;
	}
	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] step in\n");
	ret = (int)copy_from_user(&reg, argp, sizeof(struct dss_reg));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		return -2;
	}

	switch(reg.tag) {
		//case TAG_ARSR_1P_ENABLE:
		//	addr = DSS_POST_SCF_OFFSET + ARSR_POST_MODE;
		//	break;
		case TAG_LCP_XCC_ENABLE:
			addr = DSS_DPP_XCC_OFFSET + LCP_XCC_BYPASS_EN;
			break;
		case TAG_LCP_GMP_ENABLE:
			addr = DSS_DPP_GMP_OFFSET + GMP_EN;
			break;
		case TAG_LCP_IGM_ENABLE:
			addr = DSS_DPP_DEGAMMA_OFFSET + DEGAMA_EN;
			break;
		case TAG_GAMMA_ENABLE:
			addr = DSS_DPP_GAMA_OFFSET + GAMA_EN;
			break;
		case TAG_HIACE_LHIST_SFT:
			addr = DSS_HI_ACE_OFFSET + DPE_LHIST_SFT;
			break;
		default:
			HISI_FB_ERR("[effect] invalid tag : %u", reg.tag);
			return -EINVAL;
	}

	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on == false) {
		HISI_FB_ERR("[effect] panel power off\n");
		up(&hisifd->blank_sem);
		return -EINVAL;
	}
	hisifb_activate_vsync(hisifd);

	reg.value = (uint32_t)inp32(hisifd->dss_base + addr);

	hisifb_deactivate_vsync(hisifd);
	up(&hisifd->blank_sem);

	ret = (int)copy_to_user(argp, &reg, sizeof(struct dss_reg));
	if (ret) {
		HISI_FB_ERR("[effect] copy_to_user failed(param)! ret=%d.\n", ret);
		ret = -EINVAL;
	}
	return 0;
}

int hisifb_ce_service_set_param(struct fb_info *info, const void __user *argp) {
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret = 0;
	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}
	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}
	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo->hiace_support) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] Don't support HIACE\n");
		return -EINVAL;
	}
	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] step in\n");
	ret = (int)copy_from_user(&(hisifd->effect_info.hiace), argp, sizeof(struct hiace_info));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		return -2;
	}

	hisifd->effect_updated_flag.hiace_effect_updated = true;
	return ret;
}

static int set_hiace_param(struct hisi_fb_data_type *hisifd) {
	char __iomem *hiace_base = NULL;
	if (hisifd == NULL) {
		HISI_FB_DEBUG("hisifd is NULL!\n");
		return -1;
	}
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_DEBUG("[effect] fb%d, not support!", hisifd->index);
		return 0;
	}

	if(!check_underflow_clear_ack(hisifd)){
		HISI_FB_ERR("[effect] fb%d, clear_ack is not return!\n", hisifd->index);
		return -1;
	}

	if (hisifd->effect_updated_flag.hiace_effect_updated) {
		set_reg(hiace_base + HIACE_HALF_BLOCK_H_W, hisifd->effect_info.hiace.half_block_info, 32, 0);
		set_reg(hiace_base + HIACE_XYWEIGHT, hisifd->effect_info.hiace.xyweight, 32, 0);
		set_reg(hiace_base + HIACE_LHIST_SFT, hisifd->effect_info.hiace.lhist_sft, 32, 0);
		set_reg(hiace_base + HIACE_HUE, hisifd->effect_info.hiace.hue, 32, 0);
		set_reg(hiace_base + HIACE_SATURATION, hisifd->effect_info.hiace.saturation, 32, 0);
		set_reg(hiace_base + HIACE_VALUE, hisifd->effect_info.hiace.value, 32, 0);
		set_reg(hiace_base + HIACE_SKIN_GAIN, hisifd->effect_info.hiace.skin_gain, 32, 0);
		set_reg(hiace_base + HIACE_UP_LOW_TH, hisifd->effect_info.hiace.up_low_th, 32, 0);
		set_reg(hiace_base + HIACE_UP_CNT, hisifd->effect_info.hiace.up_cnt, 32, 0);
		set_reg(hiace_base + HIACE_LOW_CNT, hisifd->effect_info.hiace.low_cnt, 32, 0);
		set_reg(hiace_base + HIACE_GAMMA_EN, hisifd->effect_info.hiace.gamma_w, 32, 0);
		set_reg(hiace_base + HIACE_GAMMA_EN_HV_R, hisifd->effect_info.hiace.gamma_r, 32, 0);
	}
	hisifd->effect_updated_flag.hiace_effect_updated = false;
	return 0;
}

int hisi_effect_hiace_config(struct hisi_fb_data_type *hisifd) {
	return set_hiace_param(hisifd);
}
/*lint +e571, +e573, +e737, +e732, +e850, +e730, +e713, +e529, +e574, +e679, +e732, +e845, +e570, +e774*/
//lint +e747, +e838
