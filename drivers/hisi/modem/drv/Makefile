#
# Makefile for the modem drivers.
#
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk 
ifneq ($(strip $(CFG_ATE_VECTOR)),YES)
obj-y				+= sysctrl/
obj-y				+= s_memory/
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
obj-$(CONFIG_RFILE_SUPPORT)	+= rfile/
ifeq ($(strip $(CFG_FEATURE_TDS_WCDMA_DYNAMIC_LOAD)),FEATURE_ON)
obj-y				+= load_ps/
endif
obj-y				+= sync/
ifeq ($(strip $(CFG_ENABLE_BUILD_SOCP)),YES)
obj-y			+= socp/
endif
obj-y				+= nvim/
# make sure temperature between nvim and loadm
obj-y				+= temperature/

obj-$(CONFIG_IMAGE_LOAD) += loadm/

obj-y			+= adp/
obj-y			+= hwadp/
obj-y			+= version/
#obj-$(CONFIG_BALONG_EASY_SHELL) += easy_shell/
obj-y			+= log/

ifeq ($(strip $(CFG_CONFIG_SYSBUS)),YES)
obj-y			+= sys_bus/
endif

obj-y			+= ipf/
obj-y 			+= psam/
obj-y			+= udi/
obj-y			+= thermal/
ifeq ($(strip $(CFG_CONFIG_RFFE_ANTEN)),YES)
obj-y				+= antenna/
endif

 
obj-$(CONFIG_MEM_BALONG)	+= mem/

obj-y 	+= onoff/

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
obj-y 	+= efuse/
endif


obj-y				+= modem_dump/
ifeq ($(strip $(CFG_CONFIG_SC)),YES)
obj-y  				+= sc/
endif
obj-y			+= pm_om/
 
obj-$(CONFIG_PMIC_OCP) += ocp/

ifeq ($(LLTMDRV_HUTAF_COV),true)
obj-y			+= lltcov/
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_BUSSTRESS)),YES)
obj-y                   += busstress/$(OBB_PRODUCT_NAME)/
endif

obj-$(CONFIG_HISI_SIM_HOTPLUG) += sim_hotplug/
else
obj-y				+= sysctrl/
obj-y				+= s_memory/
ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
obj-y				+= balong_timer/
endif
obj-y				+= icc/
obj-y				+= reset/
obj-y				+= adp/
obj-y				+= hwadp/
obj-y				+= om/
obj-y				+= pm_om/
endif
ifeq ($(strip $(CFG_CONFIG_APPLOG)),YES)
obj-y				+= applog/
endif
obj-y				+= diag_frame/

