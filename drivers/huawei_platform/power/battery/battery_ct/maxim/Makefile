ifeq ($(CONFIG_ONEWIRE_PHY_PL061_GPIO), y)
ccflags-y += -I$(srctree)/drivers/gpio
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-y += -DONEWIRE_STABILITY_DEBUG
endif

obj-$(CONFIG_CERTIFICATION_IC_DS28EL15) += ds28el15.o maxim_onewire.o
obj-$(CONFIG_CERTIFICATION_IC_DS28E16)  += ds28el16.o maxim_common.o