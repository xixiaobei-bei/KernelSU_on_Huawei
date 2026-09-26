include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
#
# CONFIG_HISI_BALONG_EXTRA_MODEM A+B A≤‡ defconfig  «∑ÒÕ‚π“√®
# makefile_soc_modem    ∑«A+B–ŒÃ¨±‡“Î£¨A+B B≤‡±‡“Î
# makefile_extra_modem  A+B A≤‡±‡“Î

ifeq ($(choose_variant),user)
KBUILD_CFLAGS += -DCHOOSE_MODEM_USER
endif

ifeq ($(findstring modem_user,$(cust_config)),modem_user)
KBUILD_CFLAGS += -DCHOOSE_MODEM_USER
endif

ifeq ($(strip $(MODEM_FULL_DUMP)),true)
subdir-ccflags-y += -DMODEM_FULL_DUMP_INUSER
endif

ifeq ($(strip $(MODEM_DDR_MINI_DUMP)),true)
subdir-ccflags-y += -DMODEM_DDR_MINI_DUMP_INUSER
endif

ifeq ($(strip $(MODEM_LOGCAT_INUSER)),true)
subdir-ccflags-y += -DMODEM_LOGCAT_INUSER
endif

ifeq ($(strip $(modem_log_filter_nv_control)),true)
subdir-ccflags-y += -DMODEM_LOG_FILTER_INUSER
endif

ifeq ($(strip $(MODEM_CHR_INUSER)),true)
subdir-ccflags-y += -DMODEM_CHR_INUSER
endif

ifeq ($(strip $(MODEM_PM_OM_INUSER)),true)
subdir-ccflags-y += -DMODEM_PM_OM_INUSER
endif


ifeq ($(strip $(CONFIG_HISI_BALONG_EXTRA_MODEM)),y)
-include $(srctree)/drivers/hisi/modem/drv/makefile_extra_modem.mk

else
include $(srctree)/drivers/hisi/modem/drv/makefile_soc_modem.mk

endif

