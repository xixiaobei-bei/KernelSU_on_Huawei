ifeq ($(CONFIG_ONEWIRE_PHY_PL061_GPIO), y)
ccflags-y += -I$(srctree)/drivers/gpio
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-y += -DONEWIRE_STABILITY_DEBUG
endif

obj-$(CONFIG_CERTIFICATION_IC_A1007) += lib/ a1007.o