# ak4376 driver
ccflags-$(CONFIG_AK4376) += -Idrivers/devkit/audiokit/
ifneq ($(TARGET_BUILD_VARIANT),user)
ccflags-$(CONFIG_AK4376) += -DAK4376_DEBUG
endif
obj-$(CONFIG_AK4376) += ak4376.o
