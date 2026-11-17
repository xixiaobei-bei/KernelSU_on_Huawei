ifeq ($(CONFIG_ONEWIRE_PHY_PL061_GPIO), y)
ccflags-y += -I$(srctree)/drivers/gpio
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-y += -DONEWIRE_STABILITY_DEBUG
endif

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DBATTBD_FORCE_MATCH
endif

obj-$(CONFIG_HUAWEI_BATTCT_SLE95250) += sle95250.o
obj-$(CONFIG_HUAWEI_BATTCT_SLE95250) += optiga_nvm.o optiga_reg.o optiga_swi.o