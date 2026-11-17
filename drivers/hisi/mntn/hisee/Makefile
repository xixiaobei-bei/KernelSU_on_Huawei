ccflags-y += -Wall -Werror
EXTRA_CFLAGS += -Idrivers/hisi/tzdriver
ifeq ($(CONFIG_HISEE),y)
        EXTRA_CFLAGS += -Idrivers/hisi/hisee
endif
ifeq ($(CONFIG_MSPC),y)
        EXTRA_CFLAGS += -Idrivers/hisi/mspc
endif

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DFACTORY_TEST_HISEE
endif

obj-$(CONFIG_HISEE_MNTN) += hisee_mntn.o
