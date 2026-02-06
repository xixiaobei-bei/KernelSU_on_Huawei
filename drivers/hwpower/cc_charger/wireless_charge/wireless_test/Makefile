obj-$(CONFIG_WIRELESS_CHARGER) += charge_pump_test.o
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_test_hw.o
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_test_mmi.o
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_test_coil.o

ifeq ($(TARGET_VERSION_MODE),factory)
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_test_wp.o
endif
