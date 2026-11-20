/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef HISI_FB_STRUCT_H
#define HISI_FB_STRUCT_H

#include "hisi_fb_enum.h"

struct dpufb_vsync {
	wait_queue_head_t vsync_wait;
	ktime_t vsync_timestamp;
	ktime_t vsync_timestamp_prev;
	ktime_t vactive_timestamp;
	int vsync_created;
	/* flag for soft vsync signal synchronizing with TE */
	int vsync_enabled;
	int vsync_infinite;
	int vsync_infinite_count;

	int vsync_ctrl_expire_count;
	int vsync_ctrl_enabled;
	int vsync_ctrl_disabled_set;
	int vsync_ctrl_isr_enabled;
	int vsync_ctrl_offline_enabled;
	int vsync_disable_enter_idle;
	struct work_struct vsync_ctrl_work;
	spinlock_t spin_lock;

	struct mutex vsync_lock;

	atomic_t buffer_updated;
	void (*vsync_report_fnc)(int buffer_updated);

	struct dpu_fb_data_type *dpufd;
};

struct dss_comm_mmbuf_info {
	int ov_idx;
	dss_mmbuf_t mmbuf;
};

struct dpufb_secure {
	uint32_t secure_created;
	uint32_t secure_status;
	uint32_t secure_event;
	uint32_t secure_blank_flag;
	uint32_t tui_need_switch;
	uint32_t tui_need_skip_report;
	bool have_set_backlight;

	struct work_struct secure_ctrl_work;

	void (*secure_layer_config)(struct dpu_fb_data_type *dpufd, int32_t chn_idx);
	void (*secure_layer_deconfig)(struct dpu_fb_data_type *dpufd, int32_t chn_idx);
	void (*notify_secure_switch)(struct dpu_fb_data_type *dpufd);
	void (*set_reg)(uint32_t addr, uint32_t val, uint8_t bw, uint8_t bs);
#if defined(CONFIG_DP_HDCP)
	void (*hdcp13_enable)(uint32_t en);
	void (*hdcp22_enable)(uint32_t en);
	void (*hdcp13_encrypt_enable)(uint32_t en);
	void (*hdcp_dpc_sec_en)(void);
	void (*hdcp_obs_set)(uint32_t reg);
	void (*hdcp_int_clr)(uint32_t reg);
	void (*hdcp_int_mask)(uint32_t reg);
	void (*hdcp_cp_irq)(void);
	int (*hdcp_reg_get)(uint32_t addr);
	void (*hdcp_enc_mode)(uint32_t en);
#endif
	struct dpu_fb_data_type *dpufd;
};

/* mediacomm channel manager struct config */
typedef struct mdc_chn_info {
	unsigned int status;
	unsigned int drm_used;
	unsigned int cap_available;
	unsigned int rch_idx;
	unsigned int wch_idx;
	unsigned int ovl_idx;
	unsigned int wb_composer_type;
} mdc_chn_info_t;

typedef struct mdc_func_ops {
	unsigned int chan_num;
	struct semaphore mdc_req_sem;
	struct semaphore mdc_rel_sem;
	struct task_struct *refresh_handle_thread;
	wait_queue_head_t refresh_handle_wait;

	mdc_chn_info_t mdc_channel[MAX_MDC_CHANNEL];

	int (*chn_request_handle)(struct dpu_fb_data_type *dpufd, mdc_ch_info_t *chn_info);
	int (*chn_release_handle)(struct dpu_fb_data_type *dpufd, mdc_ch_info_t *chn_info);
} mdc_func_ops_t;

typedef struct hisi_fb_fix_var_screeninfo {
	uint32_t fix_type;
	uint32_t fix_xpanstep;
	uint32_t fix_ypanstep;
	uint32_t var_vmode;

	uint32_t var_blue_offset;
	uint32_t var_green_offset;
	uint32_t var_red_offset;
	uint32_t var_transp_offset;

	uint32_t var_blue_length;
	uint32_t var_green_length;
	uint32_t var_red_length;
	uint32_t var_transp_length;

	uint32_t var_blue_msb_right;
	uint32_t var_green_msb_right;
	uint32_t var_red_msb_right;
	uint32_t var_transp_msb_right;
	uint32_t bpp;
} hisi_fb_fix_var_screeninfo_t;


