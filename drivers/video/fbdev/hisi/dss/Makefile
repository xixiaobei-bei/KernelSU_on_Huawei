ifeq ($(CONFIG_FB),y)
obj-$(CONFIG_FB) := hisifb.o
endif

ifeq ($(CONFIG_SYNC_FILE),y)
obj-$(CONFIG_SYNC_FILE) += hisi_dss_sync.o
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
	hisi_fb_debug.o \
	hisi_mdc_resource.o \
	hisi_overlay_utils.o \
	hisi_overlay_online.o \
	hisi_overlay_offline.o \
	hisi_block_algorithm.o \
	hisi_overlay_cmdlist_utils.o \
	hisi_offlinecomposer_panel.o \
	panel/mipi_lcd_utils.o \
	hisi_display_effect.o \
	hisi_mmbuf_manager.o

hisifb-objs += \
	hisi_dss_ion.o \
	hisi_dss_iommu.o

ifeq ($(CONFIG_HISI_FB_970),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_dpe_utils_kirin970.o \
	hisi_display_effect_kirin970.o \
	hisi_overlay_utils_kirin970.o \
	hisi_mipi_dsi_v410.o \
	hisi_overlay_copybit.o \
	hisi_overlay_mediacommon.o \
	hisi_mediacommon_panel.o \
	hisi_sensorhub_aod.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	backlight/rt8555.o \
	panel/kirin970/mipi_cmi_NT35521.o\
	panel/kirin970/mipi_jdi_NT35695_cut3_1.o\
	panel/kirin970/mipi_lg_TD4322.o \
	panel/kirin970/mipi_lg_ER69006A.o\
	panel/kirin970/mipi_jdi_ER63450.o\
	panel/kirin970/mipi_sharp_NT36860.o\
	panel/kirin970/mipi_jdi_NT36860.o\
	panel/mipi_sharp_TD4322_6p0.o \
	panel/mipi_lg_TD4322_6p0.o \
	panel/mipi_jdi_R63452_6p0.o \
	panel/kirin970/mipi_jdi_NT36860C.o \
	panel/kirin970/mipi_jdi_2lane_NT36860C.o \
	panel/kirin970/mipi_lgd_NT36870.o \
	panel/kirin970/mipi_sharp_NT36870.o \
	panel/kirin970/mipi_lcd_external_panel.o

hisifb-objs += \
	dp/dp_aux.o \
	dp/avgen.o \
	dp/core.o \
	dp/intr.o \
	dp/edid.o \
	dp/link.o \
	dp/drm_dp_helper.o \
	panel/dp_fake_panel.o

hisifb-objs += hdcp22/host_lib_driver_linux_kernel.o \
			hdcp22/hdcp13.o

hisifb-objs += voltage/tps65132.o
hisifb-objs += voltage/rt4801h.o
hisifb-objs += voltage/ina231.o
endif

ifeq ($(CONFIG_HISI_FB_V501),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_dpe_utils_dssv501.o \
	hisi_display_effect_dssv501.o \
	hisi_overlay_utils_dssv501.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_mipi_dsi_v501.o \
	hisi_overlay_copybit.o \
	hisi_overlay_mediacommon.o \
	hisi_mediacommon_panel.o \
	hisi_fb_video_idle.o \
	hisi_ovl_online_wb.o \
	hisi_sensorhub_aod.o \
	panel/dssv501/mipi_cmi_NT35521.o \
	panel/dssv501/mipi_jdi_R63452.o \
	panel/kirin970/mipi_lg_TD4322.o \
	panel/kirin970/mipi_sharp_NT36870.o \
	panel/kirin970/mipi_lcd_external_panel.o

hisifb-objs += voltage/ina231.o
hisifb-objs += voltage/tps65132.o

hisifb-objs += \
	dp/dp_aux.o \
	dp/avgen.o \
	dp/core.o \
	dp/intr.o \
	dp/edid.o \
	dp/link.o \
	dp/drm_dp_helper.o \
	panel/dp_fake_panel.o

hisifb-objs += hdcp22/host_lib_driver_linux_kernel.o \
			hdcp22/hdcp13.o
endif

ifeq ($(CONFIG_HISI_FB_V510),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_dpe_utils_dssv510.o \
	hisi_display_effect_dssv510.o \
	hisi_overlay_utils_dssv510.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_mipi_dsi_v510.o \
	hisi_overlay_copybit.o \
	hisi_overlay_mediacommon.o \
	hisi_mediacommon_panel.o \
	hisi_fb_video_idle.o \
	hisi_sensorhub_aod.o \
	backlight/lm36923.o \
	panel/dssv501/mipi_cmi_NT35521.o \
	panel/dssv501/mipi_jdi_R63452.o \
	panel/kirin970/mipi_sharp_NT36870.o \
	panel/kirin970/mipi_samsung_EA8074.o \
	panel/kirin970/mipi_lcd_external_panel.o

hisifb-objs += voltage/ina231.o
hisifb-objs += voltage/tps65132.o

hisifb-objs += \
	dp/dp_aux.o \
	dp/avgen.o \
	dp/core.o \
	dp/intr.o \
	dp/edid.o \
	dp/link.o \
	dp/drm_dp_helper.o \
	panel/dp_fake_panel.o

hisifb-objs += hdcp22/host_lib_driver_linux_kernel.o \
			hdcp22/hdcp13.o
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
	panel/dssv320/mipi_tm_OTM1911A.o \
	panel/dssv320/mipi_cmi_NT35521.o

hisifb-objs += \
	panel/dp_fake_panel.o

hisifb-objs += voltage/tps65132.o
hisifb-objs += voltage/rt4801h.o
hisifb-objs += voltage/ina231.o
endif

ifeq ($(CONFIG_HISI_FB_V330),y)
hisifb-objs += \
	hisi_display_effect_dssv330.o \
	hisi_dpe_utils_dssv330.o \
	hisi_overlay_utils_dssv330.o \
	hisi_mipi_dsi_v330.o \
	hisi_sensorhub_aod.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	backlight/rt8555.o \
	panel/dssv330/mipi_lg_TD4322.o \
	panel/dssv330/mipi_boe_TD4320.o

hisifb-objs += \
	panel/dp_fake_panel.o
hisifb-objs += voltage/tps65132.o
hisifb-objs += voltage/rt4801h.o
hisifb-objs += voltage/ina231.o
endif

ifeq ($(CONFIG_HISI_FB_V350),y)
hisifb-objs += \
	hisi_display_effect_dssv330.o \
	hisi_dpe_utils_dssv350.o \
	hisi_overlay_utils_dssv350.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_mipi_dsi_v350.o \
	hisi_fb_video_idle.o \
	backlight/lm36274.o \
	backlight/lm36923.o \
	backlight/lp8556.o \
	panel/dssv330/mipi_lg_TD4322.o
endif

ifeq ($(CONFIG_HISI_FB_V600),y)
hisifb-objs += \
	hisi_dp.o \
	hisi_dp_hpd.o \
	hisi_fb_video_idle.o \
	hisi_dpe_utils_dssv510.o \
	hisi_display_effect_dssv510.o \
	hisi_overlay_utils_dssv510.o \
	hisi_dpe_pipe_clk_utils.o \
	hisi_mipi_dsi_v510.o \
	hisi_ovl_online_wb.o \
	hisi_overlay_copybit.o \
	hisi_overlay_mediacommon.o \
	hisi_mediacommon_panel.o \
	backlight/lm36923.o \
	panel/dssv330/mipi_lg_TD4322.o \
	panel/kirin970/mipi_lcd_external_panel.o

hisifb-objs += \
	dp/dp_aux.o \
	dp/avgen.o \
	dp/core.o \
	dp/intr.o \
	dp/edid.o \
	dp/link.o \
	dp/drm_dp_helper.o \
	panel/dp_fake_panel.o

hisifb-objs += \
	hdcp22/host_lib_driver_linux_kernel.o \
	hdcp22/hdcp13.o

EXTRA_CFLAGS += -DCONFIG_HISI_FB_V510
endif

hisifb-objs += \
	panel/lcdc_fake_panel.o \
	panel/mipi_fake_panel.o \
	panel/hdmi_fake_panel.o

EXTRA_CFLAGS += -Idrivers/video/hisi/dss \
		-Idrivers/video/hisi/panel \
		-Idrivers/video/hisi \
		-Iinclude \
		-Idrivers/staging/android \
		-Idrivers/devkit/lcdkit/lcdkit1.0/include

ifeq ($(CONFIG_TEE_TUI),y)
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
endif

ifeq ($(CONFIG_HISI_PERIDVFS),y)
EXTRA_CFLAGS += -Idrivers/clk/hisi/peri_dvfs
endif

ifeq ($(CONFIG_LCD_KIT_DRIVER),y)
EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include \
                -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/adapt \
                -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/include

hisifb-objs += backlight/lp8556.o
hisifb-objs += backlight/rt8555.o
hisifb-objs += voltage/tps65132.o
hisifb-objs += backlight/lm36274.o
hisifb-objs += voltage/rt4801h.o
hisifb-objs += backlight/ktz8864.o
hisifb-objs += backlight/rt4831.o
hisifb-objs += backlight/nt50356.o
endif

ifeq ($(CONFIG_HUAWEI_DEVKIT_HISI),y)
EXTRA_CFLAGS += -DLV_GET_LCDBK_ON
endif

clean:
	rm *.o .*cmd
