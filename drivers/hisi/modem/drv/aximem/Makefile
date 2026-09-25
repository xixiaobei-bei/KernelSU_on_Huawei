ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

subdir-ccflags-y := -Werror

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
ifeq ($(strip $(CFG_CONFIG_AXIMEM_BALONG)),YES)
obj-y           += aximem.o
endif
