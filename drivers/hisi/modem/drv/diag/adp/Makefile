#
# Makefile for the memory technology device drivers.
#
# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk


ifeq ($(strip $(CFG_DIAG_SYSTEM_FUSION)),YES)
obj-y				+= adp_fusion_diag.o 
else
obj-y				+= adp_diag.o
endif




