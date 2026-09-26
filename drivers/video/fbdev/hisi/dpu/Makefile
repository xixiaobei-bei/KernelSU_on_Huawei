obj-$(CONFIG_CMDLIST_ENABLE) := common/cmdlist/
obj-$(CONFIG_HISI_DPU_DISP) += dpu_disp.o

dpu_disp-objs := \
	device/chrdev/hisi_chrdev.o \
	effect/hisi_disp_dpp.o \
	resource_manager/hisi_disp_rm.o \
	resource_manager/hisi_disp_rm_operator.o \
	composer/operators/sdma/hisi_operator_sdma.o \
	composer/operators/cld/dpu_disp_cld.o \
	composer/operators/srot/dpu_disp_srot.o \
	composer/operators/itfsw/hisi_operator_itfsw.o \
	composer/operators/overlayer/hisi_operator_overlayer.o \
	composer/operators/hisi_operators_manager.o \
	composer/operators/mcu/hisi_disp_mcu.o \
	composer/operators/dpp/hisi_operator_dpp.o \
	composer/operators/scl/vscf/dpu_operator_vscf.o \
	composer/operators/scl/arsr/dpu_operator_arsr.o \
	composer/operators/scl/dpu_scl.o \
	composer/operators/widget/dpu_csc.o \
	composer/operators/widget/dpu_dfc.o \
	composer/operators/widget/dpu_post_clip.o \
	composer/operators/widget/dpu_scf.o \
	composer/operators/widget/dpu_scf_lut.o \
	composer/operators/widget/hebc/dpu_hebcd.o \
	composer/operators/dsc/dsc_algorithm.o \
	composer/operators/dsc/utilities/hisi_debug.o \
	composer/operators/dsc/dsc_algorithm_manager.o \
	composer/operators/dsc/dpu_operator_dsc.o \
	composer/operators/block/dpu_block_algorithm.o \
	composer/operators/wch/dpu_operator_wch.o \
	composer/operators/wch/dpu_wch_wdma.o \
	composer/operators/wch/dpu_wch_dither.o \
	composer/operators/wch/dpu_wch_post_clip.o \
	composer/operators/wch/dpu_wch_dfc.o \
	composer/operators/wch/dpu_wch_scf.o \
	composer/operators/wch/dpu_wch_csc.o \
	composer/operators/utils/hisi_operator_tool.o \
	composer/core/isr/hisi_isr_primary.o \
	composer/core/isr/hisi_isr_offline.o \
	composer/core/isr/hisi_isr_utils.o \
	composer/core/timeline/hisi_disp_timeline_listener.o \
	composer/core/timeline/hisi_disp_timeline.o \
	composer/core/fence/hisi_disp_fence.o \
	composer/core/layer_buf/hisi_layer_buf.o \
	composer/core/online/hisi_composer_online_base.o \
	composer/core/offline/hisi_composer_offline_base.o \
	composer/utils/hisi_comp_utils.o \
	composer/utils/notify/hisi_comp_notify.o \
	composer/hisi_composer_primary.o \
	composer/hisi_composer_offline.o \
	composer/hisi_tmp_fb2.o \
	composer/hisi_offline_adaptor.o \
	composer/init/hisi_composer_init.o \
	composer/core/vsync/hisi_disp_vactive.o \
	panel/hisi_disp_panel_mipi_fake.o \
	common/hisi_dpu_module.o \
	common/utils/hisi_drv_kthread.o \
	panel/hisi_disp_panel.o \
	common/utils/hisi_disp_gadgets.o \
	common/utils/hisi_drv_utils.o \
	common/config/hisi_disp_config.o \
	connector/mipi/hisi_mipi_slaver_base.o \
	connector/mipi/hisi_drv_mipi.o \
	connector/mipi/hisi_mipi_utils.o \
	connector/mipi/hisi_drv_mipi_chrdev.o \
	connector/mipi/hisi_mipi_master_base.o \
	connector/mipi/hisi_mipi_dsi_host.o \
	connector/utils/hisi_connector_utils.o \
	connector/mipi/hisi_mipi_dsi_v700.o \
	lights/hisi_disp_bl.o \
	smmu/hisi_disp_smmu.o \
	gralloc/hisi_disp_iommu.o \
	power_manager/hisi_disp_pm.o \
	power_manager/dacc/hisi_disp_dacc.o \
	power_manager/low_power/hisi_disp_pm_low_power.o \
	power_manager/low_power/hisi_disp_pm_low_power_dssv700.o

dpu_disp-objs += \
	panel/mipi_lg_TD4322.o

