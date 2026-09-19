#
# Makefile for the modem drivers.
#
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(CONFIG_HISI_BALONG_MODEM)),m)
drv-y :=
drv-builtin :=

subdir-ccflags-y+= -Idrivers/hisi/modem/drv/icc
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/rtc
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/nvim
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/mem
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/om \
                   -Idrivers/hisi/modem/drv/om/common \
                   -Idrivers/hisi/modem/drv/om/dump \
                   -Idrivers/hisi/modem/drv/om/log
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/udi
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/balong_timer
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/hds
subdir-ccflags-y+= -Idrivers/usb/gadget
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/debug \
                   -Idrivers/hisi/modem/drv/diag/ppm \
                   -Idrivers/hisi/modem/drv/diag/soft_decode
subdir-ccflags-y+= -Idrivers/hisi/modem/include/tools

drv-builtin		+= buildin/buildin.o

ifneq ($(strip $(CFG_ATE_VECTOR)),YES)
drv-y           += adp/adp_ipc.o
drv-y           += adp/adp_icc.o
drv-y           += adp/adp_pm_om.o
drv-y           += adp/adp_version.o
drv-y           += adp/adp_socp.o
drv-y           += adp/adp_onoff.o
drv-y           += adp/adp_om.o
drv-$(CONFIG_USB_GADGET)     += adp/adp_usb.o
drv-$(CONFIG_RFILE_SUPPORT)  += adp/adp_rfile.o
drv-y           += adp/adp_nvim.o
drv-y           += adp/adp_reset.o
drv-y           += adp/adp_efuse.o
drv-y           += adp/adp_timer.o
drv-y           += adp/adp_wifi.o
drv-y           += adp/adp_mailbox.o
drv-y           += adp/adp_sec.o
drv-y           += adp/adp_mem_balong.o
drv-y           += adp/adp_cpufreq_balong.o
drv-y           += adp/adp_applog.o
drv-y           += adp/adp_hds.o
ifeq ($(filter YES,$(CFG_BSP_CONFIG_PHONE_TYPE) $(CFG_CONFIG_EMMC_BOOT)),YES)
drv-y       += adp/adp_blk_mmc.o
else
drv-y       += adp/adp_blk_mtd.o
endif
ifeq ($(strip $(CFG_CONFIG_CORESIGHT)),YES)
drv-y           += adp/adp_coresight.o
endif
drv-y           += adp/adp_charger.o
drv-y           += adp/adp_mmc.o
drv-y           += adp/adp_dload.o
drv-y           += adp/adp_misc.o
drv-y           += adp/adp_vic.o
else
drv-y           += adp/adp_pm_om.o
drv-y           += adp/adp_reset.o
drv-y           += adp/adp_timer.o
drv-y           += adp/adp_vic.o
endif

ifeq ($(strip $(CFG_FEATURE_HISOCKET)),FEATURE_ON)
drv-y           += adp/hisocket.o
endif
drv-y           += adp/adp_diag_system.o
drv-y           += adp/adp_slt.o

ifeq ($(strip $(CFG_CONFIG_APPLOG)),YES)
drv-y       += applog/applog_balong.o
ifeq ($(strip $(CFG_CONFIG_MODULE_BUSSTRESS)),YES)
#drv-y       += applog/applog_balong_test.o
else
drv-$(CONFIG_ENABLE_TEST_CODE) += applog/applog_balong_test.o
endif
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
drv-y   += balong_timer/timer_slice.o
drv-y   += balong_timer/hardtimer_arm.o
drv-y   += balong_timer/hardtimer_core.o
drv-y   += balong_timer/softtimer_balong.o
endif

drv-$(CONFIG_BBP_ACORE) += bbp/bbp_balong.o

subdir-ccflags-y += -Idrivers/hisi/modem/drv/hds
ifeq ($(strip $(CFG_BOARD_TRACE)),YES)
drv-y     += board_trace/board_trace_balong.o
endif

