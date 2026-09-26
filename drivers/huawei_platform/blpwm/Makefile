obj-$(CONFIG_HUAWEI_BLPWM) += blpwm.o
obj-$(CONFIG_HUAWEI_BLPWM) += bl_gpio.o
obj-$(CONFIG_HUAWEI_BLPWM) += bl_sns.o

EXTRA_CFLAGS += -Idrivers/huawei_platform/inputhub/default
ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DFACTORY_BLPWM
endif
