mkfilepath := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir :=$(dir $(mkfilepath))

 include $(current_dir)/balong_product_config_drv.mk
 include $(current_dir)/balong_product_config_pam.mk
 include $(current_dir)/balong_product_config_gucas.mk
 include $(current_dir)/balong_product_config_gucnas.mk
 include $(current_dir)/balong_product_config_gucphy.mk
 include $(current_dir)/balong_product_config_tlps.mk
 include $(current_dir)/balong_product_config_tlphy.mk
 include $(current_dir)/balong_product_config_audio.mk
 include $(current_dir)/balong_product_config_tool.mk
 include $(current_dir)/balong_product_config_version.mk

ifeq ($(choose_variant_modem),user)

ifeq ($(strip $(MODEM_FULL_DUMP)),true)
 include $(current_dir)/balong_product_config_modem_full_dump.mk
$(info cust MODEM_FULL_DUMP is true)
endif

ifeq ($(strip $(MODEM_DDR_MINI_DUMP)),true)
 include $(current_dir)/balong_product_config_modem_ddr_mini_dump.mk
$(info cust MODEM_DDR_MINI_DUMP is true)
endif

ifeq ($(strip $(MODEM_LOGCAT_INUSER)),true)
 include $(current_dir)/balong_product_config_modem_netlink.mk
$(info cust MODEM_LOGCAT_INUSER is true)
endif

ifeq ($(strip $(modem_log_filter_nv_control)),true)
 include $(current_dir)/balong_product_config_modem_log_filter.mk
$(info cust modem_log_filter_nv_control is true)
endif

ifeq ($(strip $(MODEM_CHR_INUSER)),true)
 include $(current_dir)/balong_product_config_modem_chr_ptm.mk
$(info cust MODEM_CHR_INUSER is true)
else
 include $(current_dir)/balong_product_config_modem_chr_ptm_off.mk
$(info cust MODEM_CHR_INUSER is not true)
endif

ifeq ($(strip $(MODEM_PM_OM_INUSER)),true)
 include $(current_dir)/balong_product_config_modem_pm_om.mk
$(info cust MODEM_PM_OM_INUSER is true)
endif
endif
