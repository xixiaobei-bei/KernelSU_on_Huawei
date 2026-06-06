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
obj-$(CONFIG_BALONG_PCIE_ETH_RC)		+= pcie_nic.o pcie_nic_rc.o pcie_nic_desc.o pcie_nic_dbg.o pcie_balong_ipc.o

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_ETH_EP)),YES)
obj-y	+= pcie_nic.o pcie_nic_ep.o pcie_nic_desc.o pcie_nic_dbg.o pcie_nic_dma.o pcie_nic_bar.o
	ifeq ($(CFG_EXTRA_MODEM_MODE_MBB),FEATURE_ON)
	obj-y      += mdrv_remote_eth.o
	endif
endif