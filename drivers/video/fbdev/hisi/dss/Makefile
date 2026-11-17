ifeq ($(CONFIG_FB),y)
obj-$(CONFIG_FB) := hisifb.o
endif

ifeq ($(CONFIG_HISI_FB_MEDIACOMMON),y)
obj-$(CONFIG_HISI_FB_MEDIACOMMON) += mediacommon/
endif

ifeq ($(CONFIG_HISI_FB_ALSC),y)
obj-$(CONFIG_HISI_FB_ALSC) += effect/display_effect_alsc.o
endif

ifeq ($(CONFIG_SYNC_FILE),y)
obj-$(CONFIG_SYNC_FILE) += hisi_dss_sync.o
endif

ifeq ($(CONFIG_DSS_SMMU_V3),y)
obj-$(CONFIG_DSS_SMMU_V3) += hisi_dss_smmuv3.o
endif

ifeq ($(CONFIG_HISI_DISPLAY_DFR),y)
obj-$(CONFIG_HISI_DISPLAY_DFR) += hisi_frame_rate_ctrl.o
endif

ifeq ($(CONFIG_HISI_DSS_DPM),y)
obj-$(CONFIG_HISI_DSS_DPM) += hisi_dss_dpm.o
endif

ifeq ($(CONFIG_HISI_DPP_CMDLIST),y)
obj-$(CONFIG_HISI_DPP_CMDLIST) += hisi_dpp_cmdlist_utils.o
endif

ifeq ($(CONFIG_VIDEO_IDLE),y)
obj-$(CONFIG_VIDEO_IDLE) += hisi_fb_video_idle.o
endif

ifeq ($(CONFIG_HISI_FB_AOD),y)
obj-$(CONFIG_HISI_FB_AOD) += hisi_sensorhub_aod.o
endif

ifeq ($(CONFIG_ASYNCHRONOUS_PLAY),y)
obj-$(CONFIG_ASYNCHRONOUS_PLAY) += hisi_overlay_asynchronous_play.o
endif

ifeq ($(CONFIG_RCH_CLD),y)
obj-$(CONFIG_RCH_CLD) += overlay/hisi_dss_cld.o
endif

ifeq ($(CONFIG_HISI_EFFECT_HIHDR),y)
obj-$(CONFIG_HISI_EFFECT_HIHDR) += hisi_hihdr.o hisi_hihdr_itm.o hisi_hihdr_gtm.o
endif

hisifb-objs := \
	hisi_mipi_dsi_host.o \
	hisi_mipi_dsi.o \
	hisi_pwm.o \
	hisi_blpwm.o \
	hisi_sensorhub_blpwm.o \
	hisi_dpe.o \
	hisi_fb_panel.o \
	hisi_fb_isr.o \
	hisi_fb_vsync.o \
	hisi_fb_esd.o \
	hisi_fb_secure.o \
	hisi_fb_buf_sync.o \
	hisi_fb_bl.o \
	hisi_fb_utils.o \
	hisi_fb.o \
	hisi_fb_dts.o \
	hisi_fb_debug.o \
	hisi_mdc_resource.o \
	hisi_dss_smmu_base.o \
	overlay/hisi_overlay_utils.o \
	overlay/hisi_dss_clip.o \
	overlay/hisi_dss_dfc.o \
	overlay/hisi_dss_dirty.o \
	overlay/hisi_dss_dump.o \
	overlay/hisi_dss_handler.o \
	overlay/hisi_dss_init.o \
	overlay/hisi_dss_mctl.o \
	overlay/hisi_dss_mif.o \
	overlay/hisi_dss_ovl.o \
	overlay/hisi_dss_rdma.o \
	overlay/hisi_dss_scl.o \
	overlay/hisi_dss_tool.o \
	overlay/hisi_dss_wdma.o \
	hisi_overlay_online.o \
	hisi_overlay_offline.o \
	hisi_block_algorithm.o \
	hisi_overlay_cmdlist_utils.o \
	hisi_offlinecomposer_panel.o \
	panel/mipi_lcd_utils.o \
	performance/hisi_dss_performance.o \
	hisi_display_effect.o \
	hisi_mmbuf_manager.o \
	hisi_fb_sysfs.o \
	product/hisi_displayengine_utils.o \
	product/hisi_displayengine_effect.o \
	product/hisi_panel_switch.o

ifeq ($(CONFIG_DEVICE_ATTRS),y)
hisifb-objs += \
	product/attrs/hisi_fb_attrs.o \
	product/attrs/hisi_dpe_attrs.o \
	product/attrs/hisi_mipi_attrs.o \
	product/attrs/hisi_panel_attrs.o
