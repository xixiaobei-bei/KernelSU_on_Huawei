#
# Makefile for PCI NET PORT Driver
#

subdir-ccflags-y := -Werror
#Build PCI NET if needed
#-I $(BALONG_TOPDIR)/system/kernel/drivers/hisi/spe/spe_drv
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

#this is kirin chip source config
obj-$(CONFIG_BALONG_WETH)		+= wan_eth.o wan_eth_rc.o wan_eth_hal.o wan_eth_hal_debug.o

ifeq ($(strip $(CFG_CONFIG_BALONG_WETH_DEV)),YES)
obj-y	+=  wan_eth_ep.o wan_eth_table_ep.o wan_eth_dma.o wan_eth_event_buffer.o wan_eth_spe.o wan_eth_debug.o
	ifeq ($(CFG_EXTRA_MODEM_MODE_MBB),FEATURE_ON)
	obj-y      += mdrv_remote_eth.o
	endif
endif
