obj-$(CONFIG_WIRELESS_CHARGER) += wireless_power_supply.o wireless_charger.o wireless_transmitter.o
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_direct_charger/
obj-$(CONFIG_WIRELESS_IDTP9221) += idtp9221/
obj-$(CONFIG_WIRELESS_STWLC68) += stwlc68/

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DWIRELESS_CHARGER_FACTORY_VERSION
endif