ifeq ($(strip $(CFG_CONFIG_CSHELL)),YES)
drv-y += console/ringbuffer.o
drv-y += console/console.o
drv-y += console/virtshell.o
drv-y += console/cshell_port.o
drv-y += console/uart_dev.o
drv-y += console/con_platform.o
drv-y += console/cshell_logger.o
endif

ifneq ($(strip $(OBB_SEPARATE)),true)
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/ppm \
                   -Idrivers/hisi/modem/drv/diag/debug
subdir-ccflags-y+= -Idrivers/hisi/modem/include/tools

drv-y           += diag/cpm/diag_port_manager.o
drv-y           += diag/debug/diag_system_debug.o
drv-y           += diag/ppm/OmCommonPpm.o
drv-y           += diag/ppm/OmHsicPpm.o
drv-y           += diag/ppm/OmSocketPpm.o
drv-y           += diag/ppm/OmUsbPpm.o
drv-y           += diag/ppm/OmVcomPpm.o

drv-y           += diag/scm/scm_ind_dst.o
drv-y           += diag/scm/scm_ind_src.o
drv-y           += diag/scm/scm_cnf_dst.o
drv-y           += diag/scm/scm_cnf_src.o
drv-y           += diag/scm/scm_common.o
drv-y           += diag/scm/scm_debug.o
drv-y           += diag/scm/scm_init.o

ifeq ($(strip $(CFG_CONFIG_MODULE_BUSSTRESS)),YES)
#drv-y          += diag_frame/diag_frame_test.o
else
drv-$(CONFIG_ENABLE_TEST_CODE) += diag_frame/diag_frame_test.o
endif

ifeq ($(strip $(CFG_CONFIG_DIAG_NETLINK)),YES)
drv-y           += diag_vcom/diag_vcom_main.o
drv-y           += diag_vcom/diag_vcom_handler.o
endif
endif
endif

drv-y           += diag_frame/diag_frame.o

ifeq ($(strip $(CFG_CONFIG_DLOCK)),YES)
drv-y           += dlock/dlock_balong.o
endif
 
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)

drv-y           += dump/comm/dump_apr.o
drv-y           += dump/comm/dump_config.o
drv-y           += dump/comm/dump_core.o
drv-y           += dump/comm/dump_exc_tpye.o
drv-y           += dump/comm/dump_file.o
drv-y           += dump/comm/dump_logs.o
drv-y           += dump/mdmap/dump_area.o
drv-y           += dump/mdmap/dump_baseinfo.o
drv-y           += dump/mdmap/dump_log_save.o
drv-y           += dump/mdmap/dump_mdmap_core.o
drv-y           += dump/mdmcp/dump_cp_agent.o
drv-y           += dump/mdmcp/dump_cp_core.o
drv-y           += dump/mdmcp/dump_cp_wdt.o
drv-y           += dump/mdmcp/dump_cphy_tcm.o
drv-y           += dump/mdmcp/dump_easyrf_tcm.o
drv-y           += dump/mdmcp/dump_lphy_tcm.o
drv-y           += dump/mdmcp/dump_sec_mem.o
endif

subdir-ccflags-y+= -Idrivers/hisi/modem/drv/dump/comm \
                   -Idrivers/hisi/modem/drv/dump/mdmap \
                   -Idrivers/hisi/modem/drv/dump/mdmcp

ifeq ($(strip $(CFG_CONFIG_ECDC)),YES)
drv-y           += ecdc/ecdc_serviceman.o
drv-y           += ecdc/ecdc_genlserver.o
drv-y           += ecdc/ecdc_panrpc.o
drv-y           += ecdc/llt_distribute.o
endif

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
drv-$(CONFIG_EFUSE_BALONG)      += efuse/efuse_balong.o
drv-$(CONFIG_EFUSE_BALONG_AGENT)    += efuse/efuse_balong_agent.o
endif

ifeq ($(strip $(CFG_CONFIG_EICC)),YES)
drv-y               += eicc/eicc_driver.o
drv-y               += eicc/eicc_acpu.o
drv-y               += eicc/eicc_core.o
drv-y               += eicc/eicc_interface.o
drv-y               += eicc/eicc_test.o
endif

