#
# Makefile for the modem drivers.
#
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifneq ($(strip $(CFG_ATE_VECTOR)),YES)
obj-y				+= sysctrl/
obj-y				+= s_memory/
obj-y				+= mperf/
obj-y				+= fiq/
ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
obj-y				+= balong_timer/
endif
ifeq ($(strip $(CFG_CONFIG_MODULE_IPC)),YES)
obj-y				+= ipc/
endif
obj-y				+= icc/
obj-y				+= reset/
obj-y				+= sec_call/

ifeq ($(strip $(CFG_CONFIG_ECDC)),YES)
obj-y				+= ecdc/
endif
obj-$(CONFIG_RFILE_SUPPORT)	+= rfile/
ifeq ($(strip $(CFG_FEATURE_TDS_WCDMA_DYNAMIC_LOAD)),FEATURE_ON)
obj-y				+= load_ps/
endif
obj-y				+= sync/
ifeq ($(strip $(CFG_ENABLE_BUILD_SOCP)),YES)
obj-y			+= socp/
endif
ifeq ($(strip $(CFG_CONFIG_NRDSP)),YES)
obj-y				+= nrdsp/
endif
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y				+= dump/
endif
obj-y				+= nvim/
ifeq ($(strip $(CFG_CONFIG_FAULT_INFLOOD)),YES)
obj-y				+= fault_inflood/
endif

ifeq ($(strip $(CFG_CONFIG_MLOADER)),YES)
obj-y += mloader/
else
obj-$(CONFIG_IMAGE_LOAD) += loadm/
endif

ifeq ($(strip $(CFG_CONFIG_ZSTD_DECOMPRESS)),YES)
obj-y                   += zstd/
endif

obj-y			+= adp/
obj-y			+= hwadp/
obj-y			+= version/
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y			+= log/
endif
ifneq ($(strip $(OBB_SEPARATE)),true) 
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
obj-y			    += diag/
obj-y			    += diag_vcom/
endif
endif
obj-y			+= hds/

ifeq ($(strip $(att_iqi)),true)
obj-y			+= iqi/
endif
ifeq ($(strip $(att_iqi)),true)
subdir-ccflags-y += -DFEATURE_IQI=FEATURE_ON
else
subdir-ccflags-y += -DFEATURE_IQI=FEATURE_OFF
endif

ifeq ($(strip $(CFG_BOARD_TRACE)),YES)
obj-y			+= board_trace/
endif

ifeq ($(strip $(CFG_CONFIG_SYSBUS)),YES)
obj-y			+= sys_bus/
endif


obj-$(CONFIG_BALONG_ESPE)	+= espe/

ifeq ($(strip $(CFG_CONFIG_NEW_PLATFORM)),YES)
obj-y			+= n_ipf/
obj-y 			+= n_psam/
else
obj-y			+= ipf/
obj-y 			+= psam/
endif
obj-y			+= udi/
obj-y			+= thermal/
 
obj-$(CONFIG_MEM_BALONG)	+= mem/

obj-y 	+= onoff/

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
obj-y 	+= efuse/
endif

ifeq ($(strip $(CFG_CONFIG_CSHELL)),YES)
obj-y += console/
endif

ifeq ($(strip $(CFG_CONFIG_SC)),YES)
obj-y  				+= sc/
endif
obj-y			+= pm_om/
 
obj-$(CONFIG_PMIC_OCP) += ocp/

# llt module
ifeq ($(strip $(OBB_LLT_MDRV)),y)
obj-y			+= llt_tools/
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
obj-y				+= sysctrl/
obj-y				+= s_memory/
obj-y				+= mperf/
ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
obj-y				+= balong_timer/
endif
obj-y				+= icc/
obj-y				+= reset/
obj-y				+= adp/
obj-y				+= hwadp/
ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y				+= om/
endif
obj-y				+= pm_om/
endif
ifeq ($(strip $(CFG_CONFIG_APPLOG)),YES)
obj-y				+= applog/
endif
obj-y				+= diag_frame/
ifeq ($(strip $(CFG_CONFIG_DLOCK)),YES)
obj-y			    += dlock/
endif
obj-$(CONFIG_BBP_ACORE)			    += bbp/
obj-y				+= net_helper/
obj-y				+= mbb_kernel_stub/