/*******************************************************************************/

/* esd func define */
struct dpufb_esd {
	int esd_inited;
	struct hrtimer esd_hrtimer;
	struct workqueue_struct *esd_check_wq;
	struct work_struct esd_check_work;

	struct dpu_fb_data_type *dpufd;
};

struct dpufb_pipe_clk {
	uint64_t pxl0_ppll_rate; /* need config in panel init. */
	uint64_t pipe_clk_rate;
	uint32_t pipe_clk_updt_hporch[3];
	uint32_t fps_updt_hporch[3];
	uint32_t pipe_clk_rate_div;
	uint8_t  pipe_clk_updt_state;
	uint8_t  pipe_clk_updt_times;
	uint8_t inited;
	uint8_t underflow_int;
	uint8_t dirty_region_updt_disable;
	uint8_t reserved[2];

	struct workqueue_struct *pipe_clk_handle_wq;
	struct work_struct pipe_clk_handle_work;
	struct dpu_fb_data_type *dpufd;
};

struct dpufb_buf_sync {
	char *fence_name;
	struct hisi_dss_timeline *timeline;
	struct hisi_dss_timeline *timeline_retire;

	int timeline_max;
	int threshold;
	int retire_threshold;
	int refresh;
	spinlock_t refresh_lock;

	struct workqueue_struct *free_layerbuf_queue;
	struct work_struct free_layerbuf_work;
	struct list_head layerbuf_list;
	bool layerbuf_flushed;
	spinlock_t layerbuf_spinlock;
	struct semaphore layerbuf_sem;
};

struct dpufb_layerbuf {
	struct dma_buf *buffer_handle;
	struct list_head list_node;
	int timeline;

	struct hisi_dss_fence *fence;
	int32_t shared_fd;
	uint32_t frame_no;
	dss_mmbuf_t mmbuf;
	uint64_t vir_addr;
	int32_t chn_idx;
};

struct dpufb_backlight {
#ifdef CONFIG_HISI_FB_BACKLIGHT_DELAY
	struct delayed_work bl_worker;
#endif
	struct semaphore bl_sem;
	int bl_updated;
	int bl_level_old;
	int frame_updated;

	ktime_t bl_timestamp;
};

/* for online play bypss function */
struct online_play_bypass_info {
	bool bypass;
	uint32_t bypass_count;
};

struct effect_init_update_status {
	uint32_t arsr1p_update_normal : 1;
	uint32_t arsr1p_update_fhd : 1;
	uint32_t arsr1p_update_hd : 1;
	uint32_t arsr2p_update_normal : 1;
	uint32_t arsr2p_update_scale_up : 1;
	uint32_t arsr2p_update_scale_down : 1;
	uint32_t hiace_mode : 8;
	uint32_t hiace_param_update : 1;
	uint32_t hiace_lut_update : 1;
	uint32_t resevered : 16;
};

struct dpu_fb_data_type {
	uint32_t index;
	uint32_t ref_cnt;
	uint32_t fb_num;
	uint32_t fb_imgType;
	uint32_t bl_level;
	uint32_t clk_vote_level;

	char __iomem *dss_base;
	char __iomem *peri_crg_base;
	char __iomem *sctrl_base;
	char __iomem *pctrl_base;
	char __iomem *noc_dss_base;
	char __iomem *mmbuf_crg_base;
	char __iomem *pmctrl_base;

	char __iomem *media_crg_base;
	char __iomem *media_common_base;
	char __iomem *dp_base;
	char __iomem *mmc0_crg;
	char __iomem *dp_phy_base;
	char __iomem *usb_dp_ctrl_base;
	char __iomem *usb_tca_base;

	char __iomem *mmbuf_asc0_base;
	char __iomem *mipi_dsi0_base;
	char __iomem *mipi_dsi1_base;
	uint32_t dss_base_phy;

