subdir-ccflags-y := -Werror

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_CDEV)),YES)
obj-y += pcie_cdev.o pcie_cdev_ep.o pcie_cdev_desc.o pcie_cdev_dbg.o pcie_cdev_test.o hisi_adp_pcdev.o pcie_cdev_bar.o
ifeq ($(strip $(CFG_CONFIG_PCIE_CDEV_ENG)),YES)
obj-y += pcie_tty.o
ifeq ($(strip $(CFG_CONFIG_PCIE_MODEM)),YES)
obj-y += pcie_modem.o
endif
endif
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_CDEV_RC)),YES)
obj-y += pcie_cdev.o pcie_cdev_rc.o pcie_cdev_desc.o pcie_cdev_dbg.o pcie_cdev_test.o hisi_adp_pcdev_rc.o

ifeq ($(strip $(CFG_CONFIG_PCIE_DEV_NORMALIZE)),NO)
obj-y += pcie_usb_relay.o
endif

ifeq ($(strip $(CFG_CONFIG_PCIE_CDEV_ENG)),YES)
ifeq ($(strip $(CFG_CONFIG_PCIE_MODEM)),YES)
obj-y += pcie_modem.o
endif
endif
endif
