obj-$(CONFIG_BCMGPS) += gps/

ifeq ($(CONFIG_BCM43752),y)
obj-$(CONFIG_BT) += bluetooth/
obj-$(CONFIG_BCMDHD) += syna/
else
obj-$(CONFIG_BT) += bluetooth/
obj-$(CONFIG_BCMDHD) += wifi/
endif