	uint32_t dpe_irq;
	uint32_t dsi0_irq;
	uint32_t dsi1_irq;
	uint32_t dp_irq;

	struct regulator_bulk_data *dpe_regulator;
	struct regulator_bulk_data *mediacrg_regulator;
	struct regulator_bulk_data *smmu_tcu_regulator;

	const char *dss_axi_clk_name;
	const char *dss_pclk_dss_name;
	const char *dss_pri_clk_name;
	const char *dss_pxl0_clk_name;
	const char *dss_pxl1_clk_name;
	const char *dss_mmbuf_clk_name;
	const char *dss_pclk_mmbuf_name;
	const char *dss_dphy0_ref_clk_name;
	const char *dss_dphy1_ref_clk_name;
	const char *dss_dphy0_cfg_clk_name;
	const char *dss_dphy1_cfg_clk_name;
	const char *dss_pclk_dsi0_name;
	const char *dss_pclk_dsi1_name;
	const char *dss_pclk_pctrl_name;
	const char *dss_auxclk_dpctrl_name;
	const char *dss_pclk_dpctrl_name;
	const char *dss_aclk_dpctrl_name;

	struct clk *dss_axi_clk;
	struct clk *dss_pclk_dss_clk;
	struct clk *dss_pri_clk;
	struct clk *dss_pxl0_clk;
	struct clk *dss_pxl1_clk;
	struct clk *dss_mmbuf_clk;
	struct clk *dss_pclk_mmbuf_clk;
	struct clk *dss_dphy0_ref_clk;
	struct clk *dss_dphy1_ref_clk;
	struct clk *dss_dphy0_cfg_clk;
	struct clk *dss_dphy1_cfg_clk;
	struct clk *dss_pclk_dsi0_clk;
	struct clk *dss_pclk_dsi1_clk;
	struct clk *dss_pclk_pctrl_clk;
	struct clk *dss_auxclk_dpctrl_clk;
	struct clk *dss_pclk_dpctrl_clk;
	struct clk *dss_aclk_dpctrl_clk;

	struct dpu_panel_info panel_info;
	bool panel_power_on;
	bool fb_shutdown;
	bool need_tuning_clk;
	bool lcd_self_testing;
#if defined(CONFIG_HISI_FB_AOD)
	bool enable_fast_unblank; /* support fast unblank only for aod */
	bool enable_fast_unblank_for_multi_panel; /* support fast unblank and fb blank meanwhile for aod */
#endif
#if defined(CONFIG_HISI_FB_AOD) || defined(CONFIG_DPU_FB_AP_AOD) || \
	defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
	unsigned int aod_function;
	unsigned int aod_mode;
#endif
	/* 0: need update base frame when overlay on;
	1: skip update base frame when overlay on */
	unsigned int base_frame_mode;
#if defined(CONFIG_DPU_FB_AP_AOD)
	unsigned int ap_aod_skip_frame_flag;
#endif
	unsigned int vr_mode;
	unsigned int mask_layer_xcc_flag;
	atomic_t atomic_v;

	struct semaphore blank_sem;
	struct semaphore blank_sem0;
	struct semaphore blank_sem_effect;

	/* for hiace_workqueue and blank_sub */
	struct semaphore blank_sem_effect_hiace;

	/* for gmp_workqueue and blank_sub */
	struct semaphore blank_sem_effect_gmp;

	/* for gmp_workqueue and gmp_online */
	struct semaphore effect_gmp_sem;
	struct semaphore brightness_esd_sem;
#if defined(CONFIG_HISI_FB_AOD)
	struct semaphore blank_aod_sem;
	struct semaphore fast_unblank_sem;
#endif
	struct semaphore power_sem;
	struct semaphore offline_composer_sr_sem;
#if defined(CONFIG_HISI_FB_V320)
	struct semaphore hiace_clear_sem;
#endif
	struct semaphore hiace_hist_lock_sem;
	struct semaphore esd_panel_change_sem;

	uint32_t offline_composer_sr_refcount;

