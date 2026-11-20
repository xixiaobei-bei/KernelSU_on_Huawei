EXTRA_CFLAGS += -Idrivers/hisi/rpmb
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_lpm3
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
EXTRA_CFLAGS += -Idrivers/hisi/mntn/hisee

subdir-ccflags-y              += -Wall -Werror

ifneq ($(CONFIG_HISEE_DISABLE_KEY),y)
ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DCONFIG_HISEE_CHIPTEST_RT
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DCONFIG_HISEE_CHIPTEST_SLT
endif

ifeq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DCONFIG_HISEE_SUPPORT_BYPASS_IMG
endif
endif

obj-$(CONFIG_HISEE)  += hisee.o
obj-$(CONFIG_HISEE)  += hisee_power.o
obj-$(CONFIG_HISEE)  += hisee_upgrade.o
obj-$(CONFIG_HISEE)  += hisee_chip_test.o
obj-$(CONFIG_HISEE)  += hisee_fs.o
obj-$(CONFIG_HISEE_SUPPORT_CASDKEY) += hisee_casdcert.o
obj-$(CONFIG_HISEE_SUPPORT_DCS) += hisee_dcs.o

ifneq ($(CONFIG_HISEE_DISABLE_KEY),y)
ifeq ($(CONFIG_HISEE),y)
obj-$(CONFIG_HISI_DEBUG_FS)  += hisee_autotest.o
endif
endif
