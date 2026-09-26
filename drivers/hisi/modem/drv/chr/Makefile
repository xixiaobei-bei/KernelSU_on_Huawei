#
# Makefile for the memory technology device drivers.
#

# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk


ifeq ($(strip $(CFG_FEATURE_CHR_OM)),FEATURE_ON)
obj-y      += chr_balong.o
else
obj-y      += chr_balong_stub.o
endif