endif

ifeq ($(CONFIG_HUAWEI_DSM),y)
hisifb-objs += \
	product/hisi_fb_panel_dsm.o
endif

hisifb-objs += \
	hisi_dss_ion.o \
	hisi_dss_iommu.o

ifneq ($(TARGET_BUILD_VARIANT),user)
hisifb-objs += \
	hisi_fb_panel_debug.o
endif

ifneq ($(filter $(CONFIG_HISI_FB_970) $(CONFIG_HISI_FB_V501) $(CONFIG_HISI_FB_V510),y),)
hisifb-objs += \
	dp/dp_dbg.o \
	dp/controller/dp_avgen_base.o \
	dp/controller/hidpc/hidpc_dp_aux.o \
	dp/controller/hidpc/hidpc_dp_irq.o \
	dp/controller/hidpc/hidpc_dp_avgen.o \
	dp/controller/hidpc/hidpc_dp_core.o \
	dp/phy/hidpc_phy_config.o \
	dp/link/dp_irq.o \
	dp/link/dp_aux.o \
	dp/link/dp_edid.o \
	dp/link/dp_link_training.o \
	dp/link/dp_mst_topology.o \
	dp/link/drm_dp_helper.o \
	dsc/dsc_algorithm.o \
	dp/controller/dsc/hidpc_dsc_config.o \
	dp/controller/dsc/dsc_config_base.o \
	dp/link/dp_dsc_algorithm.o \
	dsc/utilities/hisi_debug.o \
	dsc/dsc_algorithm_manager.o \
	panel/dp_fake_panel.o

ifeq ($(CONFIG_DP_HDCP),y)
hisifb-objs += \
	hdcp/esm/host_lib_driver_linux_kernel.o \
	hdcp/hdcp_common.o \
	hdcp/hdcp_driver.o \
	hdcp/hdcp_reg_esm.o
endif
endif

ifneq ($(filter $(CONFIG_HISI_FB_V600) $(CONFIG_HISI_FB_V360),y),)
hisifb-objs += \
	dp/dp_dbg.o \
	dp/controller/dp_avgen_base.o \
	dp/controller/hidptx/hidptx_dp_aux.o \
	dp/controller/hidptx/hidptx_dp_irq.o \
	dp/controller/hidptx/hidptx_dp_avgen.o \
	dp/controller/hidptx/hidptx_dp_core.o \
	dp/phy/hidptx_phy_config.o \
	dp/link/dp_irq.o \
	dp/link/dp_aux.o \
	dp/link/dp_edid.o \
	dp/link/dp_link_training.o \
	dp/link/dp_mst_topology.o \
	dp/link/drm_dp_helper.o \
	dp/controller/dsc/hidptx_dsc_config.o \
	dp/controller/dsc/dsc_config_base.o \
	dp/link/dp_dsc_algorithm.o \
	panel/dp_fake_panel.o

ifeq ($(CONFIG_DP_HDCP),y)
hisifb-objs += \
	hdcp/hdcp_common.o \
	hdcp/hdcp_driver.o \
	hdcp/hdcp_reg_soft.o

EXTRA_CFLAGS += -DCONFIG_HDCP_SOFT_AUTH
endif
endif

