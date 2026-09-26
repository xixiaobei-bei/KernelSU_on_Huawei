-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk 

EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

ifneq ($(strip $(CFG_CONFIG_RFILE_ON)),YES)
obj-y  += rfile_balong.o
endif
endif
