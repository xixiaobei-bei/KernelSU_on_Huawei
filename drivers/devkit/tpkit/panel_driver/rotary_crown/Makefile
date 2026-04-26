EXTRA_CFLAGS += -Idrivers/devkit/lcdkit/lcdkit1.0/include
obj-$(CONFIG_HUAWEI_ROTARY_CROWN) += rotary_crown.o
ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DRC_FACTORY_MODE
endif