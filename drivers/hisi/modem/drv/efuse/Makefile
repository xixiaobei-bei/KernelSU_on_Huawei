ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
obj-$(CONFIG_EFUSE_BALONG)		+= efuse_balong.o
obj-$(CONFIG_EFUSE_BALONG_AGENT)	+= efuse_balong_agent.o
endif
obj-y	+= efuse_balong_ioctl.o