	void (*sysfs_attrs_append_fnc)(struct dpu_fb_data_type *dpufd, struct attribute *attr);
	int (*sysfs_create_fnc)(struct platform_device *pdev);
	void (*sysfs_remove_fnc)(struct platform_device *pdev);
	void (*pm_runtime_register)(struct platform_device *pdev);
	void (*pm_runtime_unregister)(struct platform_device *pdev);
	void (*pm_runtime_get)(struct dpu_fb_data_type *dpufd);
	void (*pm_runtime_put)(struct dpu_fb_data_type *dpufd);
	void (*bl_register)(struct platform_device *pdev);
	void (*bl_unregister)(struct platform_device *pdev);
	void (*bl_update)(struct dpu_fb_data_type *dpufd);
	void (*bl_cancel)(struct dpu_fb_data_type *dpufd);
	void (*vsync_register)(struct platform_device *pdev);
	void (*vsync_unregister)(struct platform_device *pdev);
	int (*vsync_ctrl_fnc)(struct fb_info *info, const void __user *argp);
	void (*vsync_isr_handler)(struct dpu_fb_data_type *dpufd);
	void (*secure_register)(struct platform_device *pdev);
	void (*secure_unregister)(struct platform_device *pdev);
	void (*buf_sync_register)(struct platform_device *pdev);
	void (*buf_sync_unregister)(struct platform_device *pdev);
	void (*buf_sync_signal)(struct dpu_fb_data_type *dpufd);
	void (*buf_sync_suspend)(struct dpu_fb_data_type *dpufd);
	void (*esd_register)(struct platform_device *pdev);
	void (*esd_unregister)(struct platform_device *pdev);
	void (*debug_register)(struct platform_device *pdev);
	void (*debug_unregister)(struct platform_device *pdev);
	int (*cabc_update)(struct dpu_fb_data_type *dpufd);
	void (*pipe_clk_updt_isr_handler)(struct dpu_fb_data_type *dpufd);

	bool (*set_fastboot_fnc)(struct fb_info *info);
	int (*open_sub_fnc)(struct fb_info *info);
	int (*release_sub_fnc)(struct fb_info *info);
	int (*hpd_open_sub_fnc)(struct fb_info *info);
	int (*hpd_release_sub_fnc)(struct fb_info *info);
	int (*on_fnc)(struct dpu_fb_data_type *dpufd);
	int (*off_fnc)(struct dpu_fb_data_type *dpufd);
	int (*lp_fnc)(struct dpu_fb_data_type *dpufd, bool lp_enter);
	int (*esd_fnc)(struct dpu_fb_data_type *dpufd);
	int (*sbl_ctrl_fnc)(struct fb_info *info, uint32_t value);
	void (*sbl_isr_handler)(struct dpu_fb_data_type *dpufd);
	int (*fps_upt_isr_handler)(struct dpu_fb_data_type *dpufd);
	int (*mipi_dsi_bit_clk_upt_isr_handler)(struct dpu_fb_data_type *dpufd);
	void (*panel_mode_switch_isr_handler)(struct dpu_fb_data_type *dpufd, uint8_t mode_switch_to);
	void (*crc_isr_handler)(struct dpu_fb_data_type *dpufd);
	void (*ov_ldi_underflow_isr_handle)(struct dpu_fb_data_type *dpufd);

	int (*pan_display_fnc)(struct dpu_fb_data_type *dpufd);
	int (*ov_ioctl_handler)(struct dpu_fb_data_type *dpufd, uint32_t cmd, void __user *argp);
	int (*display_effect_ioctl_handler)(struct dpu_fb_data_type *dpufd, unsigned int cmd, void __user *argp);
	int (*ov_online_play)(struct dpu_fb_data_type *dpufd, void __user *argp);
#if defined(CONFIG_ASYNCHRONOUS_PLAY)
	int (*ov_asynchronous_play)(struct dpu_fb_data_type *dpufd, void __user *argp);
#endif
	int (*ov_offline_play)(struct dpu_fb_data_type *dpufd, const void __user *argp);
	int (*ov_media_common_play)(struct dpu_fb_data_type *dpufd, const void __user *argp);
	void (*ov_wb_isr_handler)(struct dpu_fb_data_type *dpufd);
	void (*ov_vactive0_start_isr_handler)(struct dpu_fb_data_type *dpufd);
	void (*set_reg)(struct dpu_fb_data_type *dpufd,
		char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs);

