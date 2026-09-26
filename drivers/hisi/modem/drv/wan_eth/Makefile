#
# Makefile for PCI NET PORT Driver
#

subdir-ccflags-y := -Werror
#Build PCI NET if needed
#-I $(BALONG_TOPDIR)/system/kernel/drivers/hisi/spe/spe_drv
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

#this is kirin chip source config
ifeq ($(strip $(CFG_CONFIG_BALONG_WETH)),YES)
obj-y	+= wan_eth_rc.o wan_eth_table_rc.o wan_eth_event_buffer.o wan_eth_debug_rc.o
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_WETH_DEV)),YES)
obj-y	+=  wan_eth_ep.o wan_eth_table_ep.o wan_eth_dma.o wan_eth_event_buffer.o wan_eth_spe.o wan_eth_debug_ep.o mdrv_remote_eth.o
endif
