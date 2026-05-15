-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk


ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif



obj-y       += iqi_device.o

subdir-ccflags-y := -Werror