	void (*sysfs_attrs_add_fnc)(struct dpu_fb_data_type *dpufd);

	int (*dp_device_srs)(struct dpu_fb_data_type *dpufd, bool blank);
	int (*dp_get_color_bit_mode)(struct dpu_fb_data_type *dpufd, void __user *argp);
	int (*dp_get_source_mode)(struct dpu_fb_data_type *dpufd, void __user *argp);
	int (*dptx_hdr_infoframe_sdp_send)(struct dp_ctrl *dptx, const void __user *argp);
	int (*dp_pxl_ppll7_init)(struct dpu_fb_data_type *dpufd, u64 pixel_clock);
	uint64_t (*dp_get_pxl_clk_rate)(struct dp_ctrl *dptx);

	void (*video_idle_ctrl_register)(struct platform_device *pdev);
	void (*video_idle_ctrl_unregister)(struct platform_device *pdev);

	void (*overlay_online_wb_register)(struct platform_device *pdev);
	void (*overlay_online_wb_unregister)(struct platform_device *pdev);

	int (*dp_wakeup)(struct dpu_fb_data_type *dpufd);

	int (*panel_set_display_region)(struct dpu_fb_data_type *dpufd, const void __user *argp);

	int (*set_display_panel_status)(struct dpu_fb_data_type *dpufd, const void __user *argp);

	struct dpufb_backlight backlight;
	int sbl_enable;
	uint32_t sbl_lsensor_value;
	int sbl_level;
	dss_sbl_t sbl;
	int sre_enable;

	ktime_t te_timestamp; /* record the timestamp of TE signal */
	int color_temperature_flag;
	int display_effect_flag;
	int xcc_coef_set;
	dss_display_effect_al_t al_ctrl;
	dss_display_effect_ce_t ce_ctrl;
	dss_display_effect_bl_t bl_ctrl;
	dss_display_effect_bl_enable_t bl_enable_ctrl;
	dss_display_effect_sre_t sre_ctrl;
	dss_display_effect_metadata_t metadata_ctrl;
	dss_ce_info_t acm_ce_info;
	dss_ce_info_t prefix_ce_info[DSS_CHN_MAX_DEFINE];
	display_engine_info_t de_info;
	display_engine_param_t de_param;
	int user_scene_mode;
	int dimming_count;
	acm_reg_t acm_reg;
	dss_ce_info_t hiace_info[DISP_PANEL_NUM];
	dss_gm_t dynamic_gamma_info;

	uint32_t hihdr_mean;
	uint32_t hihdr_basic_ready;

	char blc_last_bl_level;
	char reserved[3];
	bool dirty_region_updt_enable;
	uint32_t  online_play_count;

	struct mutex effect_lock;
	struct dss_effect effect_ctl;
	struct dss_module_update effect_updated_flag[DISP_PANEL_NUM];
	struct dpufb_effect_info effect_info[DISP_PANEL_NUM];
	struct task_struct *effect_thread;
	wait_queue_head_t effect_wq;
	bool effect_update;
	struct effect_init_update_status effect_init_update[DISP_PANEL_NUM];

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V330)
	bool effect_gmp_update_flag;
	uint32_t gmp_online_set_reg_count;
	uint32_t gmp_last_lut_sel;
	uint32_t effect_gmp_copy_status;
#endif
	struct panel_aging_time_info aging_time_info;

	int sysfs_index;
	struct attribute *sysfs_attrs[HISI_FB_SYSFS_ATTRS_NUM];
	struct attribute_group sysfs_attr_group;

	struct dpufb_vsync vsync_ctrl;
	struct dpufb_buf_sync buf_sync_ctrl;
#if defined(CONFIG_VIDEO_IDLE)
	struct dpufb_video_idle_ctrl video_idle_ctrl;
