#
# Makefile for the memory technology device drivers.
#
# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

ifeq ($(strip $(CFG_DIAG_SYSTEM_FUSION)),YES)
obj-y				+= diag_fusion_init.o
obj-y				+= diag_fusion_cfg.o
obj-y				+= diag_fusion_conn.o
else
obj-y				+= diag_comm.o
endif

ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
obj-y				+= diag_nve.o
endif 
