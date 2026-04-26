obj-$(CONFIG_HISI_USB_PHY_CHIP) += hisi_usb_phy_chip.o
obj-$(CONFIG_HISI_USB_PHY_DRIVER) += hisi_usb_phy_driver.o
ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DHISI_UPC_EFUSE_SUPPORT
EXTRA_CFLAGS += -DHISI_UPC_BC_FIX_FACTORY
endif
obj-$(CONFIG_HISI_USB_PHY_EFUSE) += hisi_usb_phy_chip_efuse.o