ifeq ($(CONFIG_HISI_FB_970),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_dpe_utils_kirin970.o \
	hisi_display_effect_kirin970.o \
	hisi_overlay_utils_kirin970.o \
	hisi_mipi_dsi_v410.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	backlight/rt8555.o

hisifb-objs += voltage/tps65132.o
hisifb-objs += voltage/rt4801h.o
hisifb-objs += voltage/ina231.o

ifeq ($(CONFIG_HISI_NOC_MODID_REGISTER),y)
hisifb-objs += \
	hisi_dss_mntn.o \
	hisi_dss_mntn_kirin970.o
endif
endif

ifeq ($(CONFIG_HISI_FB_V501),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_dpe_utils_dssv501.o \
	hisi_display_effect_dssv501.o \
	hisi_overlay_utils_dssv501.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_fb_hisync.o \
	hisi_mipi_dsi_v501.o \
	hisi_ovl_online_wb.o \
	panel_old/dssv501/mipi_lg_TD4322.o \
	panel_old/dssv501/mipi_jdi_R63452.o \
	panel_old/dssv501/mipi_sharp_NT36870.o

hisifb-objs += voltage/ina231.o
hisifb-objs += voltage/tps65132.o
ifeq ($(CONFIG_HISI_NOC_MODID_REGISTER),y)
hisifb-objs += \
	hisi_dss_mntn.o \
	hisi_dss_mntn_dssv501.o
endif

hisifb-objs += hisi_hiace_single_mode.o
EXTRA_CFLAGS += -DHIACE_SINGLE_MODE_SUPPORT
endif

ifeq ($(CONFIG_HISI_FB_V510),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_dpe_utils_dssv510.o \
	hisi_dpe_utils_clk_dssv510.o \
	hisi_dpe_clk_sub_dssv510.o \
	hisi_dpe_effect_utils.o \
	performance/hisi_dss_performance_v510.o \
	hisi_display_effect_dssv510.o \
	overlay/hisi_overlay_utils_dssv510.o \
	overlay/hisi_dss_aif_dssv510.o \
	overlay/hisi_dss_arsr_dssv510.o \
	overlay/hisi_dss_bce_bcd_dssv510.o \
	overlay/hisi_dss_clip_dssv510.o \
	overlay/hisi_dss_csc_dssv510.o \
	overlay/hisi_dss_mctl_dssv510.o \
	overlay/hisi_dss_mif_dssv510.o \
	overlay/hisi_dss_ovl_dssv510.o \
	overlay/hisi_dss_scf_dssv510.o \
	overlay/hisi_dss_scl_dssv510.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_fb_hisync.o \
	hisi_mipi_dsi_v510.o \
	hisi_spr_dsc.o \
	panel/dssv510/mipi_samsung_EA8076.o \
	backlight/lm36923.o

ifeq ($(CONFIG_ICS_DRIVER),y)
hisifb-objs += \
	panel/cdc/kernel/drivers/mipi2lvds/serdes_i2c.o \
	panel/cdc/kernel/drivers/mipi2lvds/mipi2lvds_serdes_init.o \
	panel/cdc/kernel/drivers/mipi2lvds/mipi_lvds_panel.o \
	panel/cdc/kernel/drivers/mipi2lvds/drv_indigo.o
endif

hisifb-objs += voltage/ina231.o
hisifb-objs += voltage/tps65132.o

ifeq ($(CONFIG_HISI_NOC_MODID_REGISTER),y)
hisifb-objs += \
	hisi_dss_mntn.o \
	hisi_dss_mntn_dssv510.o
endif

hisifb-objs += hisi_hiace_single_mode.o
EXTRA_CFLAGS += -DHIACE_SINGLE_MODE_SUPPORT
endif

ifeq ($(CONFIG_HUAWEI_DUBAI_RGB_STATS),y)
hisifb-objs += product/rgb_stats/hisi_fb_rgb_stats.o
endif

ifeq ($(CONFIG_HISI_FB_V320),y)
hisifb-objs += \
	hisi_display_effect_dssv320.o \
	hisi_dpe_utils_dssv320.o \
	hisi_overlay_utils_dssv320.o \
	hisi_mipi_dsi_v3xx.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	backlight/rt8555.o \
	panel_old/dssv320/mipi_tm_OTM1911A.o \
	panel_old/dssv320/mipi_cmi_NT35521.o \
	panel/dssv320/mipi_boe_RM69330.o

hisifb-objs += \
	panel/dp_fake_panel.o

hisifb-objs += voltage/tps65132.o
hisifb-objs += voltage/rt4801h.o
hisifb-objs += voltage/ina231.o
ifeq ($(CONFIG_HISI_NOC_MODID_REGISTER),y)
hisifb-objs += \
	hisi_dss_mntn.o \
	hisi_dss_mntn_dssv320.o
endif
endif

ifeq ($(CONFIG_HISI_FB_V330),y)
hisifb-objs += \
	hisi_display_effect_dssv330.o \
	hisi_dpe_utils_dssv330.o \
	hisi_overlay_utils_dssv330.o \
	hisi_mipi_dsi_v330.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	backlight/rt8555.o \
	panel/dssv330/mipi_lg_td4322.o

hisifb-objs += \
	panel/dp_fake_panel.o
hisifb-objs += voltage/tps65132.o
hisifb-objs += voltage/rt4801h.o
hisifb-objs += voltage/ina231.o

ifeq ($(CONFIG_HISI_NOC_MODID_REGISTER),y)
hisifb-objs += \
	hisi_dss_mntn.o \
	hisi_dss_mntn_dssv330.o
endif
endif

ifeq ($(CONFIG_HISI_FB_V350),y)
hisifb-objs += \
	hisi_display_effect_dssv510.o \
	hisi_dpe_utils_dssv510.o \
	hisi_dpe_utils_clk_dssv510.o \
	hisi_dpe_clk_sub_dssv510.o \
	hisi_dpe_effect_utils.o \
	performance/hisi_dss_performance_v510.o \
	overlay/hisi_overlay_utils_dssv510.o \
	overlay/hisi_dss_aif_dssv510.o \
	overlay/hisi_dss_arsr_dssv510.o \
	overlay/hisi_dss_bce_bcd_dssv510.o \
	overlay/hisi_dss_clip_dssv510.o \
	overlay/hisi_dss_csc_dssv510.o \
	overlay/hisi_dss_mctl_dssv510.o \
	overlay/hisi_dss_mif_dssv510.o \
	overlay/hisi_dss_ovl_dssv510.o \
	overlay/hisi_dss_scf_dssv510.o \
	overlay/hisi_dss_scl_dssv510.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_fb_hisync.o \
	hisi_mipi_dsi_v510.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	panel_old/dssv350/mipi_samsung_EA8076.o \
	panel/dssv330/mipi_lg_td4322.o

ifeq ($(CONFIG_HISI_NOC_MODID_REGISTER),y)
hisifb-objs += \
	hisi_dss_mntn.o \
	hisi_dss_mntn_dssv350.o
endif
endif

ifeq ($(CONFIG_HISI_FB_V600),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_dpe_utils_dssv510.o \
	hisi_dpe_utils_clk_dssv510.o \
	hisi_dpe_clk_sub_dssv600.o \
	hisi_dpe_effect_utils.o \
	performance/hisi_dss_performance_v510.o \
	hisi_display_effect_dssv510.o \
	overlay/hisi_overlay_utils_dssv510.o \
	overlay/hisi_dss_aif_dssv510.o \
	overlay/hisi_dss_arsr_dssv510.o \
	overlay/hisi_dss_bce_bcd_dssv510.o \
	overlay/hisi_dss_clip_dssv510.o \
	overlay/hisi_dss_csc_dssv510.o \
	overlay/hisi_dss_mctl_dssv510.o \
	overlay/hisi_dss_mif_dssv510.o \
	overlay/hisi_dss_ovl_dssv510.o \
	overlay/hisi_dss_scf_dssv510.o \
	overlay/hisi_dss_scl_dssv510.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_fb_hisync.o \
	hisi_mipi_dsi_v510.o \
	hisi_spr_dsc.o \
	backlight/lm36923.o \
	product/bridge/edp_bridge.o \
	panel_old/mipi_samsung_EA8076.o \
	panel_old/dssv600/mipi_sharp_NT36870.o \
	panel_old/dssv600/mipi_sharp_TD4336.o \
	panel/dssv330/mipi_lg_td4322.o \
	dsc/dsc_algorithm.o \
	dsc/utilities/hisi_debug.o \
	dsc/dsc_algorithm_manager.o \
	hisi_dpe_clk_low_temp.o \
	panel_old/dssv600/mipi_boe_nt37700p.o

hisifb-objs += hisi_hiace_single_mode.o
EXTRA_CFLAGS += -DHIACE_SINGLE_MODE_SUPPORT
endif


ifeq ($(CONFIG_HISI_FB_V360),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_dpe_utils_dssv510.o \
	hisi_dpe_utils_clk_dssv510.o \
	hisi_dpe_clk_sub_dssv360.o \
	hisi_dpe_effect_utils.o \
	performance/hisi_dss_performance_v510.o \
	hisi_display_effect_dssv510.o \
	overlay/hisi_overlay_utils_dssv510.o \
	overlay/hisi_dss_aif_dssv510.o \
	overlay/hisi_dss_arsr_dssv510.o \
	overlay/hisi_dss_bce_bcd_dssv510.o \
	overlay/hisi_dss_clip_dssv510.o \
	overlay/hisi_dss_csc_dssv510.o \
	overlay/hisi_dss_mctl_dssv510.o \
	overlay/hisi_dss_mif_dssv510.o \
	overlay/hisi_dss_ovl_dssv510.o \
	overlay/hisi_dss_scf_dssv510.o \
	overlay/hisi_dss_scl_dssv510.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_fb_hisync.o \
	hisi_mipi_dsi_v510.o \
	hisi_spr_dsc.o \
	backlight/lm36923.o \
	panel_old/mipi_samsung_EA8076.o \
	panel_old/dssv600/mipi_sharp_NT36870.o \
	panel_old/dssv600/mipi_sharp_TD4336.o \
	panel/dssv330/mipi_lg_td4322.o \
	dsc/dsc_algorithm.o \
	dsc/utilities/hisi_debug.o \
	dsc/dsc_algorithm_manager.o \
	panel_old/dssv600/mipi_boe_nt37700p.o

hisifb-objs += hisi_hiace_single_mode.o
EXTRA_CFLAGS += -DHIACE_SINGLE_MODE_SUPPORT
endif

ifeq ($(CONFIG_HISI_FB_V345),y)
hisifb-objs += \
	hisi_display_effect_dssv510.o \
	hisi_dpe_utils_dssv510.o \
	hisi_dpe_utils_clk_dssv510.o \
	hisi_dpe_clk_sub_dssv510.o \
	hisi_dpe_effect_utils.o \
	performance/hisi_dss_performance_v510.o \
	overlay/hisi_overlay_utils_dssv510.o \
	overlay/hisi_dss_aif_dssv510.o \
	overlay/hisi_dss_arsr_dssv510.o \
	overlay/hisi_dss_bce_bcd_dssv510.o \
	overlay/hisi_dss_clip_dssv510.o \
	overlay/hisi_dss_csc_dssv510.o \
	overlay/hisi_dss_mctl_dssv510.o \
	overlay/hisi_dss_mif_dssv510.o \
	overlay/hisi_dss_ovl_dssv510.o \
	overlay/hisi_dss_scf_dssv510.o \
	overlay/hisi_dss_scl_dssv510.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_fb_hisync.o \
	hisi_mipi_dsi_v510.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	backlight/ktz8864.o \
	panel/dssv330/mipi_lg_td4322.o \
	panel_old/dssv350/mipi_hx_nt36682c.o

ifeq ($(CONFIG_HISI_NOC_MODID_REGISTER),y)
hisifb-objs += \
	hisi_dss_mntn.o \
	hisi_dss_mntn_dssv350.o
endif
endif

ifeq ($(CONFIG_HISI_FB_V346),y)
hisifb-objs += \
	hisi_display_effect_dssv510.o \
	hisi_dpe_utils_dssv510.o \
	hisi_dpe_utils_clk_dssv510.o \
	hisi_dpe_clk_sub_dssv510.o \
	hisi_dpe_effect_utils.o \
	performance/hisi_dss_performance_v510.o \
	overlay/hisi_overlay_utils_dssv510.o \
	overlay/hisi_dss_aif_dssv510.o \
	overlay/hisi_dss_arsr_dssv510.o \
	overlay/hisi_dss_bce_bcd_dssv510.o \
	overlay/hisi_dss_clip_dssv510.o \
	overlay/hisi_dss_csc_dssv510.o \
	overlay/hisi_dss_mctl_dssv510.o \
	overlay/hisi_dss_mif_dssv510.o \
	overlay/hisi_dss_ovl_dssv510.o \
	overlay/hisi_dss_scf_dssv510.o \
	overlay/hisi_dss_scl_dssv510.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_fb_hisync.o \
	hisi_mipi_dsi_v510.o \
	hisi_spr_dsc.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	backlight/ktz8864.o \
	panel/dssv330/mipi_lg_td4322.o \
	panel_old/dssv346/mipi_tcl_nt36682c.o

endif

hisifb-objs += \
	panel_old/mipi_fake_panel.o \
	panel/hdmi_fake_panel.o

EXTRA_CFLAGS += -Idrivers/video/hisi/dss \
		-Idrivers/video/hisi/dss/overlay \
		-Idrivers/video/hisi/panel \
		-Idrivers/video/hisi \
		-Iinclude \
		-Idrivers/staging/android \
		-Idrivers/devkit/lcdkit/lcdkit1.0/include \
		-Idrivers/huawei_platform/inputhub/default

ifeq ($(CONFIG_TEE_TUI),y)
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
endif

ifeq ($(CONFIG_HISI_PERIDVFS),y)
EXTRA_CFLAGS += -Idrivers/clk/hisi/dvfs
endif

ifeq ($(CONFIG_LCD_KIT_DRIVER),y)
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include \
                -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/adapt \
                -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/include

hisifb-objs += backlight/lp8557.o
hisifb-objs += backlight/lp8556.o
hisifb-objs += backlight/rt8555.o
hisifb-objs += voltage/tps65132.o
hisifb-objs += backlight/lm36274.o
hisifb-objs += voltage/rt4801h.o
hisifb-objs += voltage/ina231.o
hisifb-objs += backlight/ktz8864.o
hisifb-objs += backlight/rt4831.o
hisifb-objs += backlight/nt50356.o
hisifb-objs += backlight/rt4539.o
hisifb-objs += voltage/icn68116.o
endif

ifeq ($(CONFIG_HUAWEI_DEVKIT_HISI),y)
EXTRA_CFLAGS += -DLV_GET_LCDBK_ON
endif

clean:
	rm *.o .*cmd
