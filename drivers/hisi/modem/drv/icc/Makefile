
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
ifeq ($(strip $(CFG_CONFIG_ICC)),YES)
obj-$(CONFIG_ICC_BALONG)       += icc_core.o  icc_linux.o  icc_debug.o
endif

subdir-ccflags-y := -Werror
