ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

subdir-ccflags-y := -Werror

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
ifeq ($(strip $(CFG_CONFIG_MAA_BALONG)),YES)
obj-y           += maa_acore.o bsp_maa.o maa_acore_test.o
endif
