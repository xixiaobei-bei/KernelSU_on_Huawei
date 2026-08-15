#gcov
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
GCOV_PROFILE_icc_debug.o := n
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
ifeq ($(strip $(CFG_CONFIG_ICC)),YES)
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
obj-$(CONFIG_ICC_BALONG)       += icc_core.o  icc_linux.o  icc_debug.o
obj-$(CONFIG_CA_ICC)           += ca_icc.o
obj-$(CONFIG_ENABLE_TEST_CODE) += icc_test.o
endif

subdir-ccflags-y := -Werror
