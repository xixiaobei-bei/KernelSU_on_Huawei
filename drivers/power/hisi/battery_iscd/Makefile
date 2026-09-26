ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-y += -DISC_TEST
else
ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DISC_TEST
endif
endif

obj-$(CONFIG_BATTERY_ISCD)           += battery_iscd.o battery_iscd_interface.o