ifeq ($(strip $(CONFIG_BALONG_ESPE)),y)
drv-y               += espe/spe_core.o espe/spe_platform_balong.o espe/spe_desc.o espe/spe_entry.o espe/spe_port.o espe/spe_dbg.o
drv-$(CONFIG_DEBUG_FS)          += espe/spe_debugfs.o
drv-$(CONFIG_BALONG_ESPE_XFRM)  += espe/spe_xfrm.o
drv-y                           += espe/nic/espe_nic.o
drv-y                           += espe/nic/espe_ncm.o
endif

ifeq ($(strip $(CFG_CONFIG_CCPU_FIQ_SMP)),YES)
drv-y           += fiq/fiq_smp.o
else
drv-y           += fiq/fiq.o
endif

drv-y               += hds/bsp_hds_service.o hds/bsp_hds_log.o hds/bsp_hds_ind.o

subdir-ccflags-y += -Idrivers/hisi/modem/drv/om/common \
                    -Idrivers/hisi/modem/drv/om/oms\
                    -Idrivers/hisi/modem/drv/om/log \
                    -Idrivers/hisi/modem/drv/om/sys_view \
                    -Idrivers/modem/drv/om/usbtrace

drv-$(CONFIG_HW_IP_BASE_ADDR)   += hwadp/hwadp_balong.o hwadp/hwadp_core.o hwadp/hwadp_debug.o

subdir-ccflags-y += -Idrivers/hisi/tzdriver
drv-$(CONFIG_ICC_BALONG)        += icc/icc_core.o icc/icc_linux.o icc/icc_debug.o
drv-$(CONFIG_CA_ICC)            += icc/ca_icc.o
drv-$(CONFIG_ENABLE_TEST_CODE)  += icc/icc_test.o

ifeq ($(strip $(CFG_CONFIG_MODULE_IPC)),YES)
drv-$(CONFIG_IPC_DRIVER)        += ipc/ipc_balong.o
drv-$(CONFIG_ENABLE_TEST_CODE)  += ipc/ipc_balong_test.o
endif

ifneq ($(strip $(CFG_CONFIG_NEW_PLATFORM)),YES)
drv-$(CONFIG_IPF_SUPPORT)       += ipf/ipf_balong.o
endif

ifeq ($(strip $(OBB_LLT_MDRV)),y)
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
drv-y += llt_tools/llt_gcov.o
endif
ifeq ($(strip $(LLTMDRV_HUTAF_COV)),true)
drv-y += llt_tools/ltcov.o
endif
drv-y += llt_tools/llt_tool.o
endif

ifeq ($(strip $(CFG_FEATURE_TDS_WCDMA_DYNAMIC_LOAD)),FEATURE_ON)
drv-y       += load_ps/bsp_loadps.o
endif

ifneq ($(strip $(CFG_CONFIG_MLOADER)),YES)
drv-y           += loadm/load_image.o
ifeq ($(strip $(CFG_FEATURE_DELAY_MODEM_INIT)),FEATURE_ON)
drv-y           += loadm/loadm_phone.o
else
drv-y           += loadm/loadm_mbb.o
endif
endif

ifeq ($(strip $(CFG_CONFIG_COLD_PATCH)),YES)
drv-y           += loadm/modem_cold_patch.o
else
drv-y           += loadm/modem_cold_patch_stub.o
endif

ifeq ($(bbit_type),nr)
subdir-ccflags-y += -DBBIT_TYPE_NR=FEATURE_ON
endif

ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
drv-y               += log/bsp_om_log.o
drv-y               += adp/adp_print.o
endif

ifeq ($(strip $(CFG_CONFIG_MAA_BALONG)),YES)
drv-y               += maa/maa_acore.o maa/bsp_maa.o maa/maa_acore_test.o
endif
drv-y               += mbb_modem_stub/mbb_modem_stub.o

ifeq ($(strip $(CONFIG_MEM_BALONG)),y)
drv-$(CONFIG_MEM_BALONG)    += mem/mem_balong.o
drv-$(CONFIG_ENABLE_TEST_CODE) += mem/mem_balong_test.o
endif

