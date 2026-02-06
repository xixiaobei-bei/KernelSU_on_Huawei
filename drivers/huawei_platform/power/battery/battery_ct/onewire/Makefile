ifeq ($(CONFIG_ONEWIRE_PHY_PL061_GPIO), y)
ccflags-y += -I$(srctree)/drivers/gpio
endif

obj-$(CONFIG_ONEWIRE_PHY_PL061_GPIO) += onewire_phy_pl061_gpio.o \
                                        onewire_common.o
obj-$(CONFIG_ONEWIRE_PHY_PL061_GPIO_V2) += onewire_phy_pl061_gpio_v2.o \
                                           onewire_common.o