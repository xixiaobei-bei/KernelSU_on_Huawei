ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
obj-y       += eicc_driver.o eicc_acpu.o eicc_core.o eicc_interface.o eicc_test.o

subdir-ccflags-y := -Werror

