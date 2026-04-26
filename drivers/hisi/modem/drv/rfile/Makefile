-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk 

EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

obj-y  += file_acore.o
ifeq ($(strip $(CFG_CONFIG_RFILE_ON)),YES)
obj-y  += rfile_server.o
obj-y  += rfile_server_load_mode.o
obj-y  += rfile_server_dump.o
ifeq ($(strip $(CFG_CONFIG_ICC)),YES)
obj-y  += rfile_server_icc.o
else
obj-y  += rfile_server_eicc.o
endif
endif