ifeq ($(strip $(CFG_CONFIG_MLOADER)),YES)
drv-y           += mloader/mloader_comm.o
drv-y           += mloader/mloader_load_image.o
ifeq ($(strip $(CFG_CONFIG_LOAD_SEC_IMAGE)),YES)
drv-y           += mloader/mloader_load_image_sec.o
else
drv-y           += mloader/mloader_load_image_unsec.o
endif
endif

drv-y           += mperf/s_mperf.o

ifeq ($(strip $(CFG_CONFIG_NEW_PLATFORM)),YES)
drv-$(CONFIG_IPF_SUPPORT)           += n_ipf/ipf_balong.o n_ipf/ipf_pm.o n_ipf/ipf_filter.o n_ipf/ipf_desc.o n_ipf/ipf_desc64.o n_ipf/ipf_debug.o
drv-$(CONFIG_PSAM_SUPPORT)          += n_psam/psam_balong.o n_psam/psam_hal32.o n_psam/psam_hal64.o n_psam/psam_debug.o
endif

drv-y           += net_helper/ip_limit.o

ifeq ($(strip $(CFG_CONFIG_NRDSP)),YES)
drv-y           += nrdsp/nrdsp_dump_tcm.o
endif

EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/modem/include/nv/product/
ifneq ($(strip $(CFG_FEATURE_NV_SEC_ON)),YES)
ifeq ($(strip $(CFG_FEATURE_NV_FLASH_ON)),YES)
drv-$(CONFIG_NVIM)          += nvim/nv_flash.o
endif
ifeq ($(strip $(CFG_FEATURE_NV_EMMC_ON)),YES)
drv-$(CONFIG_NVIM)          += nvim/nv_emmc.o
endif
drv-$(CONFIG_NVIM)          += nvim/nv_ctrl.o nvim/nv_comm.o nvim/nv_base.o nvim/nv_cust.o nvim/nv_xml_dec.o nvim/nv_debug.o nvim/nv_crc.o nvim/nv_index.o nvim/nv_partition_upgrade.o nvim/nv_partition_img.o nvim/nv_partition_bakup.o nvim/nv_factory_check.o nvim/nv_msg.o nvim/nv_proc.o
endif

drv-$(CONFIG_PMIC_OCP)      += ocp/pmic_ocp.o

ifeq ($(strip $(CFG_CONFIG_PHONE_DCXO_AP)),YES)
drv-y           += ocp/pmic_dcxo.o
endif

drv-$(CONFIG_BALONG_ONOFF)  += onoff/power_para.o
drv-$(CONFIG_BALONG_MODEM_ONOFF)    += onoff/power_exchange.o
drv-$(CONFIG_BALONG_MODEM_ONOFF)    += onoff/power_on.o
drv-$(CONFIG_BALONG_MODEM_ONOFF)    += onoff/power_off.o
drv-$(CONFIG_BALONG_MODEM_ONOFF)    += onoff/bsp_modem_boot.o

drv-$(CONFIG_PM_OM_BALONG) +=  pm_om/pm_om.o pm_om/pm_om_platform.o pm_om/pm_om_debug.o pm_om/pm_om_pressure.o pm_om/bsp_ring_buffer.o pm_om/modem_log_linux.o
drv-$(CONFIG_PM_OM_BALONG_TEST) +=  pm_om/pm_om_test.o

ifneq ($(strip $(CFG_CONFIG_NEW_PLATFORM)),YES)
drv-$(CONFIG_PSAM_SUPPORT)      += psam/psam_balong.o
endif

drv-$(CONFIG_BALONG_MODEM_RESET)+= reset/reset_balong.o

ifeq ($(strip $(CFG_CONFIG_SEC_RFILE)),YES)
drv-y += rfile/sec_rfile_balong.o
endif
drv-$(CONFIG_RFILE_SUPPORT)     += rfile/rfile_balong.o
drv-$(CONFIG_ENABLE_TEST_CODE)  += rfile/rfile_api_test.o

drv-y += s_memory/s_memory.o
drv-$(CONFIG_S_MEMORY_TEST)     += s_memory/s_memory_test.o

