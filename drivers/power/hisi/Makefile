obj-$(CONFIG_BATTERY_DATA)		+= battery_data.o
obj-$(CONFIG_BCI_BATTERY)		+= bci_battery.o
obj-$(CONFIG_COUL_DRV)		        += coul/
obj-$(CONFIG_COUL_DRV)		        += battery_thermal/
obj-$(CONFIG_SCHARGER)		+= charger/
obj-$(CONFIG_BATT_SOH)	                += soh/
obj-$(CONFIG_BATT_EIS)                  += eis/
obj-$(CONFIG_CHARGE_TIME)          += charge_time/
obj-$(CONFIG_BATTERY_ISCD)              += battery_iscd/

ifeq ($(HLTHERM_RUNTEST),true)
CFLAGS_KERNEL += -DCONFIG_HLTHERM_RUNTEST
endif
