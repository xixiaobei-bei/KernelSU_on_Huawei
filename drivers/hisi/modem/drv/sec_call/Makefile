
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

obj-$(CONFIG_SEC_CALL) += sec_call.o
ifeq ($(strip $(CFG_CONFIG_TRUSTZONE_HM)),YES)
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver_hm
else
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver \
                -Idrivers/tzdriver

endif

subdir-ccflags-y := -Werror
