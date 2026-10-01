-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

ifeq ($(strip $(CFG_CONFIG_DIAG_NETLINK)),YES)
obj-y       +=  diag_vcom_main.o  diag_vcom_handler.o
endif

subdir-ccflags-y := -Werror
