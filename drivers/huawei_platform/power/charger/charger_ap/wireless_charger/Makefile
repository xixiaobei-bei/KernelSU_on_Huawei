obj-$(CONFIG_WIRELESS_CHARGER) += wireless_charger.o wireless_transmitter.o
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_direct_charger/
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_power_supply/
obj-$(CONFIG_WIRELESS_IDTP9221) += idtp9221/
obj-$(CONFIG_WIRELESS_ACCESSORY) += wireless_accessory/

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DWIRELESS_CHARGER_FACTORY_VERSION
endif
