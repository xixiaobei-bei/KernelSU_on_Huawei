-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

obj-y	+= diag_acore_common.o
obj-y	+= diag_api.o
obj-y	+= diag_cfg.o
obj-y	+= diag_debug.o
obj-y	+= diag_message.o
obj-y	+= diag_msgapplog.o
obj-y	+= diag_msgbsp.o
obj-y	+= diag_msghifi.o
obj-y	+= diag_msgmsp.o
obj-y   += diag_msg_easyrf.o
obj-y	+= diag_msgphy.o
obj-y	+= diag_msgps.o
obj-y	+= diag_service.o
obj-y	+= diag_connect.o
obj-y	+= diag_msglrm.o
ifeq ($(strip $(CFG_DIAG_SYSTEM_5G)),YES)
ifeq ($(strip $(CFG_CONFIG_DIAG_NRM)),YES)
obj-y	+= diag_msgnrm.o
endif
obj-y	+= diag_time_stamp.o
endif

ifeq ($(strip $(CFG_CONFIG_DIAG_NRM)),YES)
obj-y	+= diag_msgbbp_nr_fpga.o
else
obj-y	+= diag_msgbbp.o
endif

ifeq ($(strip $(CFG_CONFIG_FILEBROSWER)),YES)
obj-y	+= diag_fs_proc.o
endif