ifeq ($(strip $(CFG_CONFIG_SC)),YES)
drv-y                           += sc/sc_balong.o
endif

drv-$(CONFIG_SEC_CALL)          += sec_call/sec_call.o

ifeq ($(strip $(CFG_FEATURE_NV_SEC_ON)),YES)
drv-$(CONFIG_NVIM)              += sec_nvim/nv_ctrl.o sec_nvim/nv_comm.o sec_nvim/nv_base.o sec_nvim/nv_emmc.o sec_nvim/nv_debug.o sec_nvim/nv_index.o sec_nvim/nv_partition_upgrade.o sec_nvim/nv_partition_img.o sec_nvim/nv_partition_bakup.o sec_nvim/nv_partition_factory.o sec_nvim/nv_factory_check.o sec_nvim/nv_msg.o sec_nvim/nv_proc.o sec_nvim/nv_verify.o
endif

drv-$(CONFIG_HISI_SIM_HOTPLUG)          += sim_hotplug/hisi_sim_hotplug.o
drv-$(CONFIG_HISI_SIM_HOTPLUG_SPMI)     += sim_hotplug/hisi_sim_hotplug_spmi.o

subdir-ccflags-y    +=  -Idrivers/hisi/modem/drv/socp/soft_decode
ifeq ($(strip $(CFG_ENABLE_BUILD_SOCP)),YES)
ifeq ($(strip $(CFG_DIAG_SYSTEM_5G)),YES)
drv-y       += socp/socp_enc.o
else
drv-y       += socp/socp_balong.o
endif
drv-y       += socp/socp_ind_delay.o socp/socp_debug.o
ifeq ($(strip $(CFG_CONFIG_DEFLATE)),YES)
drv-y       += socp/deflate.o
endif
ifneq ($(strip $(OBB_SEPARATE)),true)
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
drv-y       += socp/soft_decode/hdlc.o
drv-y       += socp/soft_decode/ring_buffer.o
drv-y       += socp/soft_decode/soft_decode.o
drv-y       += socp/soft_decode/soft_cfg.o
endif
endif
endif

drv-$(CONFIG_SYNC_BALONG)       += sync/sync_balong.o
drv-$(CONFIG_ENABLE_TEST_CODE)  += sync/sync_balong_test.o

ifeq ($(strip $(CFG_CONFIG_SYSBUS)),YES)
drv-y += sys_bus/sys_bus_core.o
drv-y += sys_bus/sys_bus_single.o
drv-y += sys_bus/sys_bus_pressure.o
drv-y += sys_bus/sys_monitor.o
endif

drv-y += sysboot/sysboot_balong.o
drv-y += sysctrl/sysctrl.o
drv-y += thermal/thermal.o

drv-$(CONFIG_UDI_SUPPORT)   += udi/udi_balong.o udi/adp_udi.o

drv-y       += version/version_balong.o

ifeq ($(strip $(CFG_CONFIG_WAN)),YES)
drv-y       += wan/ipf_ap.o wan/ipf_ap_test.o wan/wan.o
endif

ifeq ($(strip $(CFG_CONFIG_ZSTD_DECOMPRESS)),YES)
drv-y       += zstd/entropy_common.o
drv-y       += zstd/error_private.o
drv-y       += zstd/xxhash.o
drv-y       += zstd/zstd_common.o
drv-y       += zstd/zstd_decompress.o
drv-y       += zstd/fse_decompress.o
drv-y       += zstd/huf_decompress.o
endif

# module makefile end
else

ifneq ($(strip $(CFG_ATE_VECTOR)),YES)
obj-y               += sysctrl/
obj-y               += s_memory/
obj-y               += mperf/
obj-y               += fiq/
ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
obj-y               += balong_timer/
endif
ifeq ($(strip $(CFG_CONFIG_MODULE_IPC)),YES)
obj-y               += ipc/
endif
obj-y               += icc/
obj-y               += reset/
obj-y               += sec_call/
ifeq ($(strip $(CFG_CONFIG_EICC)),YES)
obj-y               += eicc/
endif

