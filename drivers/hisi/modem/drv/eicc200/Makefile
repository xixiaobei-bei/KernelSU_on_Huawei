
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

subdir-ccflags-y := -Wall -Werror
ifeq ($(strip $(CFG_CONFIG_EICC_V200)),YES)
obj-y += eicc_core.o
obj-y += eicc_device.o
obj-y += eicc_driver.o
obj-y += eicc_dump.o
obj-y += eicc_platform.o
obj-y += eicc_pmsr.o
obj-y += eicc_reset.o
obj-y += eicc_test.o
obj-y += eicc_dts.o
endif
