ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

subdir-ccflags-y := -Werror

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
ifeq ($(strip $(CFG_CONFIG_BALONG_IPC_MSG_V230)),YES)
obj-y += ipc_msg.o
endif
