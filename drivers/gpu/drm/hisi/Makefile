
# Declare header File path
EXTRA_CFLAGS += \
    -Iinclude/drm \
    -Idrivers/gpu/drm/hisi \
    -Isound/soc/codecs/hisi \

# hisi LCD(DRM CONNECTOR) driver compile files
hisi-lcd-y := \
	hisi_lcd_drv.o \
	panel/hisi_drm_panel.o \
	lcd_kit_adapt.o
obj-$(CONFIG_DRM_HISI_LCD) += hisi-lcd.o

ifeq ($(CONFIG_DRM_SMMU_V3),y)
obj-$(CONFIG_DRM_SMMU_V3) += hisi_drm_smmuv3.o
endif

# hisi DP(DRM CONNECTOR) driver compile files
hisi-dp-y := \
	hisi_dp_drv.o \
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
	dsc/dsc_algorithm.o \
	dp/controller/dsc/hidpc_dsc_config.o \
	dp/controller/dsc/dsc_config_base.o \
	dp/link/dp_dsc_algorithm.o \
	dsc/utilities/hisi_debug.o \
	dsc/dsc_algorithm_manager.o

obj-$(CONFIG_DRM_HISI_DP) += hisi-dp.o

# hisi backlight driver compile files
hisi-bl-y := \
	hisi_blpwm.o \
	backlight/lm36923.o \
	backlight_sysfs.o
obj-$(CONFIG_DRM_HISI_BL) += hisi-bl.o

# hisi drm driver compile files
hisi-drm-y := \
	hisi_fb.o \
	hisi_drm_drv.o \
	hisi_drm_utils.o \
	hisi_overlay_cmdlist_utils.o \
	hisi_gem_dmabuf.o

obj-$(CONFIG_DRM_HISI) += hisi-drm.o

# hisi display controller(dss/CRTC) driver compile files
hisi-dss-y := \
	hisi_dpe.o \
	hisi_drm_dpe.o \
	hisi_drm_plane.o \
	hisi_drm_dss_dumper.o \
	hisi_drm_dss_init.o \
	hisi_drm_dss_plane_update.o \
	hisi_drm_dss_rdma_configure.o \
	hisi_drm_dss_set_regs.o \
	hisi_drm_dss_global.o \
	hisi_drm_overlay.o \
	hisi_dpe_utils.o
obj-$(CONFIG_DRM_HISI_DPE) += hisi-dss.o

hisi-dss-iommu-y := \
         hisi_fbdev.o \
         hisi_drm_gem.o \
         hisi_drm_gem_heap_helper.o \
         hisi_dss_ion.o \
         hisi_dss_iommu.o \
         hisi_mmbuf_manager.o

obj-$(CONFIG_DRM_DSS_IOMMU) += hisi-dss-iommu.o

# hisi dsi(DRM ENCODER) driver compile files
hisi-dsi-y := \
	hisi_drm_dsi.o \
	hisi_mipi_dsi.o \
	hisi_panel.o \
	bridge/edp_bridge_register.o \
	bridge/edp_bridge.o \
	bridge/sn65dsix6.o \
	bridge/lt9711a.o \
	bridge/desktop_bridge.o \
	bridge/laptop_bridge.o \
	bridge/rtd2172.o
obj-$(CONFIG_DRM_HISI_DSI) += hisi-dsi.o

obj-$(CONFIG_DRM_GEM_HEAP) += heap/

hisi-dss-v510-y := \
	hisi_dpe_clk_sub_dssv510.o \
	hisi_mipi_dsi_v510.o
obj-$(CONFIG_HISI_DSS_V510) += hisi-dss-v510.o

hisi-dss-v600-y := \
	hisi_dpe_clk_sub_dssv600.o \
	hisi_mipi_dsi_v510.o
obj-$(CONFIG_HISI_DSS_V600) += hisi-dss-v600.o

# hisi drm debug compile files
hisi-drm-debug-y := \
	hisi_drm_debug.o
obj-$(CONFIG_DRM_HISI_DEBUG) += hisi-drm-debug.o

ifeq ($(CONFIG_TEE_TUI),y)
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
endif

ifeq ($(CONFIG_HISI_PERIDVFS),y)
EXTRA_CFLAGS += -Idrivers/clk/hisi/dvfs
endif

ifeq ($(CONFIG_LCD_KIT_DRIVER),y)
EXTRA_CFLAGS += \
    -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/common/include \
    -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/adapt \
    -Idrivers/devkit/lcdkit/lcdkit3.0/kernel/hisi/include
endif

clean:
	rm *.o .*cmd