ifeq ($(strip $(CFG_CONFIG_ECDC)),YES)
obj-y               += ecdc/
endif
obj-$(CONFIG_RFILE_SUPPORT) += rfile/
ifeq ($(strip $(CFG_FEATURE_TDS_WCDMA_DYNAMIC_LOAD)),FEATURE_ON)
obj-y               += load_ps/
endif
obj-y               += sync/
ifeq ($(strip $(CFG_ENABLE_BUILD_SOCP)),YES)
obj-y           += socp/
endif
ifeq ($(strip $(CFG_CONFIG_NRDSP)),YES)
obj-y               += nrdsp/
endif
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y               += dump/
endif
ifneq ($(strip $(CFG_FEATURE_NV_SEC_ON)),YES)
obj-y               += nvim/
endif
ifeq ($(strip $(CFG_FEATURE_NV_SEC_ON)),YES)
obj-y               += sec_nvim/
endif

ifeq ($(strip $(CFG_CONFIG_MLOADER)),YES)
obj-y += mloader/
else
obj-$(CONFIG_IMAGE_LOAD) += loadm/
endif

ifeq ($(strip $(CFG_CONFIG_ZSTD_DECOMPRESS)),YES)
obj-y                   += zstd/
endif

obj-y           += adp/
obj-y           += hwadp/
obj-y           += version/
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y           += log/
endif
ifneq ($(strip $(OBB_SEPARATE)),true)
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
obj-y               += diag/
ifeq ($(strip $(CFG_CONFIG_DIAG_NETLINK)),YES)
obj-y               += diag_vcom/
endif
endif
endif
obj-y           += hds/


ifeq ($(strip $(CFG_BOARD_TRACE)),YES)
obj-y           += board_trace/
endif

ifeq ($(strip $(CFG_CONFIG_SYSBUS)),YES)
obj-y           += sys_bus/
endif

ifeq ($(strip $(CFG_CONFIG_MAA_BALONG)),YES)
obj-y           += maa/
endif


obj-$(CONFIG_BALONG_ESPE)   += espe/

ifeq ($(strip $(CFG_CONFIG_NEW_PLATFORM)),YES)
obj-y           += n_ipf/
obj-y           += n_psam/
else
obj-y           += ipf/
obj-y           += psam/
endif

obj-y           += udi/
obj-y           += thermal/

obj-$(CONFIG_MEM_BALONG)    += mem/

obj-y   += onoff/

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
obj-y   += efuse/
endif

ifeq ($(strip $(CFG_CONFIG_CSHELL)),YES)
obj-y += console/
endif

ifeq ($(strip $(CFG_CONFIG_SC)),YES)
obj-y               += sc/
endif
obj-y           += pm_om/

obj-$(CONFIG_PMIC_OCP) += ocp/

# llt module
ifeq ($(strip $(OBB_LLT_MDRV)),y)
obj-y           += llt_tools/
endif

ifeq ($(strip $(llt_gcov)),y)
subdir-ccflags-y += -fno-inline
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_BUSSTRESS)),YES)
obj-y                   += busstress/$(OBB_PRODUCT_NAME)/
endif

obj-$(CONFIG_HISI_SIM_HOTPLUG)          += sim_hotplug/
obj-$(CONFIG_HISI_SIM_HOTPLUG_SPMI)     += sim_hotplug/

else
obj-y               += sysctrl/
obj-y               += s_memory/
obj-y               += mperf/
ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
obj-y               += balong_timer/
endif
obj-y               += icc/
obj-y               += reset/
obj-y               += adp/
obj-y               += hwadp/
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y               += om/
endif
obj-y               += pm_om/
endif
ifeq ($(strip $(CFG_CONFIG_APPLOG)),YES)
obj-y               += applog/
endif
obj-y               += diag_frame/
ifeq ($(strip $(CFG_CONFIG_DLOCK)),YES)
obj-y               += dlock/
endif
obj-$(CONFIG_BBP_ACORE)             += bbp/
obj-y               += net_helper/
obj-y               += mbb_modem_stub/

ifeq ($(strip $(CFG_CONFIG_WAN)),YES)
obj-y               += wan/
endif
obj-y               += buildin/
endif