dpu_disp-objs += \
	composer/hisi_composer_external.o \
	connector/dp/hisi_drv_dp.o \
	connector/dp/hisi_dp_utils.o \
	connector/dp/hisi_drv_dp_chrdev.o \
	connector/dp/hisi_dp_master_base.o \
	connector/dp/hisi_dp_dbg.o \
	connector/dp/controller/dp_avgen_base.o \
	connector/dp/controller/hidptx/hidptx_dp_aux.o \
	connector/dp/controller/hidptx/hidptx_dp_irq.o \
	connector/dp/controller/hidptx/hidptx_dp_avgen.o \
	connector/dp/controller/hidptx/hidptx_dp_core.o \
	connector/dp/controller/dsc/hidptx_dsc_config.o \
	connector/dp/controller/dsc/dsc_config_base.o \
	connector/dp/phy/hidptx_phy_config.o \
	connector/dp/link/dp_irq.o \
	connector/dp/link/dp_aux.o \
	connector/dp/link/dp_edid.o \
	connector/dp/link/dp_link_training.o \
	connector/dp/link/dp_mst_topology.o \
	connector/dp/link/dp_dsc_algorithm.o \
	connector/dp/link/drm_dp_helper.o \
	panel/hisi_disp_panel_dp_fake.o

ifeq ($(CONFIG_HISI_DISP_FB),y)
dpu_disp-objs += \
	device/fbdev/primary/hisi_primary_fb.o \
	device/fbdev/primary/hisi_primary_fb_base.o \
	device/fbdev/hisi_fb_pm.o \
	device/fbdev/hisi_fb_sysfs.o \
	device/fbdev/hisi_fb_vsync.o \
	device/fbdev/hisi_disp_fake_fb.o
endif

ifeq ($(CONFIG_HISI_DISP_ADAPTOR_ENABLE),y)
dpu_disp-objs += \
	device/fbdev/adaptor/hisi_fb_adaptor.o
endif

ifeq ($(CONFIG_HISI_DISP_TEST_ENABLE),y)
dpu_disp-objs += debug_test/test_drv/hisi_drv_test.o
endif

ifeq ($(CONFIG_HISI_DISP_DEBUG),y)
dpu_disp-objs += debug_test/debug/hisi_disp_debug.o
endif

dpu_disp-objs += resource_manager/hisi_disp_rm_debug.o

EXTRA_CFLAGS += -DDEBUG_ALLOC_RESOURCE_FROM_KERNEL
EXTRA_CFLAGS += -DCONFIG_OFFLINE_DBG
EXTRA_CFLAGS += -fPIC
# EXTRA_CFLAGS += -DDEBUG_ENABLE_CPU_SET_REG
# EXTRA_CFLAGS += -DCONFIG_HISI_FB_COLORBAR_USED

EXTRA_CFLAGS += -Idrivers/video/fbdev/hisi/dpu \
	-Idrivers/video/fbdev/hisi/dpu/device \
	-Idrivers/video/fbdev/hisi/dpu/device/fbdev \
	-Idrivers/video/fbdev/hisi/dpu/device/chrdev \
	-Idrivers/video/fbdev/hisi/dpu/composer \
	-Idrivers/video/fbdev/hisi/dpu/composer/core \
	-Idrivers/video/fbdev/hisi/dpu/composer/utils \
	-Idrivers/video/fbdev/hisi/dpu/composer/init \
	-Idrivers/video/fbdev/hisi/dpu/composer/operators \
	-Idrivers/video/fbdev/hisi/dpu/composer/operators/utils \
	-Idrivers/video/fbdev/hisi/dpu/panel \
	-Idrivers/video/fbdev/hisi/dpu/smmu \
	-Idrivers/video/fbdev/hisi/dpu/power_manager \
	-Idrivers/video/fbdev/hisi/dpu/power_manager/dacc \
	-Idrivers/video/fbdev/hisi/dpu/power_manager/low_power \
	-Idrivers/video/fbdev/hisi/dpu/resource_manager \
	-Idrivers/video/fbdev/hisi/dpu/common \
	-Idrivers/video/fbdev/hisi/dpu/common/utils \
	-Idrivers/video/fbdev/hisi/dpu/common/cmdlist \
	-Idrivers/video/fbdev/hisi/dpu/common/config \
	-Idrivers/video/fbdev/hisi/dpu/connector/mipi \
	-Idrivers/video/fbdev/hisi/dpu/connector/dp \
	-Idrivers/video/fbdev/hisi/dpu/connector/utils \
	-Idrivers/video/fbdev/hisi/dpu/debug_test/test_drv \
	-Idrivers/video/fbdev/hisi/dpu/debug_test/debug \
	-Idrivers/video/fbdev/hisi/dpu/lights \
	-Idrivers/video/fbdev/hisi/dpu/gralloc \
	-Iinclude

clean:
	rm *.o .*cmd
