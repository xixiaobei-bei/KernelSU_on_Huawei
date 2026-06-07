obj-$(CONFIG_HUAWEI_CHARGER) += charger/
obj-$(CONFIG_ADAPTER_PROTOCOL) += adapter/
obj-y += battery/
obj-$(CONFIG_HUAWEI_COUL) += coul/
obj-y += hardware_monitor/
obj-y += hardware_buckboost/
obj-y += common_module/
obj-$(CONFIG_HUAWEI_VSYS_SWITCH) += vsys_switch/

ifeq ($(HLTHERM_RUNTEST), true)
CFLAGS_KERNEL += -DCONFIG_HLTHERM_RUNTEST
endif

obj-$(CONFIG_HUAWEI_KSTATE) += hw_kstate/
obj-$(CONFIG_HUAWEI_FEATURE_PRINT_PID_NAME) += pid_socket/
obj-$(CONFIG_DOZE_FILTER) += wifi_filter/
