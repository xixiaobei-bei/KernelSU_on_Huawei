-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
obj-y       +=  diag_frame.o
ifeq ($(strip $(CFG_CONFIG_MODULE_BUSSTRESS)),YES)
#obj-y       += diag_frame_test.o
else
obj-$(CONFIG_ENABLE_TEST_CODE) += diag_frame_test.o
endif

subdir-ccflags-y := -Werror