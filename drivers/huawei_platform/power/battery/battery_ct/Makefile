#never change compiling order(drivers initialization order) below
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION) += maxim/ nxp/ richtek/ onewire/ infineon/

ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-y += -DBATTERY_LIMIT_DEBUG
endif

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DBATTBD_FORCE_MATCH
endif

#this should be under maxim|nxp|onewire makefile obj-y
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION) += batt_info.o
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION) += batt_aut_checker.o
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION) += batt_uuid_binder.o
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION) += batt_aut_util_v1.o
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION) += batt_aut_util_v2.o
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION) += batt_early_param.o
