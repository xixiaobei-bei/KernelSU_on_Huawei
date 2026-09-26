-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

obj-y += block_balong.o
ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
obj-y += blk_ops_mmc.o
else
ifeq ($(strip $(CFG_CONFIG_RFS_CLIENT)),YES)
obj-y += blk_ops_rfs.o
else
obj-y += blk_ops_fmc.o
obj-y += blk_ops_mmc.o
endif
endif
