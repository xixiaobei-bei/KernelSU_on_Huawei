#
#Makefile for the Linux Bluetooth HCI device drivers.
#

ifeq ($(CONFIG_BCM43752),y)
obj-y += bluetooth_power.o
obj-y += bluesleep.o
else
obj-$(CONFIG_BT_POWER) += bluetooth_power.o
obj-$(CONFIG_BT_SLEEP) += bluesleep.o
endif
