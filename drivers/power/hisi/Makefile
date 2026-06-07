obj-$(CONFIG_HISI_BATTERY_DATA)		+= hisi_battery_data.o
obj-$(CONFIG_HISI_BCI_BATTERY)		+= hisi_bci_battery.o
obj-$(CONFIG_HISI_COUL)		        += coul/
obj-$(CONFIG_HISI_CHARGER)		+= charger/
obj-$(CONFIG_HISI_SOH)	                += soh/
obj-$(CONFIG_HISI_ASW)	                += asw/
obj-$(CONFIG_HISI_CHARGE_TIME)             += charge_time/

ifeq ($(HLTHERM_RUNTEST),true)
CFLAGS_KERNEL += -DCONFIG_HLTHERM_RUNTEST
endif