#endif
	struct dss_vote_cmd dss_vote_cmd;
	struct dpufb_writeback wb_ctrl;
	struct dpufb_secure secure_ctrl;
	struct dpufb_esd esd_ctrl;
	struct dpufb_pipe_clk pipe_clk_ctrl;
	struct dp_ctrl dp;
	bool need_refresh;
	bool is_idle_display;
	bool fold_panel_display_change;
	mdc_func_ops_t mdc_ops;

	struct dss_module_reg dss_module;
	dss_overlay_t ov_req;
	dss_overlay_block_t ov_block_infos[HISI_DSS_OV_BLOCK_NUMS];
	dss_overlay_t ov_req_prev;
	dss_overlay_block_t ov_block_infos_prev[HISI_DSS_OV_BLOCK_NUMS];
	dss_overlay_t ov_req_prev_prev;
	dss_overlay_block_t ov_block_infos_prev_prev[HISI_DSS_OV_BLOCK_NUMS];

	dss_rect_t *ov_block_rects[HISI_DSS_CMDLIST_BLOCK_MAX];
	struct dss_wb_info wb_info;

	struct dss_cmdlist_data *cmdlist_data_tmp[HISI_DSS_CMDLIST_DATA_MAX];
	struct dss_cmdlist_data *cmdlist_data;
	struct dss_cmdlist_info *cmdlist_info;
	int32_t cmdlist_idx;

	dss_hihdr_info_t *hdr_info;

	struct dss_copybit_info *copybit_info;

	struct clk *dss_aclk_media_common_clk;
	struct clk *dss_clk_media_common_clk;
	struct regulator *mdc_regulator;
	struct semaphore media_common_sr_sem;
	int32_t media_common_composer_sr_refcount;
	struct dss_cmdlist_data *media_common_cmdlist_data;
	struct dss_media_common_info *media_common_info;
	/* save mdc offline compose info */
	dss_overlay_t *pov_req;
	struct mutex work_lock;
	struct kthread_work preoverlay_work;
	struct kthread_worker preoverlay_worker;
	struct task_struct *preoverlay_thread;

	struct gen_pool *mmbuf_gen_pool;
	struct dss_mmbuf_info mmbuf_infos[HISI_DSS_CMDLIST_DATA_MAX];
	struct dss_mmbuf_info *mmbuf_info;
	struct list_head *mmbuf_list;
	uint32_t use_mmbuf_cnt;
	uint32_t use_axi_cnt;

	bool dss_module_resource_initialized;
	struct dss_module_reg dss_module_default;
	struct dss_module_reg dss_mdc_module_default;

	struct dss_rect dirty_region_updt;
	uint32_t esd_happened;
	uint32_t esd_recover_state;
	uint32_t esd_check_is_doing;
	struct sg_table *fb_sg_table;
	bool fb_pan_display;

	struct gen_pool *cmdlist_pool;
	void *cmdlist_pool_vir_addr;
	phys_addr_t cmdlist_pool_phy_addr;
	size_t sum_cmdlist_pool_size;

	struct fb_info *fbi;
	struct platform_device *pdev;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif

	wait_queue_head_t vactive0_start_wq;
	uint32_t vactive0_start_flag;
	uint32_t vactive0_end_flag;
	uint32_t ldi_data_gate_en;

	wait_queue_head_t crc_wq;
	uint32_t crc_flag;
	uint32_t underflow_flag;
	struct workqueue_struct *dss_debug_wq;
	struct work_struct dss_debug_work;

#if defined(CONFIG_HISI_FB_AOD)
	struct workqueue_struct *aod_ud_fast_unblank_workqueue;
	struct work_struct aod_ud_fast_unblank_work;
