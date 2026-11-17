ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

ifeq ($(strip $(CFG_CONFIG_RFS_SERVER)),YES)
obj-y += rfs_server.o
endif

