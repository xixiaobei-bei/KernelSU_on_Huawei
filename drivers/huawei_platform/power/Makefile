obj-$(CONFIG_HUAWEI_CHARGER)	+= charger/
obj-$(CONFIG_HUAWEI_POWER_MESG_INTERFACE)	+= power_mesg/
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION)    	+= battery/
obj-$(CONFIG_HUAWEI_COUL)	+= coul/
obj-$(CONFIG_HUAWEI_KSTATE)	+= hw_kstate/
obj-$(CONFIG_HUAWEI_FEATURE_PRINT_PID_NAME) += pid_socket/
obj-y += hardware_monitor/
obj-y += hardware_buckboost/
obj-y += common_module/
obj-$(CONFIG_HUAWEI_VSYS_SWITCH) += vsys_switch/

ifeq ($(HLTHERM_RUNTEST),true)
CFLAGS_KERNEL += -DCONFIG_HLTHERM_RUNTEST
endif

obj-$(CONFIG_DOZE_FILTER)	+= wifi_filter/
