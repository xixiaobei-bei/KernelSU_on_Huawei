#gcov
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nva/comm
ifeq ($(strip $(CFG_FEATURE_NVA_ON)),YES)
    obj-y    += nva_balong.o
    obj-y    += nva_partition.o
endif
