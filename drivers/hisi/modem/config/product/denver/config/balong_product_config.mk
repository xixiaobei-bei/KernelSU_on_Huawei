 OS_NAME = $(shell uname -s)
 LC_OS_NAME = $(shell echo $(OS_NAME) | tr '[A-Z]' '[a-z]')
 last_file_name := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

 ifeq ($(LC_OS_NAME), linux)
 mkfilepath := $(abspath $(last_file_name))
 else
 mkfilepath := $(last_file_name)
 endif

 current_dir :=$(dir $(mkfilepath))

ifeq ($(extra_modem),)
 include $(current_dir)/product_config_sapper.mk
 include $(current_dir)/balong_product_config_drv.mk
 include $(current_dir)/balong_product_config_pam.mk
 include $(current_dir)/balong_product_config_gucas.mk
 include $(current_dir)/balong_product_config_gucnas.mk
 include $(current_dir)/balong_product_config_easyrf.mk
 include $(current_dir)/balong_product_config_tlps.mk
 include $(current_dir)/balong_product_config_audio.mk
 include $(current_dir)/balong_product_config_tool.mk
 include $(current_dir)/balong_product_config_nrps.mk
 include $(current_dir)/balong_product_config_version.mk

else

 include $(current_dir)/product_config_sapper.mk
 include $(current_dir)/balong_product_config_drv_extra.mk
 include $(current_dir)/balong_product_config_pam.mk
 include $(current_dir)/balong_product_config_gucas.mk
 include $(current_dir)/balong_product_config_gucnas_extra.mk
 include $(current_dir)/balong_product_config_easyrf.mk
 include $(current_dir)/balong_product_config_tlps.mk
 include $(current_dir)/balong_product_config_audio.mk
 include $(current_dir)/balong_product_config_tool.mk
 include $(current_dir)/balong_product_config_nrps.mk

endif

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

