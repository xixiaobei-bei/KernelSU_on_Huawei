obj-$(CONFIG_HUAWEI_CHARGER_AP) += huawei_charger.o
obj-$(CONFIG_HUAWEI_POWER_DEBUG) += charging_core_debug.o
obj-$(CONFIG_HUAWEI_CHARGER_AP) += charging_core.o
obj-$(CONFIG_DIRECT_CHARGER) += direct_charger/
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_charger/
obj-$(CONFIG_SERIES_BATT) += series_batt_charger.o
obj-$(CONFIG_BATTERY_BALANCE) += battery_balance.o
obj-$(CONFIG_UVDM_CHARGER) += uvdm_charger/

ifeq ($(HLTHERM_RUNTEST), true)
    CFLAGS_KERNEL += -DCONFIG_HLTHERM_RUNTEST
endif