#endif

	struct workqueue_struct *ldi_underflow_wq;
	struct work_struct ldi_underflow_work;
	spinlock_t underflow_lock;

	struct workqueue_struct *rch2_ce_end_wq;
	struct work_struct rch2_ce_end_work;
	struct workqueue_struct *rch4_ce_end_wq;
	struct work_struct rch4_ce_end_work;
	struct workqueue_struct *dpp_ce_end_wq;
	struct workqueue_struct *hiace_end_wq;
	struct work_struct hiace_end_work;
	struct workqueue_struct *display_engine_wq;
	struct work_struct display_engine_work;
	struct workqueue_struct *de_updata_te_wq;
	struct work_struct de_updata_te_work;
	struct work_struct de_open_ic_dim_work;
	struct work_struct de_set_hbm_func_work;
	struct workqueue_struct *dfr_notice_wq;
	struct work_struct dfr_notice_work;
	struct workqueue_struct *dfr_delay_wq;
	struct work_struct dfr_delay_work;
#if defined(CONFIG_HUAWEI_DSM)
	struct workqueue_struct *dss_underflow_debug_workqueue;
	struct work_struct dss_underflow_debug_work;
#endif
#if !defined(CONFIG_HISI_FB_V320)
	struct workqueue_struct *gmp_lut_wq;
	struct work_struct gmp_lut_work;
#endif

	struct workqueue_struct *masklayer_backlight_notify_wq;
	struct work_struct masklayer_backlight_notify_work;

	struct workqueue_struct *delayed_cmd_queue_wq;
	struct work_struct delayed_cmd_queue_work;

	/* ALSC module */
#ifdef CONFIG_HISI_FB_ALSC
	struct dss_alsc alsc;
	struct workqueue_struct *alsc_send_data_wq;
	struct work_struct alsc_send_data_work;
#endif

	dss_rect_t resolution_rect;

	uint32_t frame_count;
	uint32_t frame_update_flag;
	bool fb_mem_free_flag;

	uint8_t core_clk_upt_support;
	uint8_t mipi_dsi_bit_clk_update;
	spinlock_t mipi_resource_lock;

	uint32_t vactive_start_event;

	uint32_t vsync_ctrl_type;
	struct notifier_block nb;
	struct notifier_block lcd_int_nb; /* for clear lcd ocp interrupt */

	/* sensorhub aod */
	bool masklayer_maxbacklight_flag;
	uint8_t masklayer_flag;
	bool hbm_is_opened;
	bool hbm_need_to_open;
	int ud_fp_scene;
	int ud_fp_hbm_level;
	int ud_fp_current_level;
	struct semaphore sh_aod_blank_sem;

	bool enter_idle;
	uint32_t emi_protect_check_count;

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
	struct hiace_roi_info hist_hiace_roi_info;
	struct hiace_roi_info auto_hiace_roi_info;
#endif
#if defined(CONFIG_HISI_FB_V501)
	struct gamma_xcc_roi_info gamma_xcc_roi;
#endif

	struct online_play_bypass_info bypass_info;
	struct dss_dump_data_type *dumpDss;
#if defined(CONFIG_ASYNCHRONOUS_PLAY)
	wait_queue_head_t asynchronous_play_wq;
	uint32_t asynchronous_vactive0_start_flag;
#endif
	bool fb2_irq_on_flag;
	bool fb2_irq_force_on_flag;
	bool evs_enable;
	uint32_t panel_power_status;  /* inner panel on: 0x1; outer panel on:0x2; inner&outer panel on: 0x3 */
	bool dfr_create_singlethread;
#ifdef CONFIG_HISI_DPP_CMDLIST
	dpp_cmdlist_data_t *dpp_cmdlist_data[DPP_CMDLIST_REGION_NUM][DPP_CMDLIST_BUFFER_NUM];
	uint8_t dpp_cmdlist_buffer_wr[DPP_CMDLIST_REGION_NUM];
	uint8_t dpp_cmdlist_buffer_rd[DPP_CMDLIST_REGION_NUM];
	uint8_t dpp_cmdlist_region;
	uint8_t dpp_cmdlist_buffer_last_chn;
	spinlock_t dpp_cmdlist_ch_status;
	spinlock_t dpp_cmdlist_lock;
	uint32_t dpp_cmdlist_isr_vactive_cnt;
	uint8_t dpp_cmdlist_none_update_frame;
	bool dpp_cmdlist_start_config;
#endif
};
#endif /* HISI_FB_STRUCT_H */

