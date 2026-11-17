obj-$(CONFIG_HW_DP_SOURCE) += dp_source_switch.o

obj-$(CONFIG_HUAWEI_DSM) += dp_dsm.o dp_dsm_debug.o

ifeq ($(CONFIG_HW_DP_SOURCE),y)
ifeq ($(TARGET_VERSION_MODE),factory)
	ccflags-y += -DDP_FACTORY_MODE_ENABLE
endif
ifneq ($(TARGET_BUILD_VARIANT),user)
	ccflags-y += -DDP_DEBUG_ENABLE
endif
endif

obj-y += dp_factory.o
obj-y += dp_debug.o
